// Fill out your copyright notice in the Description page of Project Settings.


#include "World/ShooterGameMode.h"
#include "ShooterPlayerController.h"
#include "ShooterPlayerState.h"
#include "World/ShooterGameState.h"
#include "ShooterCharacter.h"
#include "UI/ShooterHUD.h"
#include "../ShooterTypes.h"
#include "ShooterSpectatorPawn.h"
#include "AI/ShooterZombieAIController.h"
#include "AI/ShooterZombieCharacter.h"
#include "World/ShooterPlayerStart.h"
#include "Mutators/ShooterMutator.h"
#include "ShooterWeapon.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LevelScriptActor.h"
#include "../prototype.h"


AShooterGameMode::AShooterGameMode()
{
	/* Assign the class types used by this gamemode */
	PlayerControllerClass = AShooterPlayerController::StaticClass();
	PlayerStateClass = AShooterPlayerState::StaticClass();
	GameStateClass = AShooterGameState::StaticClass();
	SpectatorClass = AShooterSpectatorPawn::StaticClass();

	BotPawnClass = AShooterZombieCharacter::StaticClass();

	bAllowFriendlyFireDamage = false;
	bSpawnZombiesAtNight = true;

	/* Start the game at 00:00 */
	TimeOfSecondStart = 0;

	BotSpawnInterval = 5.0f;

	/* Default team is 1 for players and 0 for enemies */
	PlayerTeamNum = 1;

	// You may want to make this number dynamic as players survived multiple nights
	MaxPawnsInZone = 20;
}


void AShooterGameMode::InitGameState()
{
	Super::InitGameState();

	AShooterGameState* MyGameState = Cast<AShooterGameState>(GameState);
	if (MyGameState)
	{
		MyGameState->ElapsedGameSeconds = TimeOfSecondStart;
	}
}


void AShooterGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	/* Set timer to run every second */
	GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &AShooterGameMode::DefaultTimer,
	                                GetWorldSettings()->GetEffectiveTimeDilation(), true);
}


void AShooterGameMode::StartMatch()
{
	if (!HasMatchStarted())
	{
		/* Spawn a new bot every 5 seconds (bothandler will opt-out based on his own rules for example to only spawn during night time) */
		//GetWorldTimerManager().SetTimer(TimerHandle_BotSpawns, this, &AShooterGameMode::SpawnBotHandler, BotSpawnInterval, true);
	}

	Super::StartMatch();
}


void AShooterGameMode::DefaultTimer()
{
	/* Immediately start the match while playing in editor */
	//if (GetWorld()->IsPlayInEditor())
	{
		if (GetMatchState() == MatchState::WaitingToStart)
		{
			StartMatch();
		}
	}

	/* Only increment time of day while game is active */
	if (IsMatchInProgress())
	{
		AShooterGameState* MyGameState = Cast<AShooterGameState>(GameState);
		if (MyGameState)
		{
			/* Increment our time of day */
			MyGameState->ElapsedGameSeconds += MyGameState->GetTimeOfSecondIncrement();

			///* Determine our state */
			//MyGameState->GetAndUpdateIsNight();

			///* Trigger events when night starts or ends */
			//bool CurrentIsNight = MyGameState->GetIsNight();
			//if (CurrentIsNight != LastIsNight)
			//{
			//	EHUDMessage MessageID =
			//		CurrentIsNight ? EHUDMessage::Game_SurviveStart : EHUDMessage::Game_SurviveEnded;
			//	MyGameState->BroadcastGameMessage(MessageID);

			//	/* The night just ended, respawn all dead players */
			//	if (!CurrentIsNight)
			//	{
			//		OnWaveEnded();
			//	}

			//	/* Update bot states */
			//	// 				if (CurrentIsNight)
			//	// 				{
			//	// 					WakeAllBots();
			//	// 				}
			//	// 				else
			//	// 				{
			//	// 					PassifyAllBots();
			//	// 				}
			//}

			//LastIsNight = MyGameState->bIsNight;
		}
	}
}


bool AShooterGameMode::CanDealDamage(class AShooterPlayerState* DamageCauser,
                                     class AShooterPlayerState* DamagedPlayer) const
{
	if (bAllowFriendlyFireDamage)
	{
		return true;
	}

	/* Allow damage to self */
	if (DamagedPlayer == DamageCauser)
	{
		return true;
	}

	// Compare Team Numbers
	return DamageCauser && DamagedPlayer && (DamageCauser->GetTeamNumber() != DamagedPlayer->GetTeamNumber());
}


FString AShooterGameMode::InitNewPlayer(class APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
                                        const FString& Options, const FString& Portal)
{
	FString Result = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	AShooterPlayerState* NewPlayerState = Cast<AShooterPlayerState>(NewPlayerController->PlayerState);
	if (NewPlayerState)
	{
		NewPlayerState->SetTeamNumber(PlayerTeamNum);
	}

	return Result;
}


