// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterWeapon.h"
#include "ShooterProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE_API AShooterProjectileWeapon : public AShooterWeapon
{
	GENERATED_BODY()

protected:

	AShooterProjectileWeapon();

	virtual void FireWeapon() override;

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerSpawnProjectile(FVector_NetQuantize MuzzleLocation, FRotator EyeRotation);
	void ServerSpawnProjectile_Implementation(FVector_NetQuantize MuzzleLocation, FRotator EyeRotation);
	bool ServerSpawnProjectile_Validate(FVector_NetQuantize MuzzleLocation, FRotator EyeRotation);

	UPROPERTY(EditDefaultsOnly, Category = "ProjectileWeapon")
	TSubclassOf<AActor> ProjectileClass;
	
};
