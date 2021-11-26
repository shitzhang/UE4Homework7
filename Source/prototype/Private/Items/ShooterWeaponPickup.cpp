// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ShooterWeaponPickup.h"
#include "ShooterCharacter.h"
#include "ShooterWeapon.h"
#include "ShooterPlayerController.h"


AShooterWeaponPickup::AShooterWeaponPickup()
{
	bAllowRespawn = false;

	/* Enabled to support simulated physics movement when weapons are dropped by a player */
	SetReplicateMovement(true);
}


void AShooterWeaponPickup::OnUsed(APawn* InstigatorPawn)
{
	AShooterCharacter* MyPawn = Cast<AShooterCharacter>(InstigatorPawn);
	if (MyPawn)
	{
		/* Fetch the default variables of the class we are about to pick up and check if the storage slot is available on the pawn. */
		if (MyPawn->WeaponSlotAvailable(WeaponClass->GetDefaultObject<AShooterWeapon>()->GetStorageSlot()))
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AShooterWeapon* NewWeapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, SpawnInfo);

			MyPawn->AddWeapon(NewWeapon);

			Super::OnUsed(InstigatorPawn);
		}
		else
		{
			AShooterPlayerController* PC = Cast<AShooterPlayerController>(MyPawn->GetController());
			if (PC)
			{
				PC->ClientHUDMessage(EHUDMessage::Weapon_SlotTaken);
			}
		}
	}
}