float AShooterGameMode::ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent,
                                     AController* EventInstigator, AActor* DamageCauser) const
{
	float ActualDamage = Damage;

	AShooterBaseCharacter* DamagedPawn = Cast<AShooterBaseCharacter>(DamagedActor);
	if (DamagedPawn && EventInstigator)
	{
		AShooterPlayerState* DamagedPlayerState = Cast<AShooterPlayerState>(DamagedPawn->GetPlayerState());
		AShooterPlayerState* InstigatorPlayerState = Cast<AShooterPlayerState>(EventInstigator->PlayerState);

		// Check for friendly fire
		if (!CanDealDamage(InstigatorPlayerState, DamagedPlayerState))
		{
			ActualDamage = 0.f;
		}
	}

	return ActualDamage;
}


bool AShooterGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	/* Always pick a random location */
	return false;
}


AActor* AShooterGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> PreferredSpawns;
	TArray<APlayerStart*> FallbackSpawns;

	/* Get all playerstart objects in level */
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

	/* Split the player starts into two arrays for preferred and fallback spawns */
	for (int32 i = 0; i < PlayerStarts.Num(); i++)
	{
		APlayerStart* TestStart = Cast<APlayerStart>(PlayerStarts[i]);

		if (TestStart && IsSpawnpointAllowed(TestStart, Player))
		{
			if (IsSpawnpointPreferred(TestStart, Player))
			{
				PreferredSpawns.Add(TestStart);
			}
			else
			{
				FallbackSpawns.Add(TestStart);
			}
		}
	}

	/* Pick a random spawnpoint from the filtered spawn points */
	APlayerStart* BestStart = nullptr;
	if (PreferredSpawns.Num() > 0)
	{
		BestStart = PreferredSpawns[FMath::RandHelper(PreferredSpawns.Num())];
	}
	else if (FallbackSpawns.Num() > 0)
	{
		BestStart = FallbackSpawns[FMath::RandHelper(FallbackSpawns.Num())];
	}

	/* If we failed to find any (so BestStart is nullptr) fall back to the base code */
	return BestStart ? BestStart : Super::ChoosePlayerStart_Implementation(Player);
}


bool AShooterGameMode::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Controller)
{
	if (Controller == nullptr || Controller->PlayerState == nullptr)
		return true;

	/* Check for extended playerstart class */
	AShooterPlayerStart* MyPlayerStart = Cast<AShooterPlayerStart>(SpawnPoint);
	if (MyPlayerStart)
	{
		return MyPlayerStart->GetIsPlayerOnly() && !Controller->PlayerState->IsABot();
	}

	/* Cast failed, Anyone can spawn at the base playerstart class */
	return true;
}


bool AShooterGameMode::IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Controller)
{
	if (SpawnPoint)
	{
		/* Iterate all pawns to check for collision overlaps with the spawn point */
		const FVector SpawnLocation = SpawnPoint->GetActorLocation();
		for (TActorIterator<APawn> It(GetWorld()); It; ++It)
		{
			ACharacter* OtherPawn = Cast<ACharacter>(*It);
			if (OtherPawn)
			{
				const float CombinedHeight = (SpawnPoint->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() +
					OtherPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) * 2.0f;
				const float CombinedWidth = SpawnPoint->GetCapsuleComponent()->GetScaledCapsuleRadius() + OtherPawn->
					GetCapsuleComponent()->GetScaledCapsuleRadius();
				const FVector OtherLocation = OtherPawn->GetActorLocation();

				// Check if player overlaps the playerstart
				if (FMath::Abs(SpawnLocation.Z - OtherLocation.Z) < CombinedHeight && (SpawnLocation - OtherLocation).
					Size2D() < CombinedWidth)
				{
					return false;
				}
			}
		}

		/* Check if spawnpoint is exclusive to players */
		AShooterPlayerStart* MyPlayerStart = Cast<AShooterPlayerStart>(SpawnPoint);
		if (MyPlayerStart)
		{
			return MyPlayerStart->GetIsPlayerOnly() && !Controller->PlayerState->IsABot();
		}
	}

	return false;
}


void AShooterGameMode::SpawnNewBot()
{
	// Chance for Blueprint to pick a location (for example implementation see BP: SurvivalCoopGameMode asset)
	FTransform SpawnTransform;
	if (!FindBotSpawnTransform(SpawnTransform))
	{
		// This will fail unless blueprint has implemented this function to handle spawn locations
		UE_LOG(LogGame, Warning, TEXT("Failed to find bot spawn transform for SpawnNewBot."));
		return;
	}

	GetWorld()->SpawnActor<APawn>(BotPawnClass, SpawnTransform);
}

