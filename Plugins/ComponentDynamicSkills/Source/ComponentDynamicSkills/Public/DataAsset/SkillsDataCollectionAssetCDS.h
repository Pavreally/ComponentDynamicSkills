// Pavel Gornostaev <https://github.com/Pavreally>

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SkillsDataAssetCDS.h"
#include "SkillsDataCollectionAssetCDS.generated.h"

UCLASS(BlueprintType)
class COMPONENTDYNAMICSKILLS_API USkillsDataCollectionAssetCDS : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Collection", meta = (ToolTip = "Skill data assets registered together as a single collection"))
	TArray<TObjectPtr<USkillsDataAssetCDS>> Skills;
};
