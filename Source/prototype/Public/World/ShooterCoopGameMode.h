// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "World/ShooterGameMode.h"
#include "ShooterCoopGameMode.generated.h"


enum class EWaveState : uint8;

/**
 * 
 */
UCLASS(ABSTRACT)
class PROTOTYPE_API AShooterCoopGameMode : public AShooterGameMode
{
	GENERATED_BODY()
	
	AShooterCoopGameMode();

	/* End the match when all players are dead */
	void CheckMatchEnd();

	virtual void StartMatch() override;

	/* End the match, with a delay before returning to the main menu */
	void FinishMatch();

	virtual void OnWaveEnded() override;

	EWaveState WaveState;

	/* Spawn at team player if any are alive */
	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	bool bSpawnAtTeamPlayer;

	virtual void Killed(AController* Killer, AController* VictimPlayer, APawn* VictimPawn, const UDamageType* DamageType) override;

	/************************************************************************/
	/* Scoring                                                              */
	/************************************************************************/

	/* Points awarded for surviving a night */
	UPROPERTY(EditDefaultsOnly, Category = "Scoring")
	int32 ScoreWaveSurvived;

public:
	/* Spawn the player next to his living coop buddy instead of a PlayerStart */
	virtual void RestartPlayer(class AController* NewPlayer) override;


protected:

	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	float TimeBeforeGameStart;

	FTimerHandle TimerHandle_GameStart;

	FTimerHandle TimerHandle_BotSpawner;

	FTimerHandle TimerHandle_NextWaveStart;

	// Bots to spawn in current wave
	int32 NrOfBotsToSpawn;

	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	int32 MaxWaveCount;

	int32 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	float TimeBetweenWaves;

	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	int32 PlayerRespawnCount;

protected:

	void SpawnBotTimerElapsed();

	// Start Spawning Bots
	void StartWave();

	// Stop Spawning Bots
	void EndWave();

	// Set timer for next startwave
	void PrepareForNextWave();

	void CheckWaveState();

	void GameOver();

	void SetWaveState(EWaveState NewState);

	void RestartDeadPlayers();

public:

	//virtual void StartPlay() override;

	//virtual void Tick(float DeltaSeconds) override;
};
