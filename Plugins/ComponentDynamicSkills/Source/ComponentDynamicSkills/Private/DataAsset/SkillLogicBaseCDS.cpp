// Pavel Gornostaev <https://github.com/Pavreally>

#include "DataAsset/SkillLogicBaseCDS.h"
#include "Components/ActorCDS.h"

void USkillLogicBaseCDS::OnActivate_Implementation(const FSkillExecutionContext& Context)
{
	(void)Context;
}

void USkillLogicBaseCDS::OnCommit_Implementation(const FSkillExecutionContext& Context)
{
	(void)Context;
}

void USkillLogicBaseCDS::OnExecute_Implementation(const FSkillExecutionContext& Context)
{
	(void)Context;
}

void USkillLogicBaseCDS::OnFinish_Implementation(const FSkillExecutionContext& Context)
{
	(void)Context;
}

void USkillLogicBaseCDS::OnInterrupt_Implementation(const FSkillExecutionContext& Context)
{
	(void)Context;
}

AActor* USkillLogicBaseCDS::GetOwnerActor() const
{
	if (OwningComponent)
	{
		return OwningComponent->GetOwner();
	}
	return nullptr;
}

UActorCDS* USkillLogicBaseCDS::GetCDSComponent() const
{
	return OwningComponent;
}
