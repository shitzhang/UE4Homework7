// Fill out your copyright notice in the Description page of Project Settings.


#include "World/ShooterGameState.h"
#include "ShooterPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "World/ShooterGameInstance.h"
#include "Net/UnrealNetwork.h"


AShooterGameState::AShooterGameState()
{
	/* 1 SECOND real time is 1*TimeScale MINUTES game time */
	TimeScale = 1.0f;
}


void AShooterGameState::SetTimeOfSecond(float NewSeconds)
{
	ElapsedGameSeconds = NewSeconds;
}


float AShooterGameState::GetTimeOfSecondIncrement() const
{
	return (GetWorldSettings()->GetEffectiveTimeDilation() * TimeScale);
}


int32 AShooterGameState::GetElapsedMinutes()
{
	const float SecondsInMinute = 60;
	const float ElapsedMinutes = ElapsedGameSeconds / SecondsInMinute;
	return FMath::FloorToInt(ElapsedMinutes);
}


int32 AShooterGameState::GetElapsedFullMinutesInSeconds()
{
	const int32 SecondsInMinute = 60;
	return GetElapsedMinutes() * SecondsInMinute;
}


int32 AShooterGameState::GetElapsedSecondsCurrentMinute()
{
	return ElapsedGameSeconds - GetElapsedFullMinutesInSeconds();
}


/* As with Server side functions, NetMulticast functions have a _Implementation body */
void AShooterGameState::BroadcastGameMessage_Implementation(EHUDMessage MessageID)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		AShooterPlayerController* MyController = Cast<AShooterPlayerController>(*It);
		if (MyController && MyController->IsLocalController())
		{
			MyController->ClientHUDMessage(MessageID);
		}
	}
}


void AShooterGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	if(PlayerState->IsABot())
	{
		return;
	}

	for (int32 i = 0; i < PlayerArray.Num(); i++)
	{
		if (PlayerArray[i] == PlayerState)
		{
			return;
		}
	}

	UShooterGameInstance* GI = GetWorld()->GetGameInstance<UShooterGameInstance>();
	if (ensure(GI))
	{
		GI->OnPlayerStateAdded.Broadcast(PlayerState);
	}
}


void AShooterGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	UShooterGameInstance* GI = GetWorld()->GetGameInstance<UShooterGameInstance>();
	if (ensure(GI))
	{
		GI->OnPlayerStateRemoved.Broadcast(PlayerState);
	}
}


void AShooterGameState::GetPlayers(TArray<APlayerState*>& Players)
{
	for (auto PlayerState : PlayerArray)
		Players.Add(PlayerState);
}

int32 AShooterGameState::GetLeftMatchSeconds()
{
	return MatchDuration - ElapsedGameSeconds;
}

int32 AShooterGameState::GetTotalScore()
{
	return TotalScore;
}


void AShooterGameState::AddScore(int32 Score)
{
	TotalScore += Score;
}


void AShooterGameState::OnRep_WaveState(EWaveState OldState)
{
	WaveStateChanged(WaveState, OldState);
}

void AShooterGameState::WaveStateChanged(EWaveState NewState, EWaveState OldState)
{
	EHUDMessage MessageID;

	switch (NewState)
	{
	case EWaveState::WaveInProgress:
		MessageID = EHUDMessage::Game_WaveStart;
		break;
	case EWaveState::WaitingToComplete:
		MessageID = EHUDMessage::Game_WaveEnded;
		break;
	case EWaveState::WaitingToStart:
		MessageID = EHUDMessage::Game_WaveComplete;
		break;
	default:
		MessageID = EHUDMessage::None;
	}

	BroadcastGameMessage(MessageID);
}


void AShooterGameState::SetWaveCount(int32 NewWaveCount)
{
	WaveCount = NewWaveCount;
}

void AShooterGameState::SetWaveState(EWaveState NewState)
{
	if (HasAuthority())
	{
		EWaveState OldState = WaveState;

		WaveState = NewState;
		// Call on server
		OnRep_WaveState(OldState);
	}
}

void AShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterGameState, ElapsedGameSeconds);
	DOREPLIFETIME(AShooterGameState, TotalScore);
	DOREPLIFETIME(AShooterGameState, WaveState);
	DOREPLIFETIME(AShooterGameState, WaveCount);
	DOREPLIFETIME(AShooterGameState, MatchDuration);
}
