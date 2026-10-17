// Pavel Gornostaev <https://github.com/Pavreally>

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAsset/SkillParameterModifierDataAssetCDS.h"
#include "SkillParameterModifierCollectionAssetCDS.generated.h"

/** Collection of parameter modifier assets registered together by UActorCDS. */
UCLASS(BlueprintType)
class COMPONENTDYNAMICSKILLS_API USkillParameterModifierCollectionAssetCDS : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Modifier assets registered together by an ActorCDS component. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Parameter Modifier|Collection")
	TArray<TObjectPtr<USkillParameterModifierDataAssetCDS>> Modifiers;
};
