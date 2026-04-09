// Pavel Gornostaev <https://github.com/Pavreally>

#include "Executors/SkillExecutor.h"
#include "DataAsset/SkillsDataAssetCDS.h"

void USkillExecutor::Execute(const FSkillExecutionContext& Context)
{
	if (Context.Data == nullptr)
	{
		return;
	}

	// if (Context.Data->Effect)
	// {
	// 	UE_LOG(LogTemp, Log, TEXT("Skill [%s] executed with effect: %s"), 
	// 		*Context.Data->SkillTag.ToString(), *Context.Data->Effect->EffectTag.ToString());
	// }
}

void UMeleeSkillExecutor::Execute(const FSkillExecutionContext& Context)
{
	if (Context.Data == nullptr)
	{
		return;
	}

	// Base execution
	Super::Execute(Context);

	// Melee-specific logic
	// if (Context.Data->Effect)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Melee Skill [%s] applied to target: %s"), 
	// 		*Context.Data->SkillTag.ToString(), 
	// 		*GetNameSafe(Context.Target));
	// }
}

void UProjectileSkillExecutor::Execute(const FSkillExecutionContext& Context)
{
	if (Context.Data == nullptr)
	{
		return;
	}

	// Base execution
	Super::Execute(Context);

	// Projectile-specific logic
	// if (Context.Data->ProjectileClass)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Projectile Skill [%s] spawned: %s"), 
	// 		*Context.Data->SkillTag.ToString(), 
	// 		*Context.Data->ProjectileClass->GetName());
	// }
}

void UAOESkillExecutor::Execute(const FSkillExecutionContext& Context)
{
	if (Context.Data == nullptr)
	{
		return;
	}

	// Base execution
	Super::Execute(Context);

	// AOE-specific logic
	if (Context.Data->Effect)
	{
		UE_LOG(LogTemp, Warning, TEXT("AOE Skill [%s] radius: min=%.2f max=%.2f"), 
			*Context.Data->SkillTag.ToString(),
			Context.Data->MinRadius,
			Context.Data->MaxRadius);
	}
}
