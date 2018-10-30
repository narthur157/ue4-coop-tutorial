#include "SHordeGameState.h"
#include "Net/UnrealNetwork.h"

void ASHordeGameState::OnRep_WaveState(EWaveState OldState)
{
	WaveStateChanged(WaveState, OldState);
}

void ASHordeGameState::SetWaveState(EWaveState NewState)
{
	if (Role == ROLE_Authority)
	{
		EWaveState OldState = WaveState;
		WaveState = NewState;

		OnRep_WaveState(OldState);
	}
}

void ASHordeGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASHordeGameState, WaveState);
}
