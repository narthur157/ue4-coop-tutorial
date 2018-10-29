#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPowerupActor.generated.h"

class ASCharacter;

UCLASS()
class COOPGAME_API ASPowerupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ASPowerupActor();

protected:
	virtual void BeginPlay() override;

	FTimerHandle TimerHandle_PowerupTick;

	UFUNCTION()
	void OnTickPowerup();

	int32 TicksProcessed;

	/* Time between powerup application */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	float PowerupInterval;

	/* Total times we apply the powerup*/
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	int32 TotalNumTicks;

	UPROPERTY(ReplicatedUsing=OnRep_PowerupActive)
	bool bIsPowerupActive;

	UFUNCTION()
	void OnRep_PowerupActive();

	UFUNCTION(BlueprintImplementableEvent)
	void OnPowerupStateChanged(bool bNewIsActive);

	/**
	 * The actor who picked up this powerup
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Powerups")
	ASCharacter* ActivatedFor;


public:	
	void ActivatePowerup(ASCharacter* ActivateFor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerupTicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnExpired();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
