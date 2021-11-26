// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ShooterGrenadeProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "ShooterBaseCharacter.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AI/ShooterZombieCharacter.h"

// Sets default values
AShooterGrenadeProjectile::AShooterGrenadeProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AShooterGrenadeProjectile::OnHit);
	// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 1000.0f;
	ProjectileMovement->MaxSpeed = 1000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	ProjectileMovement->OnProjectileStop.AddDynamic(this, &AShooterGrenadeProjectile::OnStop);

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(CollisionComp);
	RadialForceComp->Radius = 200;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false; // Prevent component from ticking, and only use FireImpulse() instead
	RadialForceComp->bIgnoreOwningActor = true; // ignore self

	// Die after 3 seconds by default
	//InitialLifeSpan = 3.0f;

	SetReplicates(true);
}


void AShooterGrenadeProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                      FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && OtherComp->IsSimulatingPhysics())
	{
		//OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());

		UPhysicalMaterial* PhysMat = Hit.PhysMaterial.Get();

		if (PhysMat)
		{
			ProjectileMovement->Bounciness = PhysMat->Restitution;

			ProjectileMovement->Friction = PhysMat->Friction;
		}

		if (Cast<AShooterZombieCharacter>(OtherActor))
		{
			Explode();
		}
	}
}


void AShooterGrenadeProjectile::OnStop(const FHitResult& ImpactResult)
{
	Explode();
}


// Called when the game starts or when spawned
void AShooterGrenadeProjectile::BeginPlay()
{
	Super::BeginPlay();
}


void AShooterGrenadeProjectile::Explode_Implementation()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	RadialForceComp->FireImpulse();

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	//IgnoreActors.Add(this->GetInstigatorController()->GetCharacter());
	IgnoreActors.Add(this->GetOwner()->GetOwner());

	UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage, GetActorLocation(), ExplosionRadius, DamageType,
		IgnoreActors, this, this->GetInstigatorController(), true);

	Destroy();
}


bool AShooterGrenadeProjectile::Explode_Validate()
{
	return true;
}

// Called every frame
void AShooterGrenadeProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
