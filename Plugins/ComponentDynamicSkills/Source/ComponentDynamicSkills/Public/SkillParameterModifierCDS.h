// Pavel Gornostaev <https://github.com/Pavreally>

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SkillParameterModifierCDS.generated.h"

/**
 * Operations are evaluated in fixed groups:
 * (Base + sum(Adds) - sum(Subtracts)) * product(Multipliers) / product(Divisors).
 */
UENUM(BlueprintType)
enum class ESkillParameterOperationCDS : uint8
{
	Add,
	Subtract,
	Multiply,
	Divide
};

/** One numeric operation applied to a skill parameter at runtime. */
USTRUCT(BlueprintType)
struct COMPONENTDYNAMICSKILLS_API FSkillParameterModificationCDS
{
	GENERATED_BODY()

	/** Exact gameplay tag of the skill parameter affected by this operation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameter Modifier")
	FGameplayTag ParameterTag;

	/** Operand used by the selected operation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameter Modifier")
	float Value = 0.0f;

	/** Arithmetic operation applied to the parameter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameter Modifier")
	ESkillParameterOperationCDS Operation = ESkillParameterOperationCDS::Add;
};

/**
 * Runtime instruction shared by asset-based and dynamically created modifiers.
 * ModifierTag may be omitted for runtime applications; those receive unique FGuid handles.
 */
USTRUCT(BlueprintType)
struct COMPONENTDYNAMICSKILLS_API FSkillParameterModifierSpecCDS
{
	GENERATED_BODY()

	/** Optional identity. If unset, each runtime application receives its own handle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameter Modifier")
	FGameplayTag ModifierTag;

	/** Duration in seconds; zero keeps the modifier active until explicitly removed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameter Modifier", meta = (ClampMin = "0.0"))
	float Duration = 0.0f;

	/** Operations copied into the skill runtime when this specification is applied. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameter Modifier")
	TArray<FSkillParameterModificationCDS> Modifications;
};
