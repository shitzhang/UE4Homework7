// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../ShooterTypes.h"
#include "ShooterBaseCharacter.generated.h"


class UPawnNoiseEmitterComponent;
class USoundCue;
class ADecalActor;


UCLASS()
class PROTOTYPE_API AShooterBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterBaseCharacter(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundCue* SoundTakeHit;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundCue* SoundDeath;

	UFUNCTION(BlueprintCallable, Category = "PlayerCondition")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerCondition")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerCondition")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	virtual bool IsSprinting() const;

	virtual void SetSprinting(bool NewSprinting);

	/* Server side call to update actual sprint state */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetSprinting(bool NewSprinting);
	void ServerSetSprinting_Implementation(bool NewSprinting);
	bool ServerSetSprinting_Validate(bool NewSprinting);

	float GetSprintingSpeedModifier() const;

	/************************************************************************/
	/* Targeting                                                            */
	/************************************************************************/

	void SetTargeting(bool NewTargeting);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetTargeting(bool NewTargeting);
	void ServerSetTargeting_Implementation(bool NewTargeting);
	bool ServerSetTargeting_Validate(bool NewTargeting);

	float GetTargetingSpeedModifier() const;

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	bool IsTargeting() const;

	/* Retrieve Pitch/Yaw from current camera */
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	FRotator GetAimOffsets() const;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintingSpeedModifier;

	/* Character wants to run, checked during Tick to see if allowed */
	UPROPERTY(Transient, Replicated)
	bool bWantsToRun;

	UPROPERTY(Transient, Replicated)
	bool bIsTargeting;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetingSpeedModifier;

protected:


	/* Tracks noise data used by the pawn sensing component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPawnNoiseEmitterComponent* NoiseEmitterComp;


	/************************************************************************/
	/* Damage & Death                                                       */
	/************************************************************************/
protected:
	UPROPERTY(EditDefaultsOnly, Category = "PlayerCondition", Replicated)
	float Health;

	/* Take damage & handle death */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	virtual bool CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const;

	virtual bool Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser);

	virtual void OnDeath(float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser);

	virtual void FellOutOfWorld(const class UDamageType& DmgType) override;

	void SetRagdollPhysics();

	virtual void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser, bool bKilled);

	void ReplicateHit(float DamageTaken, struct FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser, bool bKilled);

	/* Holds hit data to replicate hits and death to clients */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_LastTakeHitInfo)
	struct FTakeHitInfo LastTakeHitInfo;

	UFUNCTION()
	void OnRep_LastTakeHitInfo();

	bool bDied;

	// Footprint
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Footprint")
	TSubclassOf<AActor> RightFootprintDecal;

	UPROPERTY(EditDefaultsOnly, Category = "Footprint")
	TSubclassOf<AActor> LeftFootprintDecal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Footprint")
	UArrowComponent* RightFootArrowComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Footprint")
	UArrowComponent* LeftFootArrowComp;

	UPROPERTY(EditDefaultsOnly, Category = "Footprint")
	FName RightFootSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "Footprint")
	FName LeftFootSocketName;

	void TraceFootprint(FHitResult& OutHit, const FVector& Location) const;

	void SpawnFootprint(UArrowComponent* FootArrow, TSubclassOf<AActor> FootprintDecal) const;

public:
	UFUNCTION(BlueprintCallable, Category = "Footprint")
	void RightFootDown() const;

	UFUNCTION(BlueprintCallable, Category = "Footprint")
	void LeftFootDown() const;
};
