// Pavel Gornostaev <https://github.com/Pavreally>

#include "StateTree/StateTreeTaskExecuteSkillCDS.h"

#include "Components/ActorCDS.h"
#include "GameFramework/Actor.h"

EStateTreeRunStatus UStateTreeTaskExecuteSkillCDS::EnterState(
		FStateTreeExecutionContext &Context,
		const FStateTreeTransitionResult &Transition)
{
	// Initialize ComponentCDS
	if (!ComponentCDS && OwnerActor)
	{
		ComponentCDS = OwnerActor->FindComponentByClass<UActorCDS>();
	}

	Super::EnterState(Context, Transition);

	if (!SkillTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("StateTreeTaskExecuteSkillCDS: SkillTag is not valid!"));
		return EStateTreeRunStatus::Failed;
	}

	if (!ComponentCDS)
	{
		UE_LOG(LogTemp, Warning, TEXT("StateTreeTaskExecuteSkillCDS: Component CDS not found."));
		return EStateTreeRunStatus::Failed;
	}

	if (!ComponentCDS->TryActivateSkill(SkillTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("StateTreeTaskExecuteSkillCDS: Failed to activate skill with tag: %s"), *SkillTag.ToString());
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}
