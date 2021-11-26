// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPowerupSpawner.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "ShooterPowerupActor.h"
#include "TimerManager.h"

// Sets default values
AShooterPowerupSpawner::AShooterPowerupSpawner()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(75.0f);
	RootComponent = SphereComp;

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetRelativeRotation(FRotator(90, 0.0f, 0.0f));
	DecalComp->DecalSize = FVector(64, 75, 75);
	DecalComp->SetupAttachment(RootComponent);

	CooldownDuration = 10.0f;

	SetReplicates(true);

}

// Called when the game starts or when spawned
void AShooterPowerupSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		Respawn();
	}
}

void AShooterPowerupSpawner::Respawn()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PowerUpInstance = GetWorld()->SpawnActor<AShooterPowerupActor>(PowerUpClass, GetTransform(), SpawnParams);
}

void AShooterPowerupSpawner::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (HasAuthority() && PowerUpInstance)
	{
		PowerUpInstance->ActivatePowerup(OtherActor);
		PowerUpInstance = nullptr;

		// Set Timer to respawn powerup
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &AShooterPowerupSpawner::Respawn, CooldownDuration);
	}
}

