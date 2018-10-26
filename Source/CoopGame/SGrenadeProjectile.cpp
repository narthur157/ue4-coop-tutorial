#include "SGrenadeProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SWeapon.h"
#include "SHealthComponent.h"

ASGrenadeProjectile::ASGrenadeProjectile()
{
	ProjectileSpeed = 3000.0f;
	TimeToDetonate = 1.0f;

	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &ASGrenadeProjectile::OnHit);	// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	RootComponent = CollisionComp;

	PrimaryActorTick.bCanEverTick = true;
	MoveComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjMoveComp"));
	MoveComp->InitialSpeed = ProjectileSpeed;
	MoveComp->MaxSpeed = ProjectileSpeed;
	MoveComp->bShouldBounce = true;

	SetReplicates(true);
	SetReplicateMovement(true);
}

void ASGrenadeProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	USHealthComponent* OtherHealth = OtherActor->FindComponentByClass<USHealthComponent>();
	if (OtherActor && OtherActor != this && Cast<APawn>(OtherActor) != Instigator && 
		OtherHealth)
	{
		Explode();
	}
}


// Called when the game starts or when spawned
void ASGrenadeProjectile::BeginPlay()
{
	Super::BeginPlay();

	APawn* Inst = Cast<APawn>(Instigator);
	if (Inst && Inst->IsLocallyControlled())
	{
		GetWorldTimerManager().SetTimer(ExplodeTimer, this, &ASGrenadeProjectile::Explode, TimeToDetonate);
	}
}

void ASGrenadeProjectile::Explode()
{
	if (Role < ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("GrenadeProjectileExplode4"));
	}
	TArray<AActor*> IgnoredActors = { this, GetOwner() };
	UGameplayStatics::ApplyRadialDamage(GetWorld(), Damage, GetActorLocation(), DamageRadius, DamageType, IgnoredActors);

	if (ImpactEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, GetActorLocation(), GetActorRotation());
		if (ExplosionSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GrenadeProjectile is missing sound effect"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GrenadeProjectile is missing impact effect"));
	}

	OnExplosion(GetActorLocation(), DamageRadius);
	SetActorHiddenInGame(true);

	// Prevent server from destroying the projectile too early for replication to occur
	SetLifeSpan(5.0f);
}

void ASGrenadeProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

