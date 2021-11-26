// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterProjectileWeapon.h"

AShooterProjectileWeapon::AShooterProjectileWeapon()
{
	StorageSlot = EInventorySlot::Secondary;
	WeaponType = EWeaponType::Rifle;
}


void AShooterProjectileWeapon::FireWeapon()
{
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector MuzzleLocation = GetMuzzleLocation();

		ServerSpawnProjectile(MuzzleLocation, EyeRotation);
	}
}


void AShooterProjectileWeapon::ServerSpawnProjectile_Implementation(FVector_NetQuantize MuzzleLocation,
                                                                    FRotator EyeRotation)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Instigator = MyPawn;
	SpawnParams.Owner = this;

	GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams);
}


bool AShooterProjectileWeapon::ServerSpawnProjectile_Validate(FVector_NetQuantize MuzzleLocation, FRotator EyeRotation)
{
	return true;
}
