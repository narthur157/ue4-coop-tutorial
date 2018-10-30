#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SHordeGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilledSignature, AActor*, VictimActor, AActor*, KillerActor, AController*, KillerController);

class USoundBase;
enum class EWaveState : uint8;
class ASCharacter;
class USHealthComponent;

UCLASS()
class COOPGAME_API ASHordeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASHordeGameMode();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();
	void SpawnBotTimerElapsed();
	void StartWave();
	void EndWave();
	void PrepareForNextWave();
	void CheckWaveState();
	void CheckAnyPlayerAlive();
	void GameOver();
	void SetWaveState(EWaveState NewState);

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	USoundBase* PrepareWaveSound;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TimeBetweenWaves;

	FTimerHandle TimerHandle_BotSpawner;
	FTimerHandle TimerHandle_NextWaveStart;

	int NumBotsToSpawn;
	int WaveCount;

public:
	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnActorKilledSignature OnActorKilled;
};