/* Used by RestartPlayer() to determine the pawn to create and possess when a bot or player spawns */
UClass* AShooterGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (Cast<AShooterZombieAIController>(InController))
	{
		return BotPawnClass;
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}


bool AShooterGameMode::CanSpectate_Implementation(APlayerController* Viewer, APlayerState* ViewTarget)
{
	/* Don't allow spectating of other non-player bots */
	return (ViewTarget && !ViewTarget->IsABot());
}


void AShooterGameMode::PassifyAllBots()
{
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		AShooterZombieCharacter* AIPawn = Cast<AShooterZombieCharacter>(*It);
		if (AIPawn)
		{
			AIPawn->SetBotType(EBotBehaviorType::Passive);
		}
	}
}


void AShooterGameMode::WakeAllBots()
{
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		AShooterZombieCharacter* AIPawn = Cast<AShooterZombieCharacter>(*It);
		if (AIPawn)
		{
			AIPawn->SetBotType(EBotBehaviorType::Patrolling);
		}
	}
}


void AShooterGameMode::SpawnBotHandler()
{
	AShooterGameState* MyGameState = Cast<AShooterGameState>(GameState);
	if (MyGameState)
	{
		int32 PawnsInWorld = 0;
		for (TActorIterator<APawn> It(GetWorld()); It; ++It)
		{
			++PawnsInWorld;
		}

		/* Check number of available pawns (players included) */
		if (PawnsInWorld < MaxPawnsInZone)
		{
			SpawnNewBot();
		}
	}
}


void AShooterGameMode::OnWaveEnded()
{
	// Do nothing (can be used to apply score or trigger other time of day events)
}

void AShooterGameMode::Killed(AController* Killer, AController* VictimPlayer, APawn* VictimPawn,
                              const UDamageType* DamageType)
{
	// Do nothing (can we used to apply score or keep track of kill count)
}


void AShooterGameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	SpawnDefaultInventory(PlayerPawn);
}


void AShooterGameMode::SpawnDefaultInventory(APawn* PlayerPawn)
{
	AShooterCharacter* MyPawn = Cast<AShooterCharacter>(PlayerPawn);
	if (MyPawn)
	{
		for (int32 i = 0; i < DefaultInventoryClasses.Num(); i++)
		{
			if (DefaultInventoryClasses[i])
			{
				FActorSpawnParameters SpawnInfo;
				SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				AShooterWeapon* NewWeapon = GetWorld()->SpawnActor<AShooterWeapon>(
					DefaultInventoryClasses[i], SpawnInfo);

				MyPawn->AddWeapon(NewWeapon);
			}
		}
	}
}


/************************************************************************/
/* Modding & Mutators                                                   */
/************************************************************************/


void AShooterGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	/* Spawn all mutators. */
	for (int32 i = 0; i < MutatorClasses.Num(); i++)
	{
		AddMutator(MutatorClasses[i]);
	}

	if (BaseMutator)
	{
		BaseMutator->InitGame(MapName, Options, ErrorMessage);
	}


	for (TActorIterator<AActor> It(GetWorld(), AActor::StaticClass()); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor->IsPendingKill())
		{
			// Some classes can't be removed via mutators
			bool bIsValidClass = !Actor->IsA(ALevelScriptActor::StaticClass()) && !Actor->IsA(
				AShooterMutator::StaticClass());
			// Static actors can't be removed.
			bool bIsRemovable = Actor->GetRootComponent() && Actor->GetRootComponent()->Mobility !=
				EComponentMobility::Static;

			if (bIsValidClass && bIsRemovable)
			{
				// a few type checks being AFTER the CheckRelevance() call is intentional; want mutators to be able to modify, but not outright destroy
				if (!CheckRelevance(Actor) && !Actor->IsA(APlayerController::StaticClass()))
				{
					/* Actors are destroyed if they fail the relevance checks */
					Actor->Destroy();
				}
			}
		}
	}

	Super::InitGame(MapName, Options, ErrorMessage);
}


bool AShooterGameMode::CheckRelevance_Implementation(AActor* Other)
{
	/* Execute the first in the mutator chain */
	if (BaseMutator)
	{
		return BaseMutator->CheckRelevance(Other);
	}

	return true;
}


void AShooterGameMode::AddMutator(TSubclassOf<AShooterMutator> MutClass)
{
	AShooterMutator* NewMut = GetWorld()->SpawnActor<AShooterMutator>(MutClass);
	if (NewMut)
	{
		if (BaseMutator == nullptr)
		{
			BaseMutator = NewMut;
		}
		else
		{
			// Add as child in chain
			BaseMutator->NextMutator = NewMut;
		}
	}
}
