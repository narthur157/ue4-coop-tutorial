#include "SPowerupActor.h"
#include "Net/UnrealNetwork.h"
#include "SCharacter.h"

// Sets default values
ASPowerupActor::ASPowerupActor()
{
	PowerupInterval = 0.0f;
	TotalNumTicks = 0;
	TicksProcessed = 0;
	bIsPowerupActive = false;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ASPowerupActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASPowerupActor::OnTickPowerup()
{
	TicksProcessed++;

	OnPowerupTicked();

	if (TicksProcessed >= TotalNumTicks)
	{
		OnExpired();

		bIsPowerupActive = false;
		OnRep_PowerupActive();
		
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void ASPowerupActor::OnRep_PowerupActive()
{
	OnPowerupStateChanged(bIsPowerupActive);
}

void ASPowerupActor::ActivatePowerup(ASCharacter* ActivateFor = nullptr)
{
	ActivatedFor = ActivateFor;
	OnActivated();

	bIsPowerupActive = true;
	OnRep_PowerupActive();

	if (PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerupActor::OnTickPowerup, PowerupInterval, true);
	}
	else
	{
		OnTickPowerup();
	}
}

void ASPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerupActor, bIsPowerupActive);
}
