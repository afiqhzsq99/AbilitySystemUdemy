// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "ModMagnitudeCalculation.generated.h"

/**
 * 
 */
UCLASS()
class ABILITYSYSTEM_API UModMagnitudeCalculation : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UModMagnitudeCalculation();

	UProperty* HealthProperty;
	FGameplayEffectAttributeCaptureDefinition HealthDef;

	UProperty* MaxHealthProperty;
	FGameplayEffectAttributeCaptureDefinition MaxHealthDef;

	float CalculateBaseMagnitude_Implemetation(const FGameplayEffectSpec& Spec) const;
};
