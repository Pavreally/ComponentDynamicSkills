// Pavel Gornostaev <https://github.com/Pavreally>

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameplayTagContainer.h"
#include "SkillExecutionContextCDS.generated.h"

class AActor;
class USkillsDataAssetCDS;
class UActorCDS;
class USkillContextDataAssetCDS;

/**
 * Execution context passed through the entire skill pipeline.
 * Provides all necessary information to modifiers, executors, and skill logic.
 * 
 * This struct is designed to be garbage-collection-safe and does not own references
 * to gameplay logic. All pointers should be validated before use.
 * 
 * Used in:
 * - Modifier::ModifyExecution()
 * - SkillLogic callbacks (OnActivate, OnExecute, OnCommit, OnFinish, OnInterrupt)
 * - Executor::Execute()
 */
USTRUCT(BlueprintType)
struct FSkillExecutionContext
{
	GENERATED_BODY()

public:
	/**
	 * Source actor for skill execution.
	 * Typically the owner of the ActorCDS component (player, enemy, NPC).
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Skill Context")
	TObjectPtr<AActor> Source = nullptr;

	/**
	 * Target actor for skill execution.
	 * May be nullptr if the skill is self-cast or area-targeted.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Skill Context")
	TObjectPtr<AActor> Target = nullptr;

	/**
	 * Skeletal mesh used as the source for sockets, bones, traces, and VFX spawn points.
	 * Resolved once by UActorCDS and injected into the context for skill logic/executors.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Skill Context")
	TObjectPtr<USkeletalMeshComponent> SourceSkeletalMesh = nullptr;

	/**
	 * Normalized charge progress (0.0 to 1.0).
	 * Used by modifiers and executors to scale damage, effects, or visual feedback.
	 * 1.0 means fully charged.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Skill Context")
	float ChargeAlpha = 1.0f;

	/**
	 * Reference to the skill data asset.
	 * Contains skill configuration like ExecutionType, Effect, Modifier, etc.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Skill Context")
	TObjectPtr<const USkillsDataAssetCDS> Data = nullptr;

	/**
	 * Reference to the ActorCDS component that owns this skill.
	 * Can be used to access component methods or owner information.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Skill Context")
	TObjectPtr<UActorCDS> SourceComponent = nullptr;

	/**
	 * Selected skill context asset, resolved from ContextBindings by gameplay-tag matching and priority.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Skill Context")
	TObjectPtr<const USkillContextDataAssetCDS> ContextData = nullptr;
};
