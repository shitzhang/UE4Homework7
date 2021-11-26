// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

// This is NOT included by default in an empty project! It's required for replication and setting of the GetLifetimeReplicatedProps
#include "Net/UnrealNetwork.h"

/* Define a log category for error messages */
DEFINE_LOG_CATEGORY_STATIC(LogGame, Log, All);

#define SURFACE_DEFAULT				SurfaceType_Default
#define SURFACE_FLESHDEFAULT		SurfaceType1
#define SURFACE_FLESHVULNERABLE		SurfaceType2
#define SURFACE_ZOMBIEBODY			SurfaceType3
#define SURFACE_ZOMBIEHEAD			SurfaceType4
#define SURFACE_ZOMBIELIMB			SurfaceType5

#define COLLISION_WEAPON			ECC_GameTraceChannel1