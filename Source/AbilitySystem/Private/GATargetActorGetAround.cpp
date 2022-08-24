// Fill out your copyright notice in the Description page of Project Settings.


#include "GATargetActorGetAround.h"
#include "GameFrameWork/Pawn.h"
#include "GameFrameWork/PlayerController.h"
#include "Abilities/GameplayAbility.h"

void AGATargetActorGetAround::StartTargeting(UGameplayAbility* Ability)
{
	OwningAbility = Ability;
	MasterPC = Cast<APlayerController>(Ability->GetOwningActorFromActorInfo()->GetInstigatorController());
}

void AGATargetActorGetAround::ConfirmTargetingAndContinue()
{
	APawn* OwningPawn = MasterPC->GetPawn();
	if (!OwningPawn)
	{
		TargetDataReadyDelegate.Broadcast(FGameplayAbilityTargetDataHandle());
		return;
	}

	FVector ViewLocation = OwningPawn->GetActorLocation();

	TArray<FOverlapResult> Overlaps;
	TArray<TWeakObjectPtr<AActor>> OverlapedActors;
	bool TraceComplex = false;

	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.bTraceComplex = TraceComplex;
	CollisionQueryParams.bReturnPhysicalMaterial = false;
	APawn* MasterPawn = MasterPC->GetPawn();
	if (MasterPawn)
	{
		CollisionQueryParams.AddIgnoredActor(MasterPawn->GetUniqueID());
	}

	bool TryOverlap = GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		ViewLocation, FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Radius),
		CollisionQueryParams);

	if (TryOverlap)
	{
		for (int32 i = 0; i < Overlaps.Num(); i++)
		{
			APawn* PawnOverlaped = Cast<APawn>(Overlaps[i].GetActor());
			if (PawnOverlaped && !OverlapedActors.Contains(PawnOverlaped))
			{
				OverlapedActors.Add(PawnOverlaped);
			}
		}
	}
	if (OverlapedActors.Num() > 0)
	{
		FGameplayAbilityTargetDataHandle TargetDate = StartLocation.MakeTargetDataHandleFromActors(OverlapedActors);
		TargetDataReadyDelegate.Broadcast(TargetDate);
	}
	else
	{
		TargetDataReadyDelegate.Broadcast(FGameplayAbilityTargetDataHandle());
	}
}