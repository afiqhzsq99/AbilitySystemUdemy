// Fill out your copyright notice in the Description page of Project Settings.


#include "ModMagnitudeCalculation.h"
#include "AttributeSetBase.h"

UModMagnitudeCalculation::UModMagnitudeCalculation()
{
	//--> Get the attribute base captured from enemy 
	HealthProperty = FindFieldChecked<UProperty>(UAttributeSetBase::StaticClass(), GET_MEMBER_NAME_CHECKED(UAttributeSetBase, Health));
	HealthDef = FGameplayEffectAttributeCaptureDefinition(HealthProperty, EGameplayEffectAttributeCaptureSource::Target, true);

	MaxHealthProperty = FindFieldChecked<UProperty>(UAttributeSetBase::StaticClass(), GET_MEMBER_NAME_CHECKED(UAttributeSetBase, MaxHealth));
	MaxHealthDef = FGameplayEffectAttributeCaptureDefinition(HealthProperty, EGameplayEffectAttributeCaptureSource::Target, true);

	RelevantAttributesToCapture.Add(HealthDef);
	RelevantAttributesToCapture.Add(MaxHealthDef);
}

float UModMagnitudeCalculation::CalculateBaseMagnitude_Implemetation(const FGameplayEffectSpec& Spec) const
{
	// Gather the tags from the source and target as that can affect which buffs should be used
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float Health = 0.f;
	GetCapturedAttributeMagnitude(HealthDef, Spec, EvaluationParameters, Health);
	Health = FMath::Max<float>(Health, 0.0f);

	float MaxHealth = 0.f;
	GetCapturedAttributeMagnitude(HealthDef, Spec, EvaluationParameters, MaxHealth);
	MaxHealth = FMath::Max<float>(MaxHealth, 1.0f); // Avoid divide by zero

	float Reduction = -20.0f;
	if (Health / MaxHealth > 0.5f)
	{
		// Double the effect if the target has more than half their mana
		Reduction *= 2;
	}

	if (TargetTags->HasTagExact(FGameplayTag::RequestGameplayTag(FName("Status.WeakToPoisonMana"))))
	{
		// Double the effect if the target is weak to PoisonMana
		Reduction *= 2;
	}

	return Reduction;

	UE_LOG(LogTemp, Warning, TEXT("MMC working! %f"), Reduction);
}
