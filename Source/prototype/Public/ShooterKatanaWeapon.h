// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterWeapon.h"
#include "ShooterKatanaWeapon.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE_API AShooterKatanaWeapon : public AShooterWeapon
{
	GENERATED_BODY()

protected:

	AShooterKatanaWeapon();

	virtual void FireWeapon() override;

};
