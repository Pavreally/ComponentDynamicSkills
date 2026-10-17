// Pavel Gornostaev <https://github.com/Pavreally>

#include "Components/ActorCDS.h"
#include "DataAsset/SkillLogicBaseCDS.h"
#include "DataAsset/SkillsDataAssetCDS.h"

#include "Engine/World.h"

bool UActorCDS::TryActivateSkill(FGameplayTag Tag, AActor *TargetActor, bool bCanStart)
{
	if (!bCanStart)
	{
		return false;
	}

	// Find and validate skill
	FSkillRuntimeData *Skill = FindSkill(Tag);
	if (Skill == nullptr || !CanActivateSkill(*Skill))
	{
		return false;
	}

	// Store target actor for skill logic
	Skill->TargetActor = TargetActor;
	Skill->bInterruptedThisActivation = false;
	Skill->ChargeStartTime = 0.0f;
	Skill->State = ESkillState::PreCast;

	// Notify skill logic of activation
	if (Skill->LogicInstance)
	{
		FSkillExecutionContext Context = BuildContext(*Skill);
		Skill->LogicInstance->OnActivate(Context);
		if (Skill->bInterruptedThisActivation)
		{
			return false;
		}
	}

	// Broadcast activation event
	if (Skill->Asset)
	{
		OnSkillActivated.Broadcast(Skill->Asset->SkillTag);
	}
	if (Skill->bInterruptedThisActivation)
	{
		return false;
	}

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

		return !Skill->bInterruptedThisActivation;
	}

	// Non-chargeable skills proceed directly to commit
	CommitSkill(*Skill);
	return !Skill->bInterruptedThisActivation;
}

bool UActorCDS::CanActivateSkill(const FSkillRuntimeData &Skill) const
{
	// Validate asset reference
	if (Skill.Asset == nullptr)
		return false;
	if (!Skill.Asset->SkillTag.IsValid())
		return false;
	
	// Check skill is idle and not inside a lifecycle callback
	if (Skill.State != ESkillState::Idle || Skill.bIsInterrupting || Skill.bIsFinishing)
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
	InterruptSkillInternal(Tag, false);
}

bool UActorCDS::ForceInterruptSkill(FGameplayTag Tag)
{
	return InterruptSkillInternal(Tag, true);
}

bool UActorCDS::InterruptSkillInternal(FGameplayTag Tag, bool bForce)
{
	FSkillRuntimeData *Skill = FindSkill(Tag);
	if (Skill == nullptr || Skill->Asset == nullptr || Skill->bIsInterrupting || Skill->bIsFinishing)
		return false;

	// Check interrupt policy
	if (!bForce && !ShouldInterrupt(Skill->Asset->InterruptPolicy))
		return false;

	// Only an active lifecycle phase can be interrupted.
	if (Skill->State != ESkillState::PreCast &&
		Skill->State != ESkillState::Charging &&
		Skill->State != ESkillState::Casting &&
		Skill->State != ESkillState::Active)
	{
		return false;
	}

	Skill->bIsInterrupting = true;
	Skill->bInterruptedThisActivation = true;

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
		// Cancel the pending activation without consuming charges or applying cooldown.
		ClearSkillTimers(*Skill);
		if (!bForce)
		{
			Skill->State = ESkillState::Idle;
			Skill->CurrentCooldown = 0.0f;
			Skill->ChargeStartTime = 0.0f;
		}
		OnSkillInterrupted.Broadcast(Skill->Asset->SkillTag);
		OnSkillInterruptedReason.Broadcast(Skill->Asset->SkillTag, Skill->Asset->InterruptPolicy);
		if (bForce && Skill->LogicInstance)
		{
			Skill->LogicInstance->OnFinish(BuildContext(*Skill));
		}
		if (bForce)
		{
			Skill->State = ESkillState::Idle;
			Skill->CurrentCooldown = 0.0f;
			Skill->ChargeStartTime = 0.0f;
		}
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

	Skill->bIsInterrupting = false;
	return true;
}
