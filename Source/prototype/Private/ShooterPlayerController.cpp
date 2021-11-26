// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include "ShooterPlayerCameraManager.h"
#include "ShooterCharacter.h"
#include "UI/ShooterHUD.h"
#include "World/ShooterGameState.h"
#include "GameFramework/PlayerState.h"
#include "../prototype.h"


AShooterPlayerController::AShooterPlayerController()
{
	/* Assign the class types we wish to use */
	PlayerCameraManagerClass = AShooterPlayerCameraManager::StaticClass();

	/* Example - Can be set to true for debugging, generally a value like this would exist in the GameMode instead */
	bRespawnImmediately = false;

	PlayerRespawnInterval = 5.0f;
}


void AShooterPlayerController::UnFreeze()
{
	Super::UnFreeze();

	// Check if match is ending or has ended.
	AShooterGameState* MyGameState = GetWorld()->GetGameState<AShooterGameState>();
	if (MyGameState && MyGameState->HasMatchEnded())
	{
		/* Don't allow spectating or respawns */
		return;
	}

	/* Respawn or spectate */
	if (bRespawnImmediately)
	{
		Respawn();
	}
	else
	{
		StartSpectating();

		GetWorldTimerManager().SetTimer(TimerHandle_Respawn, this, &AShooterPlayerController::Respawn, PlayerRespawnInterval);
	}
}


void AShooterPlayerController::Respawn()
{
	AShooterGameState* MyGameState = GetWorld()->GetGameState<AShooterGameState>();
	if (MyGameState && MyGameState->HasMatchEnded())
	{
		/* Don't allow respawns */
		return;
	}
	ClientHUDStateChanged(EHUDState::Playing);

	ServerRestartPlayer();
}

float AShooterPlayerController::GetRespawnLeftTime() const
{
	float SpawnLeftTime = 0.0f;

	if (PlayerState->IsSpectator())
	{
		SpawnLeftTime = GetWorldTimerManager().GetTimerRemaining(TimerHandle_Respawn);
	}

	return FMath::Max(0.0f, SpawnLeftTime);
}


void AShooterPlayerController::StartSpectating()
{
	/* Update the state on server */
	PlayerState->SetIsSpectator(true);
	/* Waiting to respawn */
	bPlayerIsWaiting = true;
	ChangeState(NAME_Spectating);
	/* Push the state update to the client */
	ClientGotoState(NAME_Spectating);

	/* Focus on the remaining alive player */
	ViewAPlayer(1);

	/* Update the HUD to show the spectator screen */
	ClientHUDStateChanged(EHUDState::Spectating);
}


void AShooterPlayerController::GameHasEnded(AActor* EndGameFocus, bool bIsWinner)
{
	//Super::GameHasEnded(EndGameFocus, bIsWinner);

	// Cancel the repsawn timer
	GetWorldTimerManager().ClearTimer(TimerHandle_Respawn);
}


void AShooterPlayerController::Suicide()
{
	if (IsInState(NAME_Playing))
	{
		ServerSuicide();
	}
}

void AShooterPlayerController::ServerSuicide_Implementation()
{
	AShooterCharacter* MyPawn = Cast<AShooterCharacter>(GetPawn());
	if (MyPawn && ((GetWorld()->TimeSeconds - MyPawn->CreationTime > 1) || (GetNetMode() == NM_Standalone)))
	{
		MyPawn->Suicide();
	}
}


bool AShooterPlayerController::ServerSuicide_Validate()
{
	return true;
}


void AShooterPlayerController::ClientHUDStateChanged_Implementation(EHUDState NewState)
{
	AShooterHUD* HUD = Cast<AShooterHUD>(GetHUD());
	if (HUD)
	{
		HUD->OnStateChanged(NewState);
	}
}


void AShooterPlayerController::ClientHUDMessage_Implementation(EHUDMessage MessageID)
{
	/* Turn the ID into a message for the HUD to display */
	const FText TextMessage = GetText(MessageID);

	AShooterHUD* HUD = Cast<AShooterHUD>(GetHUD());
	if (HUD)
	{
		/* Implemented in SurvivalHUD Blueprint */
		HUD->MessageReceived(TextMessage);
	}
}


void AShooterPlayerController::ServerSendChatMessage_Implementation(class APlayerState* Sender, const FString& Message)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		AShooterPlayerController* PC = Cast<AShooterPlayerController>(Iterator->Get());
		if (PC)
		{
			PC->ClientReceiveChatMessage(Sender, Message);
		}
	}
}


void AShooterPlayerController::ClientReceiveChatMessage_Implementation(class APlayerState* Sender, const FString& Message)
{
	OnChatMessageReceived.Broadcast(Sender, Message);
}


bool AShooterPlayerController::ServerSendChatMessage_Validate(class APlayerState* Sender, const FString& Message)
{
	return true;
}


/* Temporarily set the namespace. If this was omitted, we should call NSLOCTEXT(Namespace, x, y) instead */
#define LOCTEXT_NAMESPACE "HUDMESSAGES"

FText AShooterPlayerController::GetText(EHUDMessage MsgID) const
{
	switch (MsgID)
	{
	case EHUDMessage::Weapon_SlotTaken:
		return LOCTEXT("WeaponSlotTaken", "Weapon slot already taken.");
	case EHUDMessage::Character_EnergyRestored:
		return LOCTEXT("CharacterEnergyRestored", "Energy Restored");
	case EHUDMessage::Game_WaveStart:
		return LOCTEXT("GameWaveStart", "Wave Start!");
	case EHUDMessage::Game_WaveEnded:
		return LOCTEXT("GameWaveEnd", "Beat the bots!");
	case EHUDMessage::Game_WaveComplete:
		return LOCTEXT("GameWaveComplete", "Wave survived! Prepare for the coming wave.");
	default:
		UE_LOG(LogGame, Warning, TEXT("No Message set for enum value in ShooterPlayerContoller::GetText(). "))
			return FText::FromString("No Message Set");
	}
}

/* Remove the namespace definition so it doesn't exist in other files compiled after this one. */
#undef LOCTEXT_NAMESPACE