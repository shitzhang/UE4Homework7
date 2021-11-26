// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "../prototype.h"
#include "Components/ShooterMovementComponent.h"
#include "ShooterWeapon.h"
#include "Net/UnrealNetwork.h"
#include "Items/ShooterUsableActor.h"
#include "Items/ShooterWeaponPickup.h"
#include "Sound/SoundCue.h"

// Sets default values
AShooterCharacter::AShooterCharacter(const class FObjectInitializer& ObjectInitializer)
/* Override the movement class from the base class to our own to support multiple speeds (eg. sprinting) */
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UShooterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bUsePawnControlRotation = true;

	// Enable crouching
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	/* Ignore this channel or it will absorb the trace impacts instead of the skeletal mesh */
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	ZoomedFOV = 65.0f;
	ZoomInterpSpeed = 20;

	WeaponAttachSocketName = "WeaponSocket";
	SpineAttachSocketName = "SpineSocket";
	SpineAttachSecondarySocketName = "SpineSecondarySocket";
	PelvisAttachSocketName = "PelvisSocket";

	MaxUseDistance = 500;
	DropWeaponMaxDistance = 100;
	bHasNewFocus = true;
	TargetingSpeedModifier = 0.5f;
	SprintingSpeedModifier = 2.5f;

	bPendingPunch = false;
}


void AShooterCharacter::SetSprinting(bool NewSprinting)
{
	if (bWantsToRun)
	{
		StopFire();
	}

	Super::SetSprinting(NewSprinting);
}


bool AShooterCharacter::IsFiring() const
{
	return CurrentWeapon && CurrentWeapon->GetCurrentState() == EWeaponState::Firing;
}


void AShooterCharacter::MakePawnNoise(float Loudness)
{
	if (HasAuthority())
	{
		/* Make noise to be picked up by PawnSensingComponent by the enemy pawns */
		MakeNoise(Loudness, this, GetActorLocation());
	}
	LastNoiseLoudness = Loudness;
	LastMakeNoiseTime = GetWorld()->GetTimeSeconds();
}


float AShooterCharacter::GetLastNoiseLoudness()
{
	return LastNoiseLoudness;
}


float AShooterCharacter::GetLastMakeNoiseTime()
{
	return LastMakeNoiseTime;
}


// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultFOV = CameraComp->FieldOfView;
}

void AShooterCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	DestroyInventory();
}


void AShooterCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.f)
	{
		const FRotator Rotation = bIsFreelooking ? GetActorRotation() : Controller->GetControlRotation();

		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0.f)
	{
		const FRotator Rotation = bIsFreelooking ? GetActorRotation() : Controller->GetControlRotation();

		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::BeginFreelook()
{
	bIsFreelooking = true;

	ControllerRotationBeforeFreelook = GetControlRotation();

	// Don't rotate when the controller rotates. Let that just affect the camera.
	//bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	//bUseControllerRotationRoll = false;
}

void AShooterCharacter::EndFreelook()
{
	bIsFreelooking = false;

	Controller->SetControlRotation(ControllerRotationBeforeFreelook);
	//bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	//bUseControllerRotationRoll = true;
}

/*
Performs ray-trace to find closest looked-at UsableActor.
*/
AShooterUsableActor* AShooterCharacter::GetUsableInView() const
{
	FVector CamLoc;
	FRotator CamRot;

	if (Controller == nullptr)
		return nullptr;

	Controller->GetPlayerViewPoint(CamLoc, CamRot);
	const FVector TraceStart = CamLoc;
	const FVector Direction = CamRot.Vector();
	const FVector TraceEnd = TraceStart + (Direction * MaxUseDistance);

	FCollisionQueryParams TraceParams(TEXT("TraceUsableActor"), true, this);
	TraceParams.bReturnPhysicalMaterial = false;

	/* Not tracing complex uses the rough collision instead making tiny objects easier to select. */
	TraceParams.bTraceComplex = false;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);

	return Cast<AShooterUsableActor>(Hit.GetActor());
}


void AShooterCharacter::Use()
{
	// Only allow on server. If called on client push this request to the server
	if (HasAuthority())
	{
		AShooterUsableActor* Usable = GetUsableInView();
		if (Usable)
		{
			Usable->OnUsed(this);
		}
	}
	else
	{
		ServerUse();
	}
}


void AShooterCharacter::ServerUse_Implementation()
{
	Use();
}


bool AShooterCharacter::ServerUse_Validate()
{
	return true;
}

void AShooterCharacter::BeginCrouch()
{
	if (IsSprinting())
	{
		SetSprinting(false);
	}

	Crouch();
}

void AShooterCharacter::EndCrouch()
{
	UnCrouch();
}

void AShooterCharacter::BeginTarget()
{
	SetTargeting(true);
}

void AShooterCharacter::EndTarget()
{
	SetTargeting(false);
}

void AShooterCharacter::BeginJump()
{
	SetIsJumping(true);
}


void AShooterCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode /*= 0*/)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	/* Check if we are no longer falling/jumping */
	if (PrevMovementMode == EMovementMode::MOVE_Falling &&
		GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Falling)
	{
		SetIsJumping(false);
	}
}

void AShooterCharacter::BeginSprint()
{
	SetSprinting(true);
}

void AShooterCharacter::EndSprint()
{
	SetSprinting(false);
}

bool AShooterCharacter::IsInitiatedJump() const
{
	return bIsJumping;
}

void AShooterCharacter::SetIsJumping(bool NewJumping)
{
	// Go to standing pose if trying to jump while crouched
	if (bIsCrouched && NewJumping)
	{
		UnCrouch();
	}
	else if (NewJumping != bIsJumping)
	{
		bIsJumping = NewJumping;

		if (bIsJumping)
		{
			/* Perform the built-in Jump on the character */
			Jump();
		}
	}

	if (!HasAuthority())
	{
		ServerSetIsJumping(NewJumping);
	}
}

void AShooterCharacter::ServerSetIsJumping_Implementation(bool NewJumping)
{
	SetIsJumping(NewJumping);
}


bool AShooterCharacter::ServerSetIsJumping_Validate(bool NewJumping)
{
	return true;
}


void AShooterCharacter::Punch()
{
	if (!HasAuthority())
	{
		ServerPunch();
		return;
	}
	if (CanPunch())
	{
		bPendingPunch = true;
		SimulatePunch();
	}
}


void AShooterCharacter::ServerPunch_Implementation()
{
	Punch();
}


bool AShooterCharacter::ServerPunch_Validate()
{
	return true;
}


void AShooterCharacter::SimulatePunch()
{
	float AnimDuration = 0.0f;

	if (PlayerPose == EPlayerPose::NoWeaponPose)
	{
		USkeletalMeshComponent* UseMesh = GetMesh();
		if (!UseMesh->GetAnimInstance()->Montage_IsPlaying(PunchAnim))
		{
			AnimDuration = PlayAnimMontage(PunchAnim, 1, "Start");
		}
	}

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(TimerHandle_StopPunch, this, &AShooterCharacter::StopSimulatePunch,
		                                AnimDuration, false);
	}
}

void AShooterCharacter::StopSimulatePunch()
{
	if (HasAuthority())
	{
		bPendingPunch = false;
	}

	if (PunchAnim)
	{
		StopAnimMontage(PunchAnim);
	}
}


void AShooterCharacter::OnRep_Punch()
{
	if (bPendingPunch)
	{
		SimulatePunch();
	}
	else
	{
		StopSimulatePunch();
	}
}


bool AShooterCharacter::CanPunch()
{
	bool bCanPunch = IsAlive();
	bool bPoseOKToPunch = (PlayerPose == EPlayerPose::NoWeaponPose);
	return (bCanPunch && bPoseOKToPunch && (!bPendingPunch));
}


void AShooterCharacter::StartFire()
{
	if (IsSprinting())
	{
		SetSprinting(false);
	}

	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void AShooterCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void AShooterCharacter::Reload()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartReload();
	}
}


void AShooterCharacter::DropWeapon()
{
	if (!HasAuthority())
	{
		ServerDropWeapon();
		return;
	}

	if (CurrentWeapon)
	{
		FVector CamLoc;
		FRotator CamRot;

		if (Controller == nullptr)
		{
			return;
		}

		/* Find a location to drop the item, slightly in front of the player.
			Perform ray trace to check for blocking objects or walls and to make sure we don't drop any item through the level mesh */
		Controller->GetPlayerViewPoint(CamLoc, CamRot);
		FVector SpawnLocation;
		FRotator SpawnRotation = CamRot;

		const FVector Direction = CamRot.Vector();
		const FVector TraceStart = GetActorLocation();
		const FVector TraceEnd = GetActorLocation() + (Direction * DropWeaponMaxDistance);

		/* Setup the trace params, we are only interested in finding a valid drop position */
		FCollisionQueryParams TraceParams;
		TraceParams.bTraceComplex = false;
		TraceParams.bReturnPhysicalMaterial = false;
		TraceParams.AddIgnoredActor(this);

		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldDynamic, TraceParams);

		/* Find farthest valid spawn location */
		if (Hit.bBlockingHit)
		{
			/* Slightly move away from impacted object */
			SpawnLocation = Hit.ImpactPoint + (Hit.ImpactNormal * 20);
		}
		else
		{
			SpawnLocation = TraceEnd;
		}

		/* Spawn the "dropped" weapon */
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AShooterWeaponPickup* NewWeaponPickup = GetWorld()->SpawnActor<AShooterWeaponPickup>(
			CurrentWeapon->WeaponPickupClass, SpawnLocation, FRotator::ZeroRotator, SpawnInfo);

		if (NewWeaponPickup)
		{
			/* Apply torque to make it spin when dropped. */
			UStaticMeshComponent* MeshComp = NewWeaponPickup->GetMeshComponent();
			if (MeshComp)
			{
				MeshComp->SetSimulatePhysics(true);
				MeshComp->AddTorqueInRadians(FVector(1, 1, 1) * 4000000);
			}
		}

		RemoveWeapon(CurrentWeapon, true);
	}

	DeterminPlayerPose();
}

void AShooterCharacter::ServerDropWeapon_Implementation()
{
	DropWeapon();
}

bool AShooterCharacter::ServerDropWeapon_Validate()
{
	return true;
}


// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//float TargetFOV = bIsTargeting ? ZoomedFOV : DefaultFOV;

	//float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

	//CameraComp->SetFieldOfView(NewFOV);

	if (bWantsToRun && !IsSprinting())
	{
		SetSprinting(true);
	}

	if (Controller && Controller->IsLocalController())
	{
		AShooterUsableActor* Usable = GetUsableInView();

		// End Focus
		if (FocusedUsableActor != Usable)
		{
			if (FocusedUsableActor)
			{
				FocusedUsableActor->OnEndFocus();
			}

			bHasNewFocus = true;
		}

		// Assign new Focus
		FocusedUsableActor = Usable;

		// Start Focus.
		if (Usable)
		{
			if (bHasNewFocus)
			{
				Usable->OnBeginFocus();
				bHasNewFocus = false;
			}
		}
	}
}


FVector AShooterCharacter::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

bool AShooterCharacter::CanFire() const
{
	/* Add your own checks here, for example non-shooting areas or checking if player is in an NPC dialogue etc. */
	return IsAlive();
}


bool AShooterCharacter::CanReload() const
{
	return IsAlive();
}


FName AShooterCharacter::GetInventoryAttachPoint(EInventorySlot Slot) const
{
	/* Return the socket name for the specified storage slot */
	switch (Slot)
	{
	case EInventorySlot::Hands:
		return WeaponAttachSocketName;
	case EInventorySlot::Primary:
		return SpineAttachSocketName;
	case EInventorySlot::Secondary:
		return PelvisAttachSocketName;
	default:
		// Not implemented.
		return "";
	}
}


void AShooterCharacter::DestroyInventory()
{
	if (!HasAuthority())
	{
		return;
	}

	for (int32 i = Inventory.Num() - 1; i >= 0; i--)
	{
		AShooterWeapon* Weapon = Inventory[i];
		if (Weapon)
		{
			RemoveWeapon(Weapon, true);
		}
	}
}


void AShooterCharacter::SetCurrentWeapon(AShooterWeapon* NewWeapon, AShooterWeapon* LastWeapon)
{
	/* Maintain a reference for visual weapon swapping */
	PreviousWeapon = LastWeapon;

	AShooterWeapon* LocalLastWeapon = nullptr;
	if (LastWeapon)
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != CurrentWeapon)
	{
		LocalLastWeapon = CurrentWeapon;
	}

	CurrentWeapon = NewWeapon;

	// UnEquip the current
	bool bHasPreviousWeapon = false;
	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnEquip();
		bHasPreviousWeapon = true;
	}

	if (NewWeapon)
	{
		NewWeapon->SetOwningPawn(this);
		/* Only play equip animation when we already hold an item in hands */
		NewWeapon->OnEquip(true);
	}

	/* NOTE: If you don't have an equip animation w/ animnotify to swap the meshes halfway through, then uncomment this to immediately swap instead */
	//SwapToNewWeaponMesh();
}


void AShooterCharacter::OnRep_CurrentWeapon(AShooterWeapon* LastWeapon)
{
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}


AShooterWeapon* AShooterCharacter::GetCurrentWeapon() const
{
	return CurrentWeapon;
}


