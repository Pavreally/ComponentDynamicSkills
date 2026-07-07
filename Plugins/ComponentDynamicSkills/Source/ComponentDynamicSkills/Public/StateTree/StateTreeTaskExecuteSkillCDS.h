// Pavel Gornostaev <https://github.com/Pavreally>

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "GameplayTagContainer.h"
#include "StateTreeTaskExecuteSkillCDS.generated.h"

class UActorCDS;

UCLASS(BlueprintType, meta = (DisplayName = "Execute Skill CDS"))
class COMPONENTDYNAMICSKILLS_API UStateTreeTaskExecuteSkillCDS : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	/** The owner actor of the skill to execute. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	TObjectPtr<AActor> OwnerActor = nullptr;

	/** Gameplay Tag of the skill to execute. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CDS")
	FGameplayTag SkillTag;

	/** Optional CDS component. If empty, the component will be searched on the owner actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CDS", meta = (DisplayName = "Component CDS"))
	TObjectPtr<UActorCDS> ComponentCDS = nullptr;

protected:
	virtual EStateTreeRunStatus EnterState(
			FStateTreeExecutionContext &Context,
			const FStateTreeTransitionResult &Transition) override;

};
