// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "ShooterHUD.generated.h"


/* Expose it to Blueprint using this tag */
UENUM(BlueprintType)
enum class EHUDState : uint8
{
	Playing,
	Spectating,
	MatchEnd
};


/**
 * 
 */
UCLASS()
class PROTOTYPE_API AShooterHUD : public AHUD
{
	GENERATED_BODY()

	AShooterHUD();

	FCanvasIcon CenterDotIcon;

	/* Current HUD state */
	EHUDState CurrentState;

	/** Main HUD update loop. */
	virtual void DrawHUD() override;

	void DrawCenterDot();

public:

	UFUNCTION(BlueprintCallable, Category = "HUD")
	EHUDState GetCurrentState() const;

	/* An event hook to call HUD text events to display in the HUD. Blueprint HUD class must implement how to deal with this event. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUDEvents")
	void MessageReceived(const FText& TextMessage);

	/* Event hook to update HUD state (eg. to determine visibility of widgets) */
	UFUNCTION(BlueprintNativeEvent, Category = "HUDEvents")
	void OnStateChanged(EHUDState NewState);
	
};
