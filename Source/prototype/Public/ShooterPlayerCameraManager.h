// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "ShooterPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE_API AShooterPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
	AShooterPlayerCameraManager();

	/* Update the FOV */
	virtual void UpdateCamera(float DeltaTime) override;

	float CurrentCrouchOffset;

	/* Maximum camera offset applied when crouch is initiated. Always lerps back to zero */
	float MaxCrouchOffsetZ;

	float CrouchLerpVelocity;

	bool bWasCrouched;

	/* Default relative Z offset of the player camera */
	float DefaultCameraOffsetZ;

	/* default, hip fire FOV */
	float NormalFOV;

	/* aiming down sight / zoomed FOV */
	float TargetingFOV;
};
