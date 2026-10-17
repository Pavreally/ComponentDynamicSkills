// Pavel Gornostaev <https://github.com/Pavreally>

#include "Components/ActorCDS.h"

#include "DataAsset/SkillParameterModifierCollectionAssetCDS.h"
#include "DataAsset/SkillParameterModifierDataAssetCDS.h"
#include "DataAsset/SkillsDataAssetCDS.h"
#include "Engine/World.h"

namespace
{
	bool IsValidParameterModifierSpec(
		const FSkillParameterModifierSpecCDS &ModifierSpec,
		bool bRequireModifierTag)
	{
		if ((bRequireModifierTag && !ModifierSpec.ModifierTag.IsValid()) ||
			!FMath::IsFinite(ModifierSpec.Duration) ||
			ModifierSpec.Duration < 0.0f ||
			ModifierSpec.Modifications.IsEmpty())
		{
			return false;
		}

		for (const FSkillParameterModificationCDS &Modification : ModifierSpec.Modifications)
		{
			if (!Modification.ParameterTag.IsValid() || !FMath::IsFinite(Modification.Value))
			{
				return false;
			}

			switch (Modification.Operation)
			{
			case ESkillParameterOperationCDS::Add:
			case ESkillParameterOperationCDS::Subtract:
			case ESkillParameterOperationCDS::Multiply:
				break;

			case ESkillParameterOperationCDS::Divide:
				if (Modification.Value == 0.0f)
				{
					return false;
				}
				break;

			default:
				return false;
			}
		}

		return true;
	}
}

bool UActorCDS::RegisterParameterModifier(USkillParameterModifierDataAssetCDS *Asset)
{
	if (Asset == nullptr)
	{
		return false;
	}

	FSkillParameterModifierSpecCDS ModifierSpec;
	ModifierSpec.ModifierTag = Asset->ModifierTag;
	ModifierSpec.Duration = Asset->Duration;
	ModifierSpec.Modifications = Asset->Modifications;

	if (!IsValidParameterModifierSpec(ModifierSpec, true))
	{
		return false;
	}

	ParameterModifierLookup.Add(Asset->ModifierTag, Asset);
	return true;
}

void UActorCDS::RegisterParameterModifierCollection(USkillParameterModifierCollectionAssetCDS *Collection)
{
	if (Collection == nullptr)
	{
		return;
	}

	for (USkillParameterModifierDataAssetCDS *ModifierAsset : Collection->Modifiers)
	{
		if (ModifierAsset != nullptr)
		{
			RegisterParameterModifier(ModifierAsset);
		}
	}
}

bool UActorCDS::ApplyModParameters(FGameplayTag SkillTag, FGameplayTag ModifierTag)
{
	const TObjectPtr<USkillParameterModifierDataAssetCDS> *AssetPtr = ParameterModifierLookup.Find(ModifierTag);
	if (AssetPtr == nullptr || AssetPtr->Get() == nullptr)
	{
		return false;
	}

	const USkillParameterModifierDataAssetCDS *Asset = AssetPtr->Get();
	FSkillParameterModifierSpecCDS ModifierSpec;
	ModifierSpec.ModifierTag = Asset->ModifierTag;
	ModifierSpec.Duration = Asset->Duration;
	ModifierSpec.Modifications = Asset->Modifications;
	return ApplyParameterModifierSpec(SkillTag, ModifierSpec, true, nullptr);
}

bool UActorCDS::ApplyRuntimeModParameters(
	FGameplayTag SkillTag,
	const FSkillParameterModifierSpecCDS &ModifierSpec)
{
	FGuid IgnoredModifierHandle;
	return ApplyParameterModifierSpec(SkillTag, ModifierSpec, false, &IgnoredModifierHandle);
}