void AShooterCharacter::EquipWeapon(AShooterWeapon* Weapon)
{
	/* Ignore if trying to equip already equipped weapon */
	if (Weapon == CurrentWeapon)
		return;

	if (HasAuthority())
	{
		SetCurrentWeapon(Weapon, CurrentWeapon);
	}
	else
	{
		ServerEquipWeapon(Weapon);
	}

	DeterminPlayerPose();
}


bool AShooterCharacter::ServerEquipWeapon_Validate(AShooterWeapon* Weapon)
{
	return true;
}


void AShooterCharacter::ServerEquipWeapon_Implementation(AShooterWeapon* Weapon)
{
	EquipWeapon(Weapon);
}


void AShooterCharacter::AddWeapon(AShooterWeapon* Weapon)
{
	if (Weapon && HasAuthority())
	{
		Weapon->OnEnterInventory(this);
		Inventory.AddUnique(Weapon);

		// Equip first weapon in inventory
		if (Inventory.Num() > 0 && CurrentWeapon == nullptr)
		{
			EquipWeapon(Inventory[0]);
		}
	}
}


void AShooterCharacter::RemoveWeapon(AShooterWeapon* Weapon, bool bDestroy)
{
	if (Weapon && HasAuthority())
	{
		bool bIsCurrent = CurrentWeapon == Weapon;

		if (Inventory.Contains(Weapon))
		{
			Weapon->OnLeaveInventory();
		}
		Inventory.RemoveSingle(Weapon);

		/* Replace weapon if we removed our current weapon */
		if (bIsCurrent && Inventory.Num() > 0)
		{
			SetCurrentWeapon(Inventory[0]);
		}

		/* Clear reference to weapon if we have no items left in inventory */
		if (Inventory.Num() == 0)
		{
			SetCurrentWeapon(nullptr);
		}

		if (bDestroy)
		{
			Weapon->Destroy();
		}
	}
}


void AShooterCharacter::NextWeapon()
{
	if (Inventory.Num() >= 2) // TODO: Check for weaponstate.
	{
		const int32 CurrentWeaponIndex = Inventory.IndexOfByKey(CurrentWeapon);
		AShooterWeapon* NextWeapon = Inventory[(CurrentWeaponIndex + 1) % Inventory.Num()];
		EquipWeapon(NextWeapon);
	}
}


void AShooterCharacter::PrevWeapon()
{
	if (Inventory.Num() >= 2) // TODO: Check for weaponstate.
	{
		const int32 CurrentWeaponIndex = Inventory.IndexOfByKey(CurrentWeapon);
		AShooterWeapon* PrevWeapon = Inventory[(CurrentWeaponIndex - 1 + Inventory.Num()) % Inventory.Num()];
		EquipWeapon(PrevWeapon);
	}
}


void AShooterCharacter::EquipPrimaryWeapon()
{
	if (Inventory.Num() >= 1)
	{
		/* Find first weapon that uses primary slot. */
		for (int32 i = 0; i < Inventory.Num(); i++)
		{
			AShooterWeapon* Weapon = Inventory[i];
			if (Weapon->GetStorageSlot() == EInventorySlot::Primary)
			{
				EquipWeapon(Weapon);
			}
		}
	}
}


void AShooterCharacter::EquipSecondaryWeapon()
{
	if (Inventory.Num() >= 1)
	{
		/* Find first weapon that uses secondary slot. */
		for (int32 i = 0; i < Inventory.Num(); i++)
		{
			AShooterWeapon* Weapon = Inventory[i];
			if (Weapon->GetStorageSlot() == EInventorySlot::Secondary)
			{
				EquipWeapon(Weapon);
			}
		}
	}
}

void AShooterCharacter::CloseWeapon()
{
	EquipWeapon(nullptr);
}

bool AShooterCharacter::WeaponSlotAvailable(EInventorySlot CheckSlot)
{
	/* Iterate all weapons to see if requested slot is occupied */
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		AShooterWeapon* Weapon = Inventory[i];
		if (Weapon)
		{
			if (Weapon->GetStorageSlot() == CheckSlot)
				return false;
		}
	}

	return true;

	/* Special find function as alternative to looping the array and performing if statements
		the [=] prefix means "capture by value", other options include [] "capture nothing" and [&] "capture by reference" */
	//return nullptr == Inventory.FindByPredicate([=](ASWeapon* W){ return W->GetStorageSlot() == CheckSlot; });
}


void AShooterCharacter::StopAllAnimMontages()
{
	USkeletalMeshComponent* UseMesh = GetMesh();
	if (UseMesh && UseMesh->AnimScriptInstance)
	{
		UseMesh->AnimScriptInstance->Montage_Stop(0.0f);
	}
}


