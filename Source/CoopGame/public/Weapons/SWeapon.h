#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class UDamageType;
class UCameraShake;

USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TEnumAsByte <EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;

};

UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	ASWeapon();

protected:
	virtual void PlayTracerEffect(FVector TracerEndPoint);
	virtual void PlayImpactEffect(EPhysicalSurface SurfaceType, FVector Loc);
	virtual void PlayMuzzleEffect();
	virtual void PlaySoundEffect(FVector Loc);
	virtual bool Hitscan(FHitResult & Hit);

	void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin=0.0f))
	float BulletSpreadInDegrees;;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float HitDistance;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
		UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	TSubclassOf<UCameraShake> FireCamShake;
	
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	USoundBase* SoundEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* ImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerEffect;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	virtual void Fire();
	FTimerHandle TimerHandle_TimeBetweenShots;
	float LastFireTime;
	float TimeBetweenShots;

	/** RPM Bullets per minute **/
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	float RateOfFire;

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();
public:
	UFUNCTION(BlueprintCallable, Category = "Weapon")

	virtual void StartFire();
	virtual void StopFire();
};
