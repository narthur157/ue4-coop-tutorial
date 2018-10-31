#include "SHordeGameMode.h"
#include "TimerManager.h"
#include "SHealthComponent.h"
#include "CoopGame.h"
#include "Kismet/GameplayStatics.h"
#include "SHordeGameState.h"
#include "SPlayerState.h"

ASHordeGameMode::ASHordeGameMode()
{
	NumBotsToSpawn = 0;
	WaveCount = 0;
	TimeBetweenWaves = 2.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;

	GameStateClass = ASHordeGameState::StaticClass();
}

void ASHordeGameMode::StartWave()
{
	WaveCount++;

	SetWaveState(EWaveState::WaveInProgress);

	NumBotsToSpawn = 2 * WaveCount;
	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASHordeGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);
}

void ASHordeGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
	SetWaveState(EWaveState::WaitingToComplete);
}

void ASHordeGameMode::PrepareForNextWave()
{
	UGameplayStatics::PlaySound2D(GetWorld(), PrepareWaveSound);

	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASHordeGameMode::StartWave, TimeBetweenWaves, false);
	SetWaveState(EWaveState::PreparingNextWave);
}

void ASHordeGameMode::CheckWaveState()
{
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	if (NumBotsToSpawn > 0 || bIsPreparingForWave)
	{
		UE_LOG(LogHordeMode, Log, TEXT("Wave still in progress"));

		return;
	}

	bool bIsAnyBotAlive = false;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
	{
		APawn* TestPawn = It->Get();

		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			continue;
		}

		USHealthComponent* HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));

		if (HealthComp && HealthComp->GetHealth() > 0.0f && HealthComp->TeamNum != 0)
		{
			UE_LOG(LogHordeMode, Log, TEXT("Bot %s still alive"), *TestPawn->GetName());
			bIsAnyBotAlive = true;
			break;
		}
	}

	if (!bIsAnyBotAlive)
	{
		UE_LOG(LogHordeMode, Log, TEXT("All bots dead"));

		PrepareForNextWave();
	}
}

void ASHordeGameMode::CheckAnyPlayerAlive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		APlayerController* PC = It->Get();

		if (PC && PC->GetPawn())
		{
			APawn* MyPawn = PC->GetPawn();

			USHealthComponent* HealthComp = Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));

			if (ensure(HealthComp) && HealthComp->GetHealth() > 0.0f)
			{
				return;
			}
		}
	}

	GameOver();
}

void ASHordeGameMode::GameOver()
{
	EndWave();
	UE_LOG(LogHordeMode, Log, TEXT("Game over"));

}

void ASHordeGameMode::SetWaveState(EWaveState NewState)
{
	ASHordeGameState* GS = GetGameState<ASHordeGameState>();

	if (ensureAlways(GS))
	{
		GS->SetWaveState(NewState);
	}
}

void ASHordeGameMode::SpawnBotTimerElapsed() 
{

	if (NumBotsToSpawn > 0)
	{
		UE_LOG(LogHordeMode, Log, TEXT("SpawnBot"));
		SpawnNewBot();
	}

	NumBotsToSpawn--;

	if (NumBotsToSpawn <= 0)
	{
		EndWave();
	}
}

void ASHordeGameMode::StartPlay()
{
	Super::StartPlay();
	PrepareForNextWave();
}

void ASHordeGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();
	CheckAnyPlayerAlive();
}