void AShooterCharacter::DeterminPlayerPose()
{
	if (CurrentWeapon == nullptr)
	{
		PlayerPose = EPlayerPose::NoWeaponPose;
		return;
	}

	switch (CurrentWeapon->GetWeaponType())
	{
	case EWeaponType::Rifle:
		PlayerPose = EPlayerPose::RiflePose;
		break;
	case EWeaponType::Pistol:
		PlayerPose = EPlayerPose::PistolPose;
		break;
	case EWeaponType::Katana:
		PlayerPose = EPlayerPose::KatanaPose;
		break;
	default:
		PlayerPose = EPlayerPose::RiflePose;
	}
}

void AShooterCharacter::SwapToNewWeaponMesh()
{
	if (PreviousWeapon)
	{
		PreviousWeapon->AttachMeshToPawn(PreviousWeapon->GetStorageSlot());
	}

	if (CurrentWeapon)
	{
		CurrentWeapon->AttachMeshToPawn(EInventorySlot::Hands);
	}
}


void AShooterCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	/* Equip the weapon on the client side. */
	SetCurrentWeapon(CurrentWeapon);
}


void AShooterCharacter::OnDeath(float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator,
                                AActor* DamageCauser)
{
	if (bDied)
	{
		return;
	}

	DestroyInventory();
	StopAllAnimMontages();

	Super::OnDeath(KillingDamage, DamageEvent, PawnInstigator, DamageCauser);
}


void AShooterCharacter::Suicide()
{
	KilledBy(this);
}


void AShooterCharacter::KilledBy(class APawn* EventInstigator)
{
	if (HasAuthority() && !bDied)
	{
		AController* Killer = nullptr;
		if (EventInstigator != nullptr)
		{
			Killer = EventInstigator->Controller;
			LastHitBy = nullptr;
		}

		Die(GetHealth(), FDamageEvent(UDamageType::StaticClass()), Killer, nullptr);
	}
}


void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Value is already updated locally, skip in replication step
	DOREPLIFETIME_CONDITION(AShooterCharacter, bIsJumping, COND_SkipOwner);

	DOREPLIFETIME(AShooterCharacter, LastTakeHitInfo);

	DOREPLIFETIME(AShooterCharacter, CurrentWeapon);
	DOREPLIFETIME(AShooterCharacter, Inventory);
	DOREPLIFETIME(AShooterCharacter, PlayerPose);
	DOREPLIFETIME(AShooterCharacter, bPendingPunch);
	/* If we did not display the current inventory on the player mesh we could optimize replication by using this replication condition. */
	/* DOREPLIFETIME_CONDITION(ASCharacter, Inventory, COND_OwnerOnly);*/
}


void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Freelook", IE_Pressed, this, &AShooterCharacter::BeginFreelook);
	PlayerInputComponent->BindAction("Freelook", IE_Released, this, &AShooterCharacter::EndFreelook);

	PlayerInputComponent->BindAction("Sprinting", IE_Pressed, this, &AShooterCharacter::BeginSprint);
	PlayerInputComponent->BindAction("Sprinting", IE_Released, this, &AShooterCharacter::EndSprint);

	PlayerInputComponent->BindAction("Targeting", IE_Pressed, this, &AShooterCharacter::BeginTarget);
	PlayerInputComponent->BindAction("Targeting", IE_Released, this, &AShooterCharacter::EndTarget);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShooterCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AShooterCharacter::StopFire);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AShooterCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::BeginJump);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AShooterCharacter::Reload);

	PlayerInputComponent->BindAction("Use", IE_Pressed, this, &AShooterCharacter::Use);

	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, &AShooterCharacter::DropWeapon);

	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &AShooterCharacter::NextWeapon);
	PlayerInputComponent->BindAction("PrevWeapon", IE_Pressed, this, &AShooterCharacter::PrevWeapon);

	PlayerInputComponent->BindAction("EquipPrimaryWeapon", IE_Pressed, this, &AShooterCharacter::EquipPrimaryWeapon);
	PlayerInputComponent->BindAction("EquipSecondaryWeapon", IE_Pressed, this,
	                                 &AShooterCharacter::EquipSecondaryWeapon);

	PlayerInputComponent->BindAction("CloseWeapon", IE_Pressed, this, &AShooterCharacter::CloseWeapon);

	PlayerInputComponent->BindAction("Punch", IE_Pressed, this, &AShooterCharacter::Punch);
}
