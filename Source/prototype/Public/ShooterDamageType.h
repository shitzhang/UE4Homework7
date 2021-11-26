// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "ShooterDamageType.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE_API UShooterDamageType : public UDamageType
{
	GENERATED_BODY()
	
	UShooterDamageType();

	UPROPERTY(EditDefaultsOnly)
	FString DamageTypeDes;

	/* Can player die from this damage type (eg. players don't die from hunger) */
	UPROPERTY(EditDefaultsOnly)
	bool bCanDieFrom;

	/* Damage modifier for headshot damage */
	UPROPERTY(EditDefaultsOnly)
	float HeadDmgModifier;

	UPROPERTY(EditDefaultsOnly)
	float LimbDmgModifier;

public:

	FString GetDamageTypeDes() const;

	bool GetCanDieFrom() const;

	float GetHeadDamageModifier() const;

	float GetLimbDamageModifier() const;
};
