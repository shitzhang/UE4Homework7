// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ShooterUsableActor.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AShooterUsableActor::AShooterUsableActor()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComp;
}

void AShooterUsableActor::OnUsed(APawn* InstigatorPawn)
{
	// Nothing to do here...
}


void AShooterUsableActor::OnBeginFocus()
{
	// Used by custom PostProcess to render outlines
	MeshComp->SetRenderCustomDepth(true);
}


void AShooterUsableActor::OnEndFocus()
{
	// Used by custom PostProcess to render outlines
	MeshComp->SetRenderCustomDepth(false);
}

