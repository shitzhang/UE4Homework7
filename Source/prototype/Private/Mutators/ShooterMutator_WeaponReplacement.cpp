// Fill out your copyright notice in the Description page of Project Settings.


#include "Mutators/ShooterMutator_WeaponReplacement.h"
#include "World/ShooterGameMode.h"
#include "Items/ShooterWeaponPickup.h"


bool AShooterMutator_WeaponReplacement::CheckRelevance_Implementation(AActor* Other)
{
	AShooterWeaponPickup* WeaponPickup = Cast<AShooterWeaponPickup>(Other);
	if (WeaponPickup)
	{
		for (int32 i = 0; i < WeaponsToReplace.Num(); i++)
		{
			const FReplacementInfo& Info = WeaponsToReplace[i];

			if (Info.FromWeapon == WeaponPickup->WeaponClass)
			{
				WeaponPickup->WeaponClass = Info.ToWeapon;
			}
		}
	}

	/* Always call Super so we can run the entire chain of linked Mutators. */
	return Super::CheckRelevance_Implementation(Other);
}


void AShooterMutator_WeaponReplacement::InitGame_Implementation(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	/* Update default inventory weapons for current gamemode. */
	AShooterGameMode* GameMode = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode != nullptr)
	{
		for (int32 i = 0; i < GameMode->DefaultInventoryClasses.Num(); i++)
		{
			for (int32 j = 0; j < WeaponsToReplace.Num(); j++)
			{
				FReplacementInfo Info = WeaponsToReplace[j];
				if (GameMode->DefaultInventoryClasses[i] == Info.FromWeapon)
				{
					GameMode->DefaultInventoryClasses[i] = Info.ToWeapon;
				}
			}
		}
	}

	Super::InitGame_Implementation(MapName, Options, ErrorMessage);
}