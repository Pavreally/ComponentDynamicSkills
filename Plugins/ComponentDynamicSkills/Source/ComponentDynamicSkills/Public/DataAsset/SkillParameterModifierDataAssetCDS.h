// Pavel Gornostaev <https://github.com/Pavreally>

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SkillParameterModifierCDS.h"
#include "SkillParameterModifierDataAssetCDS.generated.h"

/** Authored instruction for temporarily or permanently modifying skill parameters. */
UCLASS(BlueprintType)
class COMPONENTDYNAMICSKILLS_API USkillParameterModifierDataAssetCDS : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Unique gameplay tag used to register, apply, refresh, and remove this modifier definition. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Parameter Modifier")
	FGameplayTag ModifierTag;

	/** Active duration in seconds; zero means the applied modifier remains until removed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Parameter Modifier", meta = (ClampMin = "0.0", ToolTip = "Duration in seconds; 0 means permanent"))
	float Duration = 0.0f;

	/** Operations applied to matching numeric parameters on the target skill. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Parameter Modifier")
	TArray<FSkillParameterModificationCDS> Modifications;
};
