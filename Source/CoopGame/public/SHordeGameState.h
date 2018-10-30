// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SHordeGameState.generated.h"

UENUM(BlueprintType)
enum class EWaveState : uint8
{
	WaitingToStart,	/* Default state */
	PreparingNextWave,
	WaveInProgress,
	WaitingToComplete, /* No longer spawning bots, waiting for player to kill remaining bots */
	GameOver
};

UCLASS()
class COOPGAME_API ASHordeGameState : public AGameStateBase
{
	GENERATED_BODY()
	
protected:
	UFUNCTION()
	void OnRep_WaveState(EWaveState OldState);

	UFUNCTION(BlueprintImplementableEvent, Category = "GameState")
	void WaveStateChanged(EWaveState NewState, EWaveState OldState);

public:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_WaveState, Category = "GameState")
	EWaveState WaveState;

	void SetWaveState(EWaveState NewState);
	
};
