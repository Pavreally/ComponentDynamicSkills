// Pavel Gornostaev <https://github.com/Pavreally>

#include "Components/ActorCDS.h"
#include "DataAsset/SkillLogicBaseCDS.h"
#include "DataAsset/SkillsDataAssetCDS.h"
#include "Executors/SkillExecutor.h"

#include "Engine/World.h"

void UActorCDS::CommitSkill(FSkillRuntimeData &Skill)
{
	// Verify skill is in a valid state for commitment
	if (!Skill.Asset)
		return;
	if (Skill.State != ESkillState::PreCast && Skill.State != ESkillState::Charging)
		return;

	// Validate minimum charge time for chargeable skills
	if (UWorld *World = GetWorld())
	{
		if (Skill.Asset && Skill.Asset->bIsChargeable)
		{
			float Elapsed = World->GetTimeSeconds() - Skill.ChargeStartTime;
			if (Elapsed < Skill.Asset->MinChargeTime)
			{
				return;
			}
		}
		World->GetTimerManager().ClearTimer(Skill.ActiveTimer);
	}

	// Notify skill logic of commitment
	if (Skill.LogicInstance)
	{
		Skill.LogicInstance->OnCommit(BuildContext(Skill));
	}

	OnSkillCommitted.Broadcast(Skill.Asset->SkillTag);

	// Consume a charge and proceed to execution
	Skill.CurrentCharges = FMath::Max(0, Skill.CurrentCharges - 1);
	OnChargesChanged.Broadcast(Skill.Asset->SkillTag, Skill.CurrentCharges);

	ExecuteSkill(Skill);
}

void UActorCDS::ExecuteSkill(FSkillRuntimeData &Skill)
{
	if (!Skill.Asset)
	{
		return;
	}

	Skill.State = ESkillState::Casting;

	FSkillExecutionContext Context = BuildContext(Skill);

	// Apply modifiers to execution context before execution
	if (Skill.Asset->Modifier)
	{
		Skill.Asset->Modifier->ModifyExecution(Context);
	}

	// Notify skill logic of execution
	if (Skill.LogicInstance)
	{
		Skill.LogicInstance->OnExecute(Context);
	}

	OnSkillExecuted.Broadcast(Skill.Asset->SkillTag);

	// Execute the skill using the appropriate executor (Melee, Projectile, AOE)
	if (USkillExecutor *Executor = ResolveExecutor(Skill.Asset->ExecutionType))
	{
		Executor->Execute(Context);
	}

	// Apply skill effects independently from CDE
	ApplyEffectInternal(Context);

	Skill.State = ESkillState::Active;
	Skill.ChargeStartTime = 0.0f;

	// Set up active duration timer or finish immediately
	if (UWorld *World = GetWorld())
	{
		const float ActiveDuration = Skill.Asset->ActiveDuration;
		if (ActiveDuration > 0.0f)
		{
			FTimerDelegate Delegate;
			Delegate.BindUFunction(this, FName("HandleActiveFinished"), Skill.Asset->SkillTag);

			World->GetTimerManager().SetTimer(Skill.ActiveTimer, Delegate, ActiveDuration, false);
		}
		else
		{
			FinishSkill(Skill);
		}
	}
	else
	{
		FinishSkill(Skill);
	}
}

void UActorCDS::FinishSkill(FSkillRuntimeData &Skill)
{
	if (Skill.Asset == nullptr)
	{
		return;
	}

	// Notify skill logic of finish
	if (Skill.LogicInstance)
	{
		FSkillExecutionContext Context = BuildContext(Skill);
		Skill.LogicInstance->OnFinish(Context);
	}

	// Broadcast skill finished event
	OnSkillFinished.Broadcast(Skill.Asset->SkillTag);

	// Clean up active timer
	UWorld *World = GetWorld();
	FTimerManager *TimerManager = World != nullptr ? &World->GetTimerManager() : nullptr;

	if (TimerManager != nullptr)
	{
		TimerManager->ClearTimer(Skill.ActiveTimer);
	}

	Skill.ChargeStartTime = 0.0f;

	// Check if skill needs cooldown before recharge
	const int32 MaxCharges = GetMaxCharges(Skill.Asset);
	if (Skill.Asset->Cooldown > 0.0f)
	{
		Skill.State = ESkillState::Cooldown;
		Skill.CurrentCooldown = Skill.Asset->Cooldown;
		OnCooldownChanged.Broadcast(Skill.Asset->SkillTag, Skill.CurrentCooldown);

		if (TimerManager != nullptr)
		{
			TimerManager->ClearTimer(Skill.CooldownTimer);

			FTimerDelegate Delegate;
			Delegate.BindUFunction(this, FName("HandleCooldownFinished"), Skill.Asset->SkillTag);

			TimerManager->SetTimer(Skill.CooldownTimer, Delegate, Skill.Asset->Cooldown, false);
		}
		else
		{
			HandleCooldownFinished(Skill.Asset->SkillTag);
		}

		return;
	}

	// If skill has limited charges and auto-recharge is disabled, go to idle
	if (MaxCharges <= 1 && Skill.CurrentCharges <= 0)
	{
		Skill.CurrentCharges = MaxCharges;
		OnChargesChanged.Broadcast(Skill.Asset->SkillTag, Skill.CurrentCharges);
		Skill.CurrentCooldown = 0.0f;
		OnCooldownChanged.Broadcast(Skill.Asset->SkillTag, Skill.CurrentCooldown);
		Skill.State = ESkillState::Idle;
		return;
	}

	// Start recharge cycle if enabled and charges are depleted
	if (Skill.Asset->bAutoRecharge && Skill.CurrentCharges < MaxCharges)
	{
		StartRecharge(Skill);
		return;
	}

	// Otherwise go straight to idle state
	Skill.CurrentCooldown = 0.0f;
	OnCooldownChanged.Broadcast(Skill.Asset->SkillTag, Skill.CurrentCooldown);
	Skill.State = ESkillState::Idle;
}