bool UActorCDS::ApplyRuntimeModParametersWithHandle(
	FGameplayTag SkillTag,
	const FSkillParameterModifierSpecCDS &ModifierSpec,
	FGuid &OutModifierHandle)
{
	OutModifierHandle.Invalidate();
	return ApplyParameterModifierSpec(SkillTag, ModifierSpec, false, &OutModifierHandle);
}

bool UActorCDS::ApplyParameterModifierSpec(
	FGameplayTag SkillTag,
	const FSkillParameterModifierSpecCDS &ModifierSpec,
	bool bRequireModifierTag,
	FGuid *OutModifierHandle)
{
	if (OutModifierHandle != nullptr)
	{
		OutModifierHandle->Invalidate();
	}

	if (!IsValidParameterModifierSpec(ModifierSpec, bRequireModifierTag))
	{
		return false;
	}

	FSkillRuntimeData *Skill = FindSkill(SkillTag);
	if (Skill == nullptr || Skill->Asset == nullptr)
	{
		return false;
	}

	UWorld *World = GetWorld();
	if (ModifierSpec.Duration > 0.0f && World == nullptr)
	{
		return false;
	}

	TArray<FSkillParameterModificationCDS> MatchingModifications;
	for (const FSkillParameterModificationCDS &Modification : ModifierSpec.Modifications)
	{
		if (Skill->Asset->Parameters.Contains(Modification.ParameterTag))
		{
			MatchingModifications.Add(Modification);
		}
	}

	if (MatchingModifications.IsEmpty())
	{
		return false;
	}

	FActiveSkillParameterModifierCDS *ActiveModifier = nullptr;
	for (FActiveSkillParameterModifierCDS &ExistingModifier : Skill->ActiveParameterModifiers)
	{
		if (ModifierSpec.ModifierTag.IsValid() && ExistingModifier.ModifierTag == ModifierSpec.ModifierTag)
		{
			ClearParameterModifierTimer(ExistingModifier);
			ActiveModifier = &ExistingModifier;
			break;
		}
	}

	if (ActiveModifier == nullptr)
	{
		ActiveModifier = &Skill->ActiveParameterModifiers.AddDefaulted_GetRef();
		ActiveModifier->ApplicationId = FGuid::NewGuid();
	}

	if (!ActiveModifier->ApplicationId.IsValid())
	{
		ActiveModifier->ApplicationId = FGuid::NewGuid();
	}
	ActiveModifier->ModifierTag = ModifierSpec.ModifierTag;
	ActiveModifier->Duration = ModifierSpec.Duration;
	ActiveModifier->Modifications = MoveTemp(MatchingModifications);

	if (ActiveModifier->Duration > 0.0f)
	{
		FTimerDelegate ExpirationDelegate;
		ExpirationDelegate.BindUFunction(
			this,
			GET_FUNCTION_NAME_CHECKED(UActorCDS, HandleParameterModifierExpired),
			SkillTag,
			ActiveModifier->ApplicationId);
		World->GetTimerManager().SetTimer(
			ActiveModifier->ExpirationTimer,
			ExpirationDelegate,
			ActiveModifier->Duration,
			false);
	}

	if (OutModifierHandle != nullptr)
	{
		*OutModifierHandle = ActiveModifier->ApplicationId;
	}

	return true;
}

bool UActorCDS::RemoveModParameter(FGameplayTag SkillTag, FGameplayTag ModifierTag)
{
	if (!ModifierTag.IsValid())
	{
		return false;
	}

	FSkillRuntimeData *Skill = FindSkill(SkillTag);
	if (Skill == nullptr)
	{
		return false;
	}

	for (int32 Index = 0; Index < Skill->ActiveParameterModifiers.Num(); ++Index)
	{
		FActiveSkillParameterModifierCDS &Modifier = Skill->ActiveParameterModifiers[Index];
		if (Modifier.ModifierTag == ModifierTag)
		{
			ClearParameterModifierTimer(Modifier);
			Skill->ActiveParameterModifiers.RemoveAtSwap(Index);
			return true;
		}
	}

	return false;
}

