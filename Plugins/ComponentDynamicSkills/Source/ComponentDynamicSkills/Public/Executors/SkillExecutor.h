// Pavel Gornostaev <https://github.com/Pavreally>

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SkillExecutionContextCDS.h"
#include "SkillExecutor.generated.h"

/**
 * Base executor interface for skill execution.
 * Specialized executors can extend this to implement custom execution logic.
 */
UCLASS(Abstract, Blueprintable)
class COMPONENTDYNAMICSKILLS_API USkillExecutor : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Executes a resolved skill context.
	 *
	 * @param Context Runtime execution payload prepared by UActorCDS
	 */
	virtual void Execute(const FSkillExecutionContext& Context);
};

/**
 * Melee skill executor - handles close-range physical attacks.
 */
UCLASS()
class COMPONENTDYNAMICSKILLS_API UMeleeSkillExecutor : public USkillExecutor
{
	GENERATED_BODY()

public:
	virtual void Execute(const FSkillExecutionContext& Context) override;
};

/**
 * Projectile skill executor - handles ranged projectile attacks.
 */
UCLASS()
class COMPONENTDYNAMICSKILLS_API UProjectileSkillExecutor : public USkillExecutor
{
	GENERATED_BODY()

public:
	virtual void Execute(const FSkillExecutionContext& Context) override;
};

/**
 * Area of Effect (AOE) skill executor - handles area damage/effects.
 */
UCLASS()
class COMPONENTDYNAMICSKILLS_API UAOESkillExecutor : public USkillExecutor
{
	GENERATED_BODY()

public:
	virtual void Execute(const FSkillExecutionContext& Context) override;
};
