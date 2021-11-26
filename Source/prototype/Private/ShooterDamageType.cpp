// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterDamageType.h"



UShooterDamageType::UShooterDamageType()
{
	/* We apply this modifier based on the physics material setup to the head of the enemy PhysAsset */
	HeadDmgModifier = 2.0f;
	LimbDmgModifier = 0.5f;

	bCanDieFrom = true;
}


FString UShooterDamageType::GetDamageTypeDes() const
{
	return DamageTypeDes;
}

bool UShooterDamageType::GetCanDieFrom() const
{
	return bCanDieFrom;
}


float UShooterDamageType::GetHeadDamageModifier() const
{
	return HeadDmgModifier;
}

float UShooterDamageType::GetLimbDamageModifier() const
{
	return LimbDmgModifier;
}