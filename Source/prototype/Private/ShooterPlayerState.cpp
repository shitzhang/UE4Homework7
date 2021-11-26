// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerState.h"
#include "World/ShooterGameState.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"


AShooterPlayerState::AShooterPlayerState()
{
	/* AI will remain in team 0, players are updated to team 1 through the GameMode::InitNewPlayer */
	TeamNumber = 0;
}


void AShooterPlayerState::Reset()
{
	Super::Reset();

	NumKills = 0;
	NumDeaths = 0;
}

void AShooterPlayerState::AddKill()
{
	NumKills++;
}

void AShooterPlayerState::AddDeath()
{
	NumDeaths++;
}

void AShooterPlayerState::ScorePoints(int32 Points)
{
	SetScore(GetScore() + Points);

	/* Add the score to the global score count */
	AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>();
	if (GS)
	{
		GS->AddScore(Points);
	}
}


void AShooterPlayerState::SetTeamNumber(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;
}


int32 AShooterPlayerState::GetTeamNumber() const
{
	return TeamNumber;
}

int32 AShooterPlayerState::GetKills() const
{
	return NumKills;
}

int32 AShooterPlayerState::GetDeaths() const
{
	return NumDeaths;
}


void AShooterPlayerState::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterPlayerState, NumKills);
	DOREPLIFETIME(AShooterPlayerState, NumDeaths);
	DOREPLIFETIME(AShooterPlayerState, TeamNumber);
}
