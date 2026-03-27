// Pavel Gornostaev <https://github.com/Pavreally>

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "EffectDataCDS.generated.h"

/**
 * Minimal data-only effect asset used by ComponentDynamicSkills.
 * Stores only a gameplay tag and contains no runtime logic or external integrations.
 */
UCLASS(BlueprintType)
class COMPONENTDYNAMICSKILLS_API UEffectDataCDS : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	FGameplayTag EffectTag;
};
