#include "SExplosiveBarrel.h"
#include "SHealthComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ASExplosiveBarrel::ASExplosiveBarrel()
{
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CylinderComp"));
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	ForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("ForceComp"));
	ForceComp->SetupAttachment(RootComponent);
	ForceComp->Radius = 250;
	ForceComp->bImpulseVelChange = true;
	ForceComp->bAutoActivate = false;
	ForceComp->bIgnoreOwningActor = true;

	BarrelLaunchStrength = 100.0f;
	ImpulseStrength = 500.0f;

	SetReplicates(true);
	SetReplicateMovement(true);
	MeshComp->SetIsReplicated(true);

	if (PreExplosionMaterial)
	{
		MeshComp->SetMaterial(0, PreExplosionMaterial);
	}
}

void ASExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();


	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);
}

void ASExplosiveBarrel::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDied)
	{
		bDied = true;
		Explode();
	}
}

void ASExplosiveBarrel::ExplodeEffects()
{
	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	}

	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());
	}

	if (PostExplosionMaterial)
	{
		MeshComp->SetMaterial(0, PostExplosionMaterial);
	}
}

void ASExplosiveBarrel::Explode()
{
	ForceComp->ImpulseStrength = ImpulseStrength;
	ForceComp->FireImpulse();
	
	MeshComp->AddImpulse(FVector::UpVector * BarrelLaunchStrength, "", true);

	ExplodeEffects();
}

void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASExplosiveBarrel, bDied, COND_SkipOwner);
}
