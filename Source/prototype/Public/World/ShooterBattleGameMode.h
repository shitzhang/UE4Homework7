// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "World/ShooterGameMode.h"
#include "ShooterBattleGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE_API AShooterBattleGameMode : public AShooterGameMode
{
	GENERATED_BODY()

	AShooterBattleGameMode();

	/* End the match, with a delay before returning to the main menu */
	void FinishMatch();

	/* Match duration(seconds)*/
	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	int32 MatchDuration;

	/* Spawn at team player if any are alive */
	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	bool bSpawnAtTeamPlayer;

	FTimerHandle TimerHandle_FinishMatch;

	virtual void Killed(AController* Killer, AController* VictimPlayer, APawn* VictimPawn,
	                    const UDamageType* DamageType) override;

public:
	/* Spawn the player next to his living coop buddy instead of a PlayerStart */
	virtual void RestartPlayer(class AController* NewPlayer) override;

	UFUNCTION(BlueprintCallable)
	float GetMatchLeftTime();
protected:
	virtual void StartMatch() override;

	virtual void InitGameState() override;
};


