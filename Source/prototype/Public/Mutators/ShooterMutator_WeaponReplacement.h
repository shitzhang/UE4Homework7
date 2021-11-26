// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Mutators/ShooterMutator.h"
#include "ShooterWeapon.h"
#include "ShooterMutator_WeaponReplacement.generated.h"


USTRUCT(BlueprintType)
struct FReplacementInfo
{
	GENERATED_BODY()

public:

	/** class name of the weapon we want to get rid of */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AShooterWeapon> FromWeapon;
	/** fully qualified path of the class to replace it with */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AShooterWeapon> ToWeapon;

	FReplacementInfo()
		:FromWeapon(nullptr)
		, ToWeapon(nullptr)
	{
	}

	FReplacementInfo(TSubclassOf<AShooterWeapon> inOldClass, TSubclassOf<AShooterWeapon> inNewClass)
		: FromWeapon(inOldClass)
		, ToWeapon(inNewClass)
	{
	}

};


/**
 * Allows mutators to replace weapon pickups in the active level
 */
UCLASS(ABSTRACT)
class PROTOTYPE_API AShooterMutator_WeaponReplacement : public AShooterMutator
{
	GENERATED_BODY()

public:

	virtual void InitGame_Implementation(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual bool CheckRelevance_Implementation(AActor* Other) override;

	UPROPERTY(EditDefaultsOnly, Category = "WeaponReplacement")
	TArray<FReplacementInfo> WeaponsToReplace;
	
};
