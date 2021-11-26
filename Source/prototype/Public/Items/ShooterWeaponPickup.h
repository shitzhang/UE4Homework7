// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ShooterPickupActor.h"
#include "ShooterWeaponPickup.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class PROTOTYPE_API AShooterWeaponPickup : public AShooterPickupActor
{
	GENERATED_BODY()

	AShooterWeaponPickup();

public:

	/* Class to add to inventory when picked up */
	UPROPERTY(EditDefaultsOnly, Category = "WeaponClass")
	TSubclassOf<class AShooterWeapon> WeaponClass;

	virtual void OnUsed(APawn* InstigatorPawn) override;
	
};
