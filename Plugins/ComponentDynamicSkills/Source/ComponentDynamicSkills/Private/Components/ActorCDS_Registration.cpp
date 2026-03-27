// Pavel Gornostaev <https://github.com/Pavreally>

#include "Components/ActorCDS.h"
#include "DataAsset/SkillLogicBaseCDS.h"
#include "DataAsset/SkillsDataAssetCDS.h"

void UActorCDS::RegisterSkill(USkillsDataAssetCDS *Asset)
{
	// Validate asset
	if (!Asset || !Asset->SkillTag.IsValid())
		return;

	// If skill already registered, replace its data
	if (const int32 *OldIndex = SkillLookup.Find(Asset->SkillTag))
	{
		FSkillRuntimeData &Skill = Skills[*OldIndex];
		ClearSkillTimers(Skill);
		Skill.Asset = Asset;
		Skill.CurrentCharges = GetMaxCharges(Asset);
		Skill.State = ESkillState::Idle;
		return;
	}

	// Create new skill runtime data
	FSkillRuntimeData Runtime;
	Runtime.Asset = Asset;
	Runtime.CurrentCharges = GetMaxCharges(Asset);
	Runtime.State = ESkillState::Idle;

	// Instantiate skill logic if configured
	if (Asset->SkillLogicClass)
	{
		Runtime.LogicInstance = NewObject<USkillLogicBaseCDS>(this, Asset->SkillLogicClass);
		if (Runtime.LogicInstance)
		{
			Runtime.LogicInstance->Initialize(this);
		}
	}

	// Add skill to storage and create lookup entry
	int32 NewIndex = Skills.Add(Runtime);
	SkillLookup.Add(Asset->SkillTag, NewIndex);

	// Auto-activate passive skills on registration
	FGameplayTag PassiveTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Passive"), false);
	if (PassiveTag.IsValid() && Asset->SkillTags.HasTag(PassiveTag))
	{
		TryActivateSkill(Asset->SkillTag);
	}
}

FSkillRuntimeData *UActorCDS::FindSkill(FGameplayTag Tag)
{
	// Validate tag
	if (!Tag.IsValid())
	{
		return nullptr;
	}

	// Find skill index in lookup map
	const int32 *SkillIndex = SkillLookup.Find(Tag);
	if (SkillIndex == nullptr || !Skills.IsValidIndex(*SkillIndex))
	{
		return nullptr;
	}

	return &Skills[*SkillIndex];
}

const FSkillRuntimeData *UActorCDS::FindSkill(FGameplayTag Tag) const
{
	// Validate tag
	if (!Tag.IsValid())
	{
		return nullptr;
	}

	// Find skill index in lookup map
	const int32 *SkillIndex = SkillLookup.Find(Tag);
	if (SkillIndex == nullptr || !Skills.IsValidIndex(*SkillIndex))
	{
		return nullptr;
	}

	return &Skills[*SkillIndex];
}

bool UActorCDS::FindSkillData(FGameplayTag Tag, FSkillRuntimeData &OutSkill) const
{
	// Find skill and copy its data
	if (const FSkillRuntimeData *Skill = FindSkill(Tag))
	{
		OutSkill = *Skill;
		return true;
	}

	// Return empty struct on failure
	OutSkill = FSkillRuntimeData{};
	return false;
}

void USkillLogicBaseCDS::Initialize(UActorCDS *InOwner)
{
	OwningComponent = InOwner;
}
