// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGrenadeProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UParticleSystem;
class USoundBase;

UCLASS()
class COOPGAME_API ASGrenadeProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASGrenadeProjectile();

protected:
	FTimerHandle ExplodeTimer;

	UFUNCTION(BlueprintImplementableEvent, Category = "Damage")
	void OnExplosion(FVector Loc, float radius);

	UFUNCTION()
	void Explode();

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	USoundBase* ExplosionSound;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ProjectileSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float TimeToDetonate;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UParticleSystem* ImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float Damage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float DamageRadius;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	UProjectileMovementComponent* MoveComp;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	USphereComponent* CollisionComp;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
