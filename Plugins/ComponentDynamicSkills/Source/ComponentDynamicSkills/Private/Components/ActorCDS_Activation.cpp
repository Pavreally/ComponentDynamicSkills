// Pavel Gornostaev <https://github.com/Pavreally>

#include "Components/ActorCDS.h"
#include "DataAsset/SkillLogicBaseCDS.h"
#include "DataAsset/SkillsDataAssetCDS.h"

#include "Engine/World.h"

bool UActorCDS::TryActivateSkill(FGameplayTag Tag, AActor *TargetActor)
{
	// Find and validate skill
	FSkillRuntimeData *Skill = FindSkill(Tag);
	if (Skill == nullptr || !CanActivateSkill(*Skill))
	{
		return false;
	}

	// Store target actor for skill logic
	Skill->TargetActor = TargetActor;

	// Notify skill logic of activation
	if (Skill->LogicInstance)
	{
		FSkillExecutionContext Context = BuildContext(*Skill);
		Skill->LogicInstance->OnActivate(Context);
	}

	// Broadcast activation event
	if (Skill->Asset)
	{
		OnSkillActivated.Broadcast(Skill->Asset->SkillTag);
	}

	Skill->State = ESkillState::PreCast;
	Skill->ChargeStartTime = 0.0f;

	// If skill is chargeable, enter charging state and set up charge timeout timer
	if (Skill->Asset && Skill->Asset->bIsChargeable)
	{
		Skill->State = ESkillState::Charging;

		if (UWorld *World = GetWorld())
		{
			Skill->ChargeStartTime = World->GetTimeSeconds();

			// Use the maximum charge time as the timeout
			const float MaxChargeTime =
					FMath::Max(Skill->Asset->MinChargeTime, Skill->Asset->MaxChargeTime);

			if (MaxChargeTime > 0.0f)
			{
				FTimerDelegate Delegate;
				Delegate.BindUFunction(this, FName("HandleChargeTimeout"), Tag);

				World->GetTimerManager().SetTimer(
						Skill->ActiveTimer,
						Delegate,
						MaxChargeTime,
						false);
			}
		}

		return true;
	}

	// Non-chargeable skills proceed directly to commit
	CommitSkill(*Skill);
	return true;
}

bool UActorCDS::CanActivateSkill(const FSkillRuntimeData &Skill) const
{
	// Validate asset reference
	if (Skill.Asset == nullptr)
		return false;
	if (!Skill.Asset->SkillTag.IsValid())
		return false;
	
	// Check skill is in idle state
	if (Skill.State != ESkillState::Idle)
		return false;
	
	// Check available charges
	if (Skill.CurrentCharges <= 0)
		return false;

	// Call custom validation hook (used for cost checking, resource requirements, etc.)
	if (!CanActivateCustom(Skill))
		return false;

	return true;
}

bool UActorCDS::CanActivateSkillByTag(FGameplayTag Tag)
{
	if (FSkillRuntimeData *Skill = FindSkill(Tag))
	{
		return CanActivateSkill(*Skill);
	}

	return false;
}

void UActorCDS::InterruptSkill(FGameplayTag Tag)
{
	FSkillRuntimeData *Skill = FindSkill(Tag);
	if (Skill == nullptr || Skill->Asset == nullptr)
		return;

	// Check interrupt policy
	if (!ShouldInterrupt(Skill->Asset->InterruptPolicy))
		return;

	// Notify skill logic of interruption
	if (Skill->LogicInstance)
	{
		FSkillExecutionContext Context = BuildContext(*Skill);
		Skill->LogicInstance->OnInterrupt(Context);
	}

	// Handle interruption based on current state
	switch (Skill->State)
	{
	case ESkillState::PreCast:
	case ESkillState::Charging:
		// Reset skill to idle without consuming charges
		ClearSkillTimers(*Skill);
		Skill->State = ESkillState::Idle;
		Skill->CurrentCooldown = 0.0f;
		Skill->ChargeStartTime = 0.0f;
		OnSkillInterrupted.Broadcast(Skill->Asset->SkillTag);
		OnSkillInterruptedReason.Broadcast(Skill->Asset->SkillTag, Skill->Asset->InterruptPolicy);
		break;

	case ESkillState::Casting:
	case ESkillState::Active:
		// Interrupt active execution and finish skill normally (applies cooldown)
		ClearSkillTimers(*Skill);
		OnSkillInterrupted.Broadcast(Skill->Asset->SkillTag);
		OnSkillInterruptedReason.Broadcast(Skill->Asset->SkillTag, Skill->Asset->InterruptPolicy);
		FinishSkill(*Skill);
		break;

	default:
		break;
	}
}
