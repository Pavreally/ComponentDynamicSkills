// Pavel Gornostaev <https://github.com/Pavreally>

#include "Components/ActorCDS.h"
#include "DataAsset/SkillsDataAssetCDS.h"
#include "Delegates/Delegate.h"

#include "Engine/World.h"

void UActorCDS::StartRecharge(FSkillRuntimeData &Skill)
{
	// Validate skill and auto-recharge setting
	if (Skill.Asset == nullptr || !Skill.Asset->bAutoRecharge)
	{
		return;
	}

	const int32 MaxCharges = GetMaxCharges(Skill.Asset);
	
	// If already at max charges, skip recharge
	if (Skill.CurrentCharges >= MaxCharges)
	{
		Skill.CurrentCooldown = 0.0f;
		OnCooldownChanged.Broadcast(Skill.Asset->SkillTag, Skill.CurrentCooldown);
		Skill.State = ESkillState::Idle;
		return;
	}

	// If no recharge time configured, fully recharge immediately
	if (Skill.Asset->RechargeTime <= 0.0f)
	{
		Skill.CurrentCharges = MaxCharges;
		Skill.CurrentCooldown = 0.0f;
		Skill.State = ESkillState::Idle;
		OnChargesChanged.Broadcast(Skill.Asset->SkillTag, Skill.CurrentCharges);
		OnCooldownChanged.Broadcast(Skill.Asset->SkillTag, Skill.CurrentCooldown);
		return;
	}

	// Enter cooldown state and set recharge timer
	Skill.State = ESkillState::Cooldown;
	Skill.CurrentCooldown = Skill.Asset->RechargeTime;
	OnCooldownChanged.Broadcast(Skill.Asset->SkillTag, Skill.CurrentCooldown);

	if (UWorld *World = GetWorld())
	{
		FTimerManager &TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(Skill.CooldownTimer);

		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, FName("HandleRechargeFinished"), Skill.Asset->SkillTag);

		TimerManager.SetTimer(Skill.CooldownTimer, Delegate, Skill.Asset->RechargeTime, false);
	}
	else
	{
		// If world is unavailable, finish recharge immediately
		HandleRechargeFinished(Skill.Asset->SkillTag);
	}
}

void UActorCDS::HandleChargeTimeout(FGameplayTag Tag)
{
	// Auto-commit skill after maximum charge time
	if (FSkillRuntimeData *Skill = FindSkill(Tag))
	{
		if (Skill->State == ESkillState::Charging)
		{
			CommitSkill(*Skill);
		}
	}
}

void UActorCDS::HandleActiveFinished(FGameplayTag Tag)
{
	// Finish skill after active duration expires
	if (FSkillRuntimeData *Skill = FindSkill(Tag))
	{
		FinishSkill(*Skill);
	}
}

void UActorCDS::HandleCooldownFinished(FGameplayTag Tag)
{
	// Transition from cooldown to idle/recharge state
	FSkillRuntimeData *Skill = FindSkill(Tag);
	if (Skill == nullptr || Skill->Asset == nullptr)
	{
		return;
	}

	// Clear cooldown timer
	if (UWorld *World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Skill->CooldownTimer);
	}

	Skill->CurrentCooldown = 0.0f;
	OnCooldownChanged.Broadcast(Skill->Asset->SkillTag, Skill->CurrentCooldown);

	const int32 MaxCharges = GetMaxCharges(Skill->Asset);
	
	// If single charge skill, return to idle
	if (MaxCharges <= 1 && Skill->CurrentCharges <= 0)
	{
		Skill->CurrentCharges = MaxCharges;
		Skill->State = ESkillState::Idle;
		OnChargesChanged.Broadcast(Skill->Asset->SkillTag, Skill->CurrentCharges);
		return;
	}

	// Start recharge cycle for multi-charge skills
	if (Skill->Asset->bAutoRecharge && Skill->CurrentCharges < MaxCharges)
	{
		StartRecharge(*Skill);
		return;
	}

	// Go to idle if no auto-recharge
	Skill->State = ESkillState::Idle;
}

void UActorCDS::HandleRechargeFinished(FGameplayTag Tag)
{
	// Restore one charge after recharge duration
	FSkillRuntimeData *Skill = FindSkill(Tag);
	if (Skill == nullptr || Skill->Asset == nullptr)
	{
		return;
	}

	// Broadcast periodic tick event for passive systems
	OnSkillTick.Broadcast(Skill->Asset->SkillTag);

	// Clear recharge timer
	if (UWorld *World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Skill->CooldownTimer);
	}

	// Increment charges
	const int32 MaxCharges = GetMaxCharges(Skill->Asset);
	Skill->CurrentCharges = FMath::Min(Skill->CurrentCharges + 1, MaxCharges);
	OnChargesChanged.Broadcast(Skill->Asset->SkillTag, Skill->CurrentCharges);

	// If not at max charges, schedule next recharge
	if (Skill->CurrentCharges < MaxCharges)
	{
		StartRecharge(*Skill);
		return;
	}

	// All charges restored, go to idle
	Skill->CurrentCooldown = 0.0f;
	OnCooldownChanged.Broadcast(Skill->Asset->SkillTag, Skill->CurrentCooldown);
	Skill->State = ESkillState::Idle;
}
