// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterBattleGameMode.h"
#include "GameFramework/GameState.h"
#include "ShooterGameState.generated.h"


UENUM(BlueprintType)
enum class EWaveState : uint8
{
	WaitingToStart,

	WaveInProgress,

	// No longer spawning new bots, waiting for players to kill remaining bots
	WaitingToComplete,

	WaveComplete,

	GameOver,
};


enum class EHUDMessage : uint8;

/**
 * 
 */
UCLASS()
class PROTOTYPE_API AShooterGameState : public AGameState
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	void GetPlayers(TArray<APlayerState*>& Players);

	/* Total accumulated score from all players  */
	UPROPERTY(Replicated)
	int32 TotalScore;

public:

	UPROPERTY(Replicated)
	int32 MatchDuration;

	UFUNCTION(BlueprintCallable)
	int32 GetLeftMatchSeconds();

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetTotalScore();

	void AddScore(int32 Score);

	AShooterGameState();


	/* Current time of day in the gamemode represented in full minutes */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "TimeOfDay")
	int32 ElapsedGameSeconds;

	/* Conversion of 1 second real time to X seconds gametime of the day/night cycle */
	UPROPERTY(EditDefaultsOnly, Category = "TimeOfDay")
	float TimeScale;

	/* Returns the time of day increment every real second (converted to accelerated game time, eg. 1 real second is 1 minute in time of day increment) */
	float GetTimeOfSecondIncrement() const;

	UFUNCTION(BlueprintCallable, Category = "TimeOfDay")
	int32 GetElapsedMinutes();

	/* Returns whole days elapsed, represented in minutes */
	UFUNCTION(BlueprintCallable, Category = "TimeOfDay")
	int32 GetElapsedFullMinutesInSeconds();

	int32 GetElapsedSecondsCurrentMinute();

	/* By passing in "exec" we expose it as a command line (press ~ to open) */
	UFUNCTION(exec)
	void SetTimeOfSecond(float NewHourOfDay);

	/* NetMulticast will send this event to all clients that know about this object, in the case of GameState that means every client. */
	UFUNCTION(Reliable, NetMulticast)
	void BroadcastGameMessage(EHUDMessage NewMessage);

	void BroadcastGameMessage_Implementation(EHUDMessage MessageID);

public:
	virtual void AddPlayerState(APlayerState* PlayerState) override;

	virtual void RemovePlayerState(APlayerState* PlayerState) override;
protected:
	UFUNCTION()
	void OnRep_WaveState(EWaveState OldState);

	void WaveStateChanged(EWaveState NewState, EWaveState OldState);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState, Category = "GameState")
	EWaveState WaveState;

	UPROPERTY(Replicated)
	int32 WaveCount;

public:
	void SetWaveCount(int32 NewWaveCount);

	UFUNCTION(BlueprintCallable)
	int32 GetWaveCount() const { return WaveCount; };

	void SetWaveState(EWaveState NewState);
};
