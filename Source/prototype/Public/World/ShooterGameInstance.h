// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ShooterGameInstance.generated.h"

class APlayerState;

// Event hook for any time a player is added/removed (triggers via GameState)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerArrayChanged, APlayerState*, PlayerState);

/**
 * 
 */
UCLASS()
class PROTOTYPE_API UShooterGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:


	// Note: Added event hooks here since GameState is spawned 'late' on clients via replication 
	// (making it hard to know when you can hook onto it in widgets) and GameInstance always exists.

	/* New Player joined (runs on clients and server) */
	UPROPERTY(BlueprintAssignable, Category = "Game|Events")
	FPlayerArrayChanged OnPlayerStateAdded;

	/* Existing Player left (runs on clients and server) */
	UPROPERTY(BlueprintAssignable, Category = "Game|Events")
	FPlayerArrayChanged OnPlayerStateRemoved;
};
