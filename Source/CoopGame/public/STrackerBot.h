#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

class USHealthComponent;
class USphereComponent;
class USoundCue;
class USCountNearbyActorComponent;

UCLASS()
class COOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	ASTrackerBot(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

	void SelfDestruct();

	UPROPERTY(VisibleAnywhere, Category = "TrackerBot")
	USCountNearbyActorComponent* ActorTrackComp;

	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, 
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void OnPowerLevelChanged(int32 NearbyActors);

	UFUNCTION()
	void DamageSelf();

	FTimerHandle TimerHandle_SelfDamage;


	float LastNavCalcTime;

	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float NavRecalcInterval;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MaxSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float DamageRadius;

	UPROPERTY(EditAnywhere, Category = "TrackerBot")
	float SelfHarmInterval;

	bool bExploded;
	bool bStartedSelfDestruct;
	int32 PowerLevel;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float SwarmRadius;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	int32 MaxPowerLevel;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float NearbyActorPowerScaling;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	USoundCue* ExplodeSound;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	USoundCue* SelfDestructSound;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* DamageSphereComp;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USHealthComponent* HealthComp;

	UMaterialInstanceDynamic* MatInst;

	FVector GetNextPathPoint();

	FVector NextPathPoint;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bUseVelocityChange;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RequiredDistanceToTarget;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

public:	
	virtual void Tick(float DeltaTime) override;
};
