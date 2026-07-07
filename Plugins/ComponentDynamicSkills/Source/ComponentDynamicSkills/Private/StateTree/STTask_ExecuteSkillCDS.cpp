// Pavel Gornostaev <https://github.com/Pavreally>

#include "StateTree/STTask_ExecuteSkillCDS.h"
#include "Components/ActorCDS.h"
#include "DataAsset/SkillsDataAssetCDS.h"
#include "DataAsset/SkillContextDataAssetCDS.h"

EStateTreeRunStatus USTTask_ExecuteSkillCDS::EnterState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition)
{
	bExecutionStarted = false;

	UActorCDS *Component = SkillComponent;
	if (Component == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	USkillsDataAssetCDS *Asset = SkillDataAsset;
	if (Asset == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!SkillTag.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	// Resolve context data if needed
	const USkillContextDataAssetCDS *ContextData = nullptr;
	if (ContextTag.IsValid())
	{
		if (const TObjectPtr<USkillContextDataAssetCDS> *FoundContext = Asset->ContextBindings.Find(ContextTag))
		{
			ContextData = FoundContext->Get();
		}
	}

	// Activate skill with resolved context
	if (Component->TryActivateSkill(SkillTag, nullptr))
	{
		bExecutionStarted = true;
		return EStateTreeRunStatus::Running;
	}

	return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus USTTask_ExecuteSkillCDS::Tick(FStateTreeExecutionContext &Context, const float DeltaSeconds)
{
	if (!bExecutionStarted)
	{
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Succeeded;
}

void USTTask_ExecuteSkillCDS::ExitState(FStateTreeExecutionContext &Context, const FStateTreeTransitionResult &Transition)
{
	bExecutionStarted = false;
}