bool UActorCDS::RemoveRuntimeModParameter(FGameplayTag SkillTag, FGuid ModifierHandle)
{
	if (!ModifierHandle.IsValid())
	{
		return false;
	}

	FSkillRuntimeData *Skill = FindSkill(SkillTag);
	if (Skill == nullptr)
	{
		return false;
	}

	for (int32 Index = 0; Index < Skill->ActiveParameterModifiers.Num(); ++Index)
	{
		FActiveSkillParameterModifierCDS &Modifier = Skill->ActiveParameterModifiers[Index];
		if (Modifier.ApplicationId == ModifierHandle)
		{
			ClearParameterModifierTimer(Modifier);
			Skill->ActiveParameterModifiers.RemoveAtSwap(Index);
			return true;
		}
	}

	return false;
}

int32 UActorCDS::RemoveModParameters(const FGameplayTagContainer &SkillTags)
{
	int32 RemovedCount = 0;
	for (const FGameplayTag &SkillTag : SkillTags)
	{
		FSkillRuntimeData *Skill = FindSkill(SkillTag);
		if (Skill == nullptr)
		{
			continue;
		}

		RemovedCount += Skill->ActiveParameterModifiers.Num();
		ClearSkillParameterModifiers(*Skill);
	}

	return RemovedCount;
}

bool UActorCDS::ResolveSkillParameterValue(
	FGameplayTag SkillTag,
	FGameplayTag ParameterTag,
	ESkillParameterOperationCDS BaseOperation,
	float Operand,
	float &OutValue) const
{
	OutValue = 0.0f;

	const FSkillRuntimeData *Skill = FindSkill(SkillTag);
	if (Skill == nullptr || Skill->Asset == nullptr)
	{
		return false;
	}

	const float *BaseValue = Skill->Asset->Parameters.Find(ParameterTag);
	if (BaseValue == nullptr)
	{
		return false;
	}

	float ModifiedBaseValue = 0.0f;
	if (!ApplyParameterOperation(*BaseValue, BaseOperation, Operand, ModifiedBaseValue))
	{
		return false;
	}

	return ResolveParameterValue(*Skill, ParameterTag, ModifiedBaseValue, OutValue);
}

bool UActorCDS::GetResolvedSkillParameterValue(
	FGameplayTag SkillTag,
	FGameplayTag ParameterTag,
	float &OutValue) const
{
	OutValue = 0.0f;
	const FSkillRuntimeData *Skill = FindSkill(SkillTag);
	if (Skill == nullptr || Skill->Asset == nullptr)
	{
		return false;
	}

	const float *BaseValue = Skill->Asset->Parameters.Find(ParameterTag);
	if (BaseValue == nullptr)
	{
		return false;
	}

	return ResolveParameterValue(*Skill, ParameterTag, *BaseValue, OutValue);
}

void UActorCDS::BuildResolvedParameters(const FSkillRuntimeData &Skill, TMap<FGameplayTag, float> &OutParameters) const
{
	OutParameters.Reset();
	if (Skill.Asset == nullptr)
	{
		return;
	}

	for (const TPair<FGameplayTag, float> &Parameter : Skill.Asset->Parameters)
	{
		float ResolvedValue = Parameter.Value;
		if (ResolveParameterValue(Skill, Parameter.Key, Parameter.Value, ResolvedValue))
		{
			OutParameters.Add(Parameter.Key, ResolvedValue);
		}
		else
		{
			OutParameters.Add(Parameter.Key, Parameter.Value);
		}
	}
}

