// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterBaseCharacter.h"
#include "../ShooterTypes.h"
#include "ShooterCharacter.generated.h"



class UCameraComponent;
class USpringArmComponent;
class AShooterWeapon;
class AShooterUsableActor;
class USoundCue;

UCLASS()
class PROTOTYPE_API AShooterCharacter : public AShooterBaseCharacter
{
	GENERATED_BODY()

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/* Called every frame */
	virtual void Tick(float DeltaSeconds) override;

	/* Called to bind functionality to input */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void PawnClientRestart() override;

	/* Stop playing all montages */
	void StopAllAnimMontages();

	FRotator ControllerRotationBeforeFreelook;

	float LastNoiseLoudness;

	float LastMakeNoiseTime;

public:
	// Sets default values for this character's properties
	AShooterCharacter(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerPose")
	EPlayerPose PlayerPose;

	void DeterminPlayerPose();

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* PunchAnim;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool IsFiring() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsInitiatedJump() const;

	// Free the camera when holding left Alt.
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFreelooking;

	/* Check if pawn is allowed to fire weapon */
	bool CanFire() const;

	bool CanReload() const;

	/* MakeNoise hook to trigger AI noise emitting (Loudness between 0.0-1.0)  */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void MakePawnNoise(float Loudness);

	UFUNCTION(BlueprintCallable, Category = "AI")
	float GetLastNoiseLoudness();

	UFUNCTION(BlueprintCallable, Category = "AI")
	float GetLastMakeNoiseTime();

	FORCEINLINE UCameraComponent* GetCameraComponent()
	{
		return CameraComp;
	}

	/************************************************************************/
	/* Movement                                                             */
	/************************************************************************/

	virtual void MoveForward(float Value);
	virtual void MoveRight(float Value);

	void BeginFreelook();
	void EndFreelook();

	// input
	void BeginCrouch();
	void EndCrouch();

	virtual void SetSprinting(bool NewSprinting) override;

	// input
	void BeginSprint();
	void EndSprint();

	void SetIsJumping(bool NewJumping);

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerSetIsJumping(bool NewJumping);
	void ServerSetIsJumping_Implementation(bool NewJumping);
	bool ServerSetIsJumping_Validate(bool NewJumping);

	void BeginJump();

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	/************************************************************************/
	/* Targeting                                                            */
	/************************************************************************/

	// input
	void BeginTarget();
	void EndTarget();


	/************************************************************************/
	/* Object Interaction                                                   */
	/************************************************************************/

	/* Use the usable actor currently in focus, if any */
	virtual void Use();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUse();
	void ServerUse_Implementation();
	bool ServerUse_Validate();

	AShooterUsableActor* GetUsableInView() const;

	/*Max distance to use/focus on actors. */
	UPROPERTY(EditDefaultsOnly, Category = "ObjectInteraction")
	float MaxUseDistance;

	/* True only in first frame when focused on a new usable actor. */
	bool bHasNewFocus;

	AShooterUsableActor* FocusedUsableActor;

protected:

	/* Is character currently performing a jump action. Resets on landed.  */
	UPROPERTY(Transient, Replicated)
	bool bIsJumping;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100));
	float ZoomInterpSpeed;

	// Default FOV set during begin play
	float DefaultFOV;

protected:

	/* Attachpoint for active weapon/item in hands */
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
	FName WeaponAttachSocketName;

	/* Attachpoint for items carried on the belt/pelvis. */
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
	FName PelvisAttachSocketName;

	/* Attachpoint for primary weapons */
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
	FName SpineAttachSocketName;

	/* Attachpoint for secondary weapons */
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
	FName SpineAttachSecondarySocketName;

	/* Distance away from character when dropping inventory items. */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	float DropWeaponMaxDistance;

	bool bWantsToFire;
public:

	virtual FVector GetPawnViewLocation() const override;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void Punch();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerPunch();
	void ServerPunch_Implementation();
	bool ServerPunch_Validate();

	UPROPERTY(ReplicatedUsing = OnRep_Punch)
	bool bPendingPunch;

	void SimulatePunch();

	void StopSimulatePunch();

	FTimerHandle TimerHandle_StopPunch;

	UFUNCTION()
	void OnRep_Punch();

	bool CanPunch();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void Reload();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void NextWeapon();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void PrevWeapon();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void EquipPrimaryWeapon();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void EquipSecondaryWeapon();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void CloseWeapon();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void DropWeapon();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerDropWeapon();
	void ServerDropWeapon_Implementation();
	bool ServerDropWeapon_Validate();

	void EquipWeapon(AShooterWeapon* Weapon);

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerEquipWeapon(AShooterWeapon* Weapon);
	void ServerEquipWeapon_Implementation(AShooterWeapon* Weapon);
	bool ServerEquipWeapon_Validate(AShooterWeapon* Weapon);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	AShooterWeapon* GetCurrentWeapon() const;

	void SetCurrentWeapon(AShooterWeapon* newWeapon, AShooterWeapon* LastWeapon = nullptr);


	/* All weapons/items the player currently holds */
	UPROPERTY(Transient, Replicated)
	TArray<AShooterWeapon*> Inventory;

	/* Check if the specified slot is available, limited to one item per type (primary, secondary) */
	bool WeaponSlotAvailable(EInventorySlot CheckSlot);

	/* Return socket name for attachments (to match the socket in the character skeleton) */
	FName GetInventoryAttachPoint(EInventorySlot Slot) const;

	void DestroyInventory();

	/* OnRep functions can use a parameter to hold the previous value of the variable. Very useful when you need to handle UnEquip etc. */
	UFUNCTION()
	void OnRep_CurrentWeapon(AShooterWeapon* LastWeapon);

	void AddWeapon(AShooterWeapon* Weapon);

	void RemoveWeapon(AShooterWeapon* Weapon, bool bDestroy);

	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
	AShooterWeapon* CurrentWeapon;

	UPROPERTY()
	AShooterWeapon* PreviousWeapon;

	/* Update the weapon mesh to the newly equipped weapon, this is triggered during an anim montage.
	   NOTE: Requires an AnimNotify created in the Equip animation to tell us when to swap the meshes. */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SwapToNewWeaponMesh();

	/************************************************************************/
	/* Damage & Death                                                       */
	/************************************************************************/
public:

	virtual void OnDeath(float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser);

	virtual void Suicide();

	virtual void KilledBy(class APawn* EventInstigator);
};


