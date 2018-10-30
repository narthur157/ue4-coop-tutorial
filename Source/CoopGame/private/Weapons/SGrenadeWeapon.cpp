// Fill out your copyright notice in the Description page of Project Settings.

#include "SGrenadeWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SGrenadeProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

ASGrenadeWeapon::ASGrenadeWeapon()
{
	BaseDamage = 0.0f;
	LaunchProjectileSocket = "LaunchProjectileSocket";
}

void ASGrenadeWeapon::Fire() {
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	AActor* MyOwner = GetOwner();

	if (MyOwner)
	{
		PlayMuzzleEffect();
		PlaySoundEffect(GetActorLocation());

		FVector EyeLoc;
		FRotator EyeRot;
		MyOwner->GetActorEyesViewPoint(EyeLoc, EyeRot);

		FVector MuzzleLocation = MeshComp->GetSocketLocation(LaunchProjectileSocket);

		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		ActorSpawnParams.Instigator = Instigator ? Instigator : Cast<APawn>(GetOwner());
		
		if (Role == ROLE_Authority)
		{
			GetWorld()->SpawnActor<ASGrenadeProjectile>(ProjectileClass, MuzzleLocation, EyeRot, ActorSpawnParams);	
		}
	}
}