bool UActorCDS::ResolveParameterValue(
	const FSkillRuntimeData &Skill,
	FGameplayTag ParameterTag,
	float BaseValue,
	float &OutValue) const
{
	OutValue = BaseValue;
	if (Skill.Asset == nullptr ||
		!ParameterTag.IsValid() ||
		!Skill.Asset->Parameters.Contains(ParameterTag) ||
		!FMath::IsFinite(BaseValue))
	{
		return false;
	}

	double AddedValue = 0.0;
	double SubtractedValue = 0.0;
	double Multiplier = 1.0;
	double Divisor = 1.0;

	for (const FActiveSkillParameterModifierCDS &ActiveModifier : Skill.ActiveParameterModifiers)
	{
		for (const FSkillParameterModificationCDS &Modification : ActiveModifier.Modifications)
		{
			if (Modification.ParameterTag != ParameterTag)
			{
				continue;
			}

			switch (Modification.Operation)
			{
			case ESkillParameterOperationCDS::Add:
				AddedValue += Modification.Value;
				break;

			case ESkillParameterOperationCDS::Subtract:
				SubtractedValue += Modification.Value;
				break;

			case ESkillParameterOperationCDS::Multiply:
				Multiplier *= Modification.Value;
				break;

			case ESkillParameterOperationCDS::Divide:
				if (Modification.Value == 0.0f)
				{
					return false;
				}
				Divisor *= Modification.Value;
				break;
			}
		}
	}

	// Aggregate by operation group so the result does not depend on modifier registration order.
	const double Result = ((static_cast<double>(BaseValue) + AddedValue - SubtractedValue) * Multiplier) / Divisor;
	const float FloatResult = static_cast<float>(Result);
	if (!FMath::IsFinite(FloatResult))
	{
		return false;
	}

	OutValue = FloatResult;
	return true;
}

bool UActorCDS::ApplyParameterOperation(
	float BaseValue,
	ESkillParameterOperationCDS Operation,
	float Operand,
	float &OutValue) const
{
	OutValue = 0.0f;
	if (!FMath::IsFinite(BaseValue) || !FMath::IsFinite(Operand))
	{
		return false;
	}

	switch (Operation)
	{
	case ESkillParameterOperationCDS::Add:
		OutValue = BaseValue + Operand;
		break;

	case ESkillParameterOperationCDS::Subtract:
		OutValue = BaseValue - Operand;
		break;

	case ESkillParameterOperationCDS::Multiply:
		OutValue = BaseValue * Operand;
		break;

	case ESkillParameterOperationCDS::Divide:
		if (Operand == 0.0f)
		{
			return false;
		}
		OutValue = BaseValue / Operand;
		break;

	default:
		return false;
	}

	return FMath::IsFinite(OutValue);
}

void UActorCDS::ClearSkillParameterModifiers(FSkillRuntimeData &Skill) const
{
	for (FActiveSkillParameterModifierCDS &Modifier : Skill.ActiveParameterModifiers)
	{
		ClearParameterModifierTimer(Modifier);
	}

	Skill.ActiveParameterModifiers.Reset();
}

void UActorCDS::ClearParameterModifierTimer(FActiveSkillParameterModifierCDS &Modifier) const
{
	if (UWorld *World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Modifier.ExpirationTimer);
	}
}

void UActorCDS::HandleParameterModifierExpired(FGameplayTag SkillTag, FGuid ApplicationId)
{
	FSkillRuntimeData *Skill = FindSkill(SkillTag);
	if (Skill == nullptr)
	{
		return;
	}

	for (int32 Index = 0; Index < Skill->ActiveParameterModifiers.Num(); ++Index)
	{
		if (Skill->ActiveParameterModifiers[Index].ApplicationId == ApplicationId)
		{
			Skill->ActiveParameterModifiers.RemoveAtSwap(Index);
			return;
		}
	}
}

void UActorCDS::RegisterParameterModifiersFromCollections()
{
	for (USkillParameterModifierCollectionAssetCDS *Collection : RegisteredParameterModifierCollections)
	{
		RegisterParameterModifierCollection(Collection);
	}

	for (USkillParameterModifierDataAssetCDS *ModifierAsset : RegisteredParameterModifierAssets)
	{
		RegisterParameterModifier(ModifierAsset);
	}
}
