#include "SBeamWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "CoopGame.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

ASBeamWeapon::ASBeamWeapon()
{
	RateOfFire = 300;
	HitDistance = 300.0f;
	BeamSocket = "BeamSocket";

	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void ASBeamWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (BeamEffect)
	{
		BeamParticleComponent = UGameplayStatics::SpawnEmitterAttached(BeamEffect, MeshComp, BeamSocket);
		
		if (BeamParticleComponent)
		{
			BeamParticleComponent->SetFloatParameter("BeamLength", HitDistance);
			BeamParticleComponent->SetIsReplicated(true);
			BeamParticleComponent->Activate();
			BeamParticleComponent->SetHiddenInGame(true);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("BeamParticleComponent failed to spawn"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Beam weapon is missing beam effect"));
	}
}

void ASBeamWeapon::PostNetReceiveLocationAndRotation()
{
	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner && !MyOwner->IsLocallyControlled())
	{
		Super::PostNetReceiveLocationAndRotation();
	}
}

void ASBeamWeapon::StartFire()
{
	Super::StartFire();

	if (Role < ROLE_Authority)
	{
		ServerStartFire();
	}

	if (BeamEffect)
	{
		BeamStatus.bIsActive = true;
		ApplyBeamStatus();
	}
}

void ASBeamWeapon::StopFire()
{
	Super::StopFire();

	if (Role < ROLE_Authority)
	{
		ServerStopFire();
	}

	if (BeamEffect)
	{

		BeamStatus.bIsActive = false;
		ApplyBeamStatus();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BeamEffect Or ParticleComponent null"));
	}
}

bool ASBeamWeapon::ServerStopFire_Validate()
{
	return true;
}
bool ASBeamWeapon::ServerStartFire_Validate()
{
	return true;
}

void ASBeamWeapon::ServerStopFire_Implementation()
{
	StopFire();
}

void ASBeamWeapon::ServerStartFire_Implementation()
{
	StartFire();
}

void ASBeamWeapon::ApplyBeamStatus()
{
	if (!BeamEffect)
	{
		return;
	}

	if (BeamParticleComponent)
	{
		if (BeamStatus.bIsActive)
		{
			BeamParticleComponent->SetHiddenInGame(false);
			BeamParticleComponent->SetFloatParameter("BeamLength", HitDistance);
		}

		if (!BeamStatus.bIsActive)
		{
			BeamParticleComponent->SetHiddenInGame(true);
		}

		if (BeamStatus.Target != FVector::ZeroVector)
		{
			BeamParticleComponent->SetVectorParameter(TracerTargetName, BeamStatus.Target);
		}
	}
}

void ASBeamWeapon::OnRep_BeamStatus()
{
	ApplyBeamStatus();

	FBeamStatus Stat;
	Stat.bIsActive = BeamStatus.bIsActive;
	Stat.Target = BeamStatus.Target;

	OnBeamStatusChange(Stat);
}

void ASBeamWeapon::Tick(float DeltaTime)
{
	if (BeamStatus.bIsActive)
	{
		FHitResult Hit;
		Hitscan(Hit);
		FVector ShotEnd = Hit.bBlockingHit ? Hit.ImpactPoint : Hit.TraceEnd;

		if (BeamParticleComponent)
		{
			BeamParticleComponent->SetFloatParameter("BeamLength", Hit.Distance);
			FVector ProjectileLocation = MeshComp->GetSocketLocation(BeamSocket);
			FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(ProjectileLocation, ShotEnd);
			BeamParticleComponent->SetWorldRotation(TargetRot);
		}

		BeamStatus.Target = Hit.TraceEnd;

		ApplyBeamStatus();
	}
}

void ASBeamWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASBeamWeapon, BeamStatus, COND_SkipOwner);
}
