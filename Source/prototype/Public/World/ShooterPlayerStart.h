// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE_API AShooterPlayerStart : public APlayerStart
{
	GENERATED_BODY()
	
public:

	AShooterPlayerStart(const FObjectInitializer& ObjectInitializer);

	/* Is only useable by players - automatically a preferred spawn for players */
	UPROPERTY(EditAnywhere, Category = "PlayerStart")
	bool bPlayerOnly;

public:

	UFUNCTION(BlueprintCallable, Category = "PlayerStart")
	bool GetIsPlayerOnly() const { return bPlayerOnly; }
};
