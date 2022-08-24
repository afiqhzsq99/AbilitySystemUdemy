// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterBase.h"
#include "GameFramework/PlayerController.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "AttributeSetBase.h"
#include "GameplayAbilityBase.h"
#include "PlayerControllerBase.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComp");
	AttributeSetBaseComp = CreateDefaultSubobject<UAttributeSetBase>("AttributeSetBaseComp");
	bIsDead = false;
	TeamID = 255;
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	AttributeSetBaseComp->OnHealthChange.AddDynamic(this, &ACharacterBase::OnHealthChanged);
	AttributeSetBaseComp->OnManaChange.AddDynamic(this, &ACharacterBase::OnManaChanged);
	AttributeSetBaseComp->OnStrengthChange.AddDynamic(this, &ACharacterBase::OnStrengthChanged);
	AutoDeterminTeamIDbyControllerType();
	AddGameplayTag(FullHealthTag);
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}

//--> Read 1 ability
void ACharacterBase::AquireAbility(TSubclassOf<UGameplayAbility>AbilityToAquire)
{
	if(AbilitySystemComp)
	{
		if (HasAuthority() && AbilityToAquire)
		{
			AbilitySystemComp->GiveAbility(FGameplayAbilitySpec(AbilityToAquire, 1, 0));
		}
		AbilitySystemComp->InitAbilityActorInfo(this, this);
	}
}

//--> Add 1 ability to array & FX UI
void ACharacterBase::AquireAbilities(TArray<TSubclassOf<UGameplayAbility>>AbilityToAquire)
{
	for (TSubclassOf<UGameplayAbility> AbilityItem : AbilityToAquire)
	{
		AquireAbility(AbilityItem);
		if (AbilityItem->IsChildOf(UGameplayAbilityBase::StaticClass()))
		{
			TSubclassOf<UGameplayAbilityBase> AbilityBaseClass = *AbilityItem;
			if (AbilityBaseClass!=nullptr)
			{
				AddAbilityToUI(AbilityBaseClass);
			}
		}
	}
}

//--> Update health status
void ACharacterBase::OnHealthChanged(float Health, float MaxHealth)
{
	if (Health <= 0.0f && !bIsDead)
	{
		bIsDead = true;
		Dead();
		BP_Die();
	}
	BP_OnHealthChanged(Health, MaxHealth);
}

//--> Update mana status
void ACharacterBase::OnManaChanged(float Mana, float MaxMana)
{
	BP_OnManaChanged(Mana, MaxMana);
}

//--> Update stregth status
void ACharacterBase::OnStrengthChanged(float Strength, float MaxStrength)
{
	BP_OnStrengthChanged(Strength, MaxStrength);
}

//--> Update character team based on TeamID
bool ACharacterBase::IsOtherHostile(ACharacterBase* Other)
{
	return TeamID != Other->GetTeamID();
}

//--> Fx return TeamID
uint8 ACharacterBase::GetTeamID() const 
{
	return TeamID;
}

//--> Add tag to character
void ACharacterBase::AddGameplayTag(FGameplayTag& TagToAdd)
{
	GetAbilitySystemComponent()->AddLooseGameplayTag(TagToAdd);
	GetAbilitySystemComponent()->SetTagMapCount(TagToAdd, 1);
}

//--> Remove tag to character
void ACharacterBase::RemoveGameplayTag(FGameplayTag& TagToRemove)
{
	GetAbilitySystemComponent()->RemoveLooseGameplayTag(TagToRemove);
}

//--> Get TeamID
void ACharacterBase::AutoDeterminTeamIDbyControllerType()
{
	if (GetController() && GetController()->IsPlayerController())
	{
		TeamID = 0;
	}
}

//--> Update when character dead
void ACharacterBase::Dead()
{
	DisableInputControl();
}

//--> Disable control when player is dead
void ACharacterBase::DisableInputControl()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->DisableInput(PC);
	}
	AAIController* AIC = Cast<AAIController>(GetController());
	if (AIC)
	{
		AIC->GetBrainComponent()->StopLogic("Dead");
	}
}

//--> Enable control when player is alive
void ACharacterBase::EnableInputControl()
{
	if (!bIsDead)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			PC->EnableInput(PC);
		}
		AAIController* AIC = Cast<AAIController>(GetController());
		if (AIC)
		{
			AIC->GetBrainComponent()->RestartLogic();
		}
	}
}

//--> Add 1 ability to UI
void ACharacterBase::AddAbilityToUI(TSubclassOf<UGameplayAbilityBase> AbilityToAdd)
{
	APlayerControllerBase* PlayerControlerBase = Cast<APlayerControllerBase>(GetController());
	if (PlayerControlerBase)
	{
		UGameplayAbilityBase* AbililtyInstance = AbilityToAdd.Get()->GetDefaultObject<UGameplayAbilityBase>();
		if (AbililtyInstance)
		{
			FGameplayAbilityInfo AbilityInfo = AbililtyInstance->GetAbilityInfo();
			PlayerControlerBase->AddAbilityToUI(AbilityInfo);
		}
	}
}

//--> Make a stun
void ACharacterBase::HitStun(float StunDuration)
{
	DisableInputControl();
	GetWorldTimerManager().SetTimer(StunTimeHandle, this, &ACharacterBase::EnableInputControl, StunDuration, false);
}

void ACharacterBase::ApplyGESpecHandleToTargetDataSpecHandle(const FGameplayEffectSpecHandle& GESpecHandle, const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	for (TSharedPtr<FGameplayAbilityTargetData>Data: TargetDataHandle.Data)
	{
		Data->ApplyGameplayEffectSpec(*GESpecHandle.Data.Get());
	}
}