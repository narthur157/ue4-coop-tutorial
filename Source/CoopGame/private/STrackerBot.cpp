#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "SCharacter.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"
#include "SCountNearbyActorComponent.h"

ASTrackerBot::ASTrackerBot(const FObjectInitializer& ObjectInitializer)
 : Super (ObjectInitializer) {
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionObjectType(ECC_Pawn);
	bUseVelocityChange = false;
	MovementForce = 1000;
	RequiredDistanceToTarget = 100;
	BaseDamage = 50.0f;
	DamageRadius = 400.0f;
	MaxSpeed = 500.0f;
	NavRecalcInterval = 1.0f;
	SelfHarmInterval = 0.5f;
	PowerLevel = 1;
	MaxPowerLevel = 3;
	NearbyActorPowerScaling = 10.0f;
	LastNavCalcTime = -10.0f;
	bExploded = false;
	bStartedSelfDestruct = false;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	DamageSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSphereComp"));
	DamageSphereComp->SetSphereRadius(DamageRadius - 100.0f);
	//DamageSphereComp->SetupAttachment(MeshComp);
	DamageSphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageSphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageSphereComp->OnComponentBeginOverlap.AddDynamic(this, &ASTrackerBot::OnBeginOverlap);

	SwarmRadius = 800.0f;
	ActorTrackComp = CreateDefaultSubobject<USCountNearbyActorComponent>(TEXT("ActorTrackComp"));
	ActorTrackComp->SetCheckRadius(SwarmRadius);
	ActorTrackComp->SetupAttachment(MeshComp);

}

void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	ActorTrackComp->SetRelativeLocation(FVector::ZeroVector);
	if (ActorTrackComp)
	{
		ActorTrackComp->OnNearbyActorCountChanged.AddDynamic(this, &ASTrackerBot::OnPowerLevelChanged);
	}

	if (Role == ROLE_Authority)
	{
		FVector NextPoint = GetNextPathPoint();
		//DrawDebugSphere(GetWorld(), NextPoint, 50, 12, FColor::Orange, false, 3.0f, 0, 1);
	}

	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
}

void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority && !bExploded)
	{
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		float TimeSinceNavCalc = GetWorld()->TimeSeconds - LastNavCalcTime;

		if (DistanceToTarget <= RequiredDistanceToTarget || TimeSinceNavCalc > NavRecalcInterval)
		{
			NextPathPoint = GetNextPathPoint();
			//DrawDebugString(GetWorld(), GetActorLocation(), "Tracer Reached!");
		}

		FVector ForceDirection = NextPathPoint - GetActorLocation();
		ForceDirection.Normalize();
		
		float ActualMoveSpeed = GetVelocity().Size();
		FVector MoveDirection = GetVelocity();
		MoveDirection.Normalize();

		float SpeedTowardsTarget = (((ForceDirection + MoveDirection) / 2) * ActualMoveSpeed).Size();

		if (SpeedTowardsTarget <= MaxSpeed)
		{

			ForceDirection *= MovementForce;
			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);
			//DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Green, false, 0);
		}


		//DrawDebugSphere(GetWorld(), NextPathPoint, RequiredDistanceToTarget, 12, FColor::Yellow, false, 0, 3.0f);
	}
}

void ASTrackerBot::OnPowerLevelChanged(int32 NearbyActors)
{
	PowerLevel = NearbyActors;

	if (MatInst)
	{
		MatInst->SetScalarParameterValue("PowerLevelAlpha", (PowerLevel-1) / (float) MaxPowerLevel);
	}

}

void ASTrackerBot::SelfDestruct()
{
	if (bExploded) {
		return;
	}

	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplodeSound, GetActorLocation());

	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Role == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		float Damage = BaseDamage + FMath::Clamp(PowerLevel - 1, 0, MaxPowerLevel - 1) * NearbyActorPowerScaling;

		UGameplayStatics::ApplyRadialDamage(this, Damage, GetActorLocation(), DamageRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);
		
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SetLifeSpan(2.0f);
	}

	//	DrawDebugSphere(GetWorld(), GetActorLocation(), DamageRadius, 12, FColor::Red, false, 3.0f);

}

void ASTrackerBot::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (bStartedSelfDestruct || bExploded)
	{
		return;
	}

	ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);

	if (PlayerPawn)
	{
		if (Role == ROLE_Authority)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfHarmInterval, true, 0.0f);
		}

		bStartedSelfDestruct = true;
		UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
	}
}

void ASTrackerBot::DamageSelf()
{
	if (Role == ROLE_Authority)
	{
		UGameplayStatics::ApplyDamage(this, 20.0f, GetInstigatorController(), this, nullptr);
	}
}

void ASTrackerBot::HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	if (Health <= 0.0f)
	{
		SelfDestruct();
	}
}

FVector ASTrackerBot::GetNextPathPoint()
{
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);

	if (PlayerPawn) {
		UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

		if (NavPath->PathPoints.Num() > 1)
		{
			LastNavCalcTime = GetWorld()->TimeSeconds;
			// DrawDebugSphere(GetWorld(), NavPath->PathPoints[1], 50, 12, FColor::Blue, false, 3.0f, 0, 1);
			return NavPath->PathPoints[1];
		}
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("STrackerBot GetNextPathPoint could not find PlayerPawn or NavPath only had 1 element"));

	return GetActorLocation();
}
