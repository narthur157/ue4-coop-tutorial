// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SGrenadeWeapon.generated.h"

class ASGrenadeProjectile;
class UProjectileMovementComponent;

UCLASS()
class COOPGAME_API ASGrenadeWeapon : public ASWeapon
{
	GENERATED_BODY()

public:
	ASGrenadeWeapon();

protected:
	virtual void Fire() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<ASGrenadeProjectile> ProjectileClass;

	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName LaunchProjectileSocket;

};
