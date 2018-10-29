#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "CoopGame.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw Debug Lines for Weapons"),
	ECVF_Cheat);

ASWeapon::ASWeapon()
{
	RateOfFire = 300;
	BaseDamage = 20.0f;
	HitDistance = 10000.0f;
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "BeamEnd";

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;
}

void ASWeapon::PlayMuzzleEffect()
{
	APawn* MyOwner = Cast<APawn>(GetOwner());
	APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());

	if (PC)
	{
		PC->ClientPlayCameraShake(FireCamShake);
	}

	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}
}

void ASWeapon::PlayTracerEffect(FVector TracerEndPoint)
{
	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);

		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}
}

void ASWeapon::PlayImpactEffect(EPhysicalSurface SurfaceType, FVector Loc)
{
	UParticleSystem* SelectedEffect = ImpactEffect;

	if (SurfaceType == SURFACE_FLESHVULNERABLE)
	{
		SelectedEffect = FleshImpactEffect;
	}

	if (SelectedEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FRotator Rot = UKismetMathLibrary::FindLookAtRotation(Loc, MuzzleLocation);

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Loc, Rot);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("No impact effect"));
	}
}

void ASWeapon::PlaySoundEffect(FVector Loc)
{
	if (SoundEffect)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundEffect, GetActorLocation());
	}
}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

bool ASWeapon::Hitscan(FHitResult& Hit)
{
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FVector EyeLoc;
		FRotator EyeRot;
		MyOwner->GetActorEyesViewPoint(EyeLoc, EyeRot);

		FVector ShotDirection = EyeRot.Vector();

		FVector TraceEnd = EyeLoc + (ShotDirection * HitDistance);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		FVector TracerEndPoint = TraceEnd;

		return GetWorld()->LineTraceSingleByChannel(Hit, EyeLoc, TraceEnd, COLLISION_WEAPON, QueryParams);
	}

	return false;
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}


void ASWeapon::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FHitResult Hit;

		HitScanTrace.SurfaceType = SurfaceType_Default;

		if (Hitscan(Hit))
		{
			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			HitScanTrace.SurfaceType = SurfaceType;

			float ActualDamage = BaseDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage = BaseDamage * 2;
			}

			FRotator ShotDirection = UKismetMathLibrary::FindLookAtRotation(Hit.TraceStart, Hit.TraceEnd);

			UGameplayStatics::ApplyPointDamage(Hit.GetActor(), ActualDamage, ShotDirection.Vector(),
				Hit, MyOwner->GetInstigatorController(), this, DamageType);

			PlayImpactEffect(SurfaceType, Hit.ImpactPoint);
		}

		PlayMuzzleEffect();
		PlayTracerEffect(Hit.ImpactPoint);
		PlaySoundEffect(GetActorLocation());

		LastFireTime = GetWorld()->TimeSeconds;

		if (Role == ROLE_Authority)
		{
			HitScanTrace.TraceTo = Hit.ImpactPoint;
		}
	}
}

void ASWeapon::OnRep_HitScanTrace()
{
	PlayMuzzleEffect();
	PlayTracerEffect(HitScanTrace.TraceTo);
	PlaySoundEffect(GetActorLocation());
	PlayImpactEffect(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}
