#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class USHealthComponent;
class USoundBase;
class URadialForceComponent;

UCLASS()
class COOPGAME_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	ASExplosiveBarrel();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing=ExplodeEffects)
	bool bDied = false;

	UFUNCTION()
	void ExplodeEffects();

	void Explode();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URadialForceComponent* ForceComp;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* PreExplosionMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* PostExplosionMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	USoundBase* ExplosionSound;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float ImpulseStrength;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float BarrelLaunchStrength;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USHealthComponent* HealthComp;
};
