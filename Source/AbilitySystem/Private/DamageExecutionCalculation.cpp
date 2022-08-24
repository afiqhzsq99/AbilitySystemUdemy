// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageExecutionCalculation.h"
#include "AttributeSetBase.h"

struct DamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackDamage)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor)
	DamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAttributeSetBase, AttackDamage, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAttributeSetBase, Armor, Target, true);
	}
};

static DamageStatics& GetDamageStatics()
{
	static DamageStatics DamageStaticsVar;
	return DamageStaticsVar;
}

UDamageExecutionCalculation::UDamageExecutionCalculation()
{
	HealthProperty = FindFieldChecked<UProperty>(UAttributeSetBase::StaticClass(), GET_MEMBER_NAME_CHECKED(UAttributeSetBase, Health));
	HealthDef = FGameplayEffectAttributeCaptureDefinition(HealthProperty, EGameplayEffectAttributeCaptureSource::Target, true);

	RelevantAttributesToCapture.Add(HealthDef);
	RelevantAttributesToCapture.Add(GetDamageStatics().AttackDamageDef);
	RelevantAttributesToCapture.Add(GetDamageStatics().ArmorDef);
}

void UDamageExecutionCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetTargetAbilitySystemComponent();

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	Spec.CapturedSourceTags;
	FAggregatorEvaluateParameters Params;

	float AttackDamageMagnitude = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetDamageStatics().AttackDamageDef, FAggregatorEvaluateParameters(), AttackDamageMagnitude);
	float ArmorMagnitude = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetDamageStatics().ArmorDef, FAggregatorEvaluateParameters(), ArmorMagnitude);
	
	//calculation
	float finalDamage = FMath::Clamp(AttackDamageMagnitude - ArmorMagnitude, 0.0f, AttackDamageMagnitude - ArmorMagnitude);
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(HealthProperty, EGameplayModOp::Additive, -100));
}