// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/ShooterHUD.h"
#include "World/ShooterGameMode.h"
#include "ShooterPlayerController.generated.h"


UENUM()
enum class EHUDMessage : uint8
{
	/* Weapons */
	Weapon_SlotTaken,

	/* Character */
	Character_EnergyRestored,

	/* Gamemode */
	Game_WaveStart,
	Game_WaveEnded,
	Game_WaveComplete,

	/* No category specified */
	None,
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FChatMessageReceived, class APlayerState*, Sender, const FString&, Message);

/**
 * 
 */
UCLASS()
class PROTOTYPE_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	AShooterPlayerController();

	/* Flag to respawn or start spectating upon death */
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	bool bRespawnImmediately;

	FTimerHandle TimerHandle_Respawn;

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	float PlayerRespawnInterval;

	/* Respawn or start spectating after dying */
	virtual void UnFreeze() override;

	void Respawn();

	UFUNCTION(BlueprintCallable)
	float GetRespawnLeftTime() const;

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerSuicide();
	void ServerSuicide_Implementation();
	bool ServerSuicide_Validate();

public:

	UFUNCTION(Reliable, Client)
	void ClientHUDStateChanged(EHUDState NewState);

	void ClientHUDStateChanged_Implementation(EHUDState NewState);

	/* Enum is remapped to localized text before sending it to the HUD */
	UFUNCTION(Reliable, Client)
	void ClientHUDMessage(EHUDMessage MessageID);

	void ClientHUDMessage_Implementation(EHUDMessage MessageID);

	FText GetText(EHUDMessage MsgID) const;

	/* Kill the current pawn */
	UFUNCTION(exec)
	virtual void Suicide();

	/* Start spectating. Should be called only on server */
	void StartSpectating();

	virtual void GameHasEnded(class AActor* EndGameFocus = nullptr, bool bIsWinner = false) override;

public:

	// -- CHAT -- //

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
	void ServerSendChatMessage(class APlayerState* Sender, const FString& Message);

	UFUNCTION(Client, Reliable)
	void ClientReceiveChatMessage(class APlayerState* Sender, const FString& Message);

	UPROPERTY(BlueprintAssignable)
	FChatMessageReceived OnChatMessageReceived;
};
