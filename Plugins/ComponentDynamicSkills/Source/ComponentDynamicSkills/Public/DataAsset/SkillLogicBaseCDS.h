// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "SkillExecutionContextCDS.h"
#include "SkillLogicBaseCDS.generated.h"

class UActorCDS;

/**
 * Base class for skill logic implementations.
 * Provides Blueprintable hooks for custom skill behavior while maintaining separation from core CDS architecture.
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class COMPONENTDYNAMICSKILLS_API USkillLogicBaseCDS : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Called when a skill is activated.
	 * Executed after validation, with the skill already in the PreCast state.
	 *
	 * @param Context Execution context with source, target, and skill data
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Skill Logic CDS")
	void OnActivate(const FSkillExecutionContext &Context);

	/**
	 * Called when a skill is committed.
	 * Executed before consuming charges.
	 *
	 * @param Context Execution context with source, target, and skill data
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Skill Logic CDS")
	void OnCommit(const FSkillExecutionContext &Context);

	/**
	 * Called when a skill is executed.
	 * Executed BEFORE calling the executor system.
	 *
	 * @param Context Execution context with source, target, and skill data
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Skill Logic CDS")
	void OnExecute(const FSkillExecutionContext &Context);

	/**
	 * Called when a skill finishes.
	 * Executed during the finish phase.
	 *
	 * @param Context Execution context with source, target, and skill data
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Skill Logic CDS")
	void OnFinish(const FSkillExecutionContext &Context);

	/**
	 * Called when a skill is interrupted.
	 * Executed when the skill is interrupted.
	 *
	 * @param Context Execution context with source, target, and skill data
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Skill Logic CDS")
	void OnInterrupt(const FSkillExecutionContext &Context);

	/**
	 * Interrupt this skill, bypassing its configured interrupt policy.
	 * On success, calls OnFinish immediately and stops later CDS lifecycle stages when the current callback returns.
	 * The current Blueprint event graph continues after this node; the return value reports whether interruption succeeded.
	 *
	 * @return True if the skill was interrupted
	 */
	UFUNCTION(BlueprintCallable, Category = "Skill Logic CDS")
	bool InterruptSkill();

	/**
	 * Get the owner actor of this skill logic.
	 *
	 * @return Owner actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Skill Logic CDS")
	AActor *GetOwnerActor() const;

	/**
	 * Get the CDS component that owns this skill logic.
	 *
	 * @return CDS component
	 */
	UFUNCTION(BlueprintCallable, Category = "Skill Logic CDS")
	UActorCDS *GetCDSComponent() const;

protected:
	void Initialize(UActorCDS* InOwner, FGameplayTag InSkillTag);
	
private:
	// Cached reference to the owning component
	UPROPERTY(Transient)
	TObjectPtr<UActorCDS> OwningComponent = nullptr;

	UPROPERTY(Transient)
	FGameplayTag OwningSkillTag;

	// Allow ActorCDS to set the owning component
	friend class UActorCDS;
};
