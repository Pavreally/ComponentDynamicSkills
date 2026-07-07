// Pavel Gornostaev <https://github.com/Pavreally>

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "GameplayTagContainer.h"
#include "STTask_ExecuteSkillCDS.generated.h"

class UActorCDS;
class USkillsDataAssetCDS;
class USkillContextDataAssetCDS;

UCLASS(BlueprintType, meta = (DisplayName = "Execute Skill CDS"))
class COMPONENTDYNAMICSKILLS_API USTTask_ExecuteSkillCDS : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CDS")
	FGameplayTag SkillTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CDS")
	FGameplayTag ContextTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CDS")
	FGameplayTagContainer ContextTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CDS")
	TObjectPtr<UActorCDS> SkillComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CDS")
	TObjectPtr<USkillsDataAssetCDS> SkillDataAsset = nullptr;

protected:
	virtual EStateTreeRunStatus EnterState(
			FStateTreeExecutionContext &Context,
			const FStateTreeTransitionResult &Transition) override;

	virtual EStateTreeRunStatus Tick(
			FStateTreeExecutionContext &Context,
			const float DeltaSeconds) override;

	virtual void ExitState(
			FStateTreeExecutionContext &Context,
			const FStateTreeTransitionResult &Transition) override;

private:
	bool bExecutionStarted = false;
};
