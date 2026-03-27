#pragma once

#include "CoreMinimal.h"
#include "SkillEnumsCDS.h"
#include "SkillRuntimeData.generated.h"

class USkillsDataAssetCDS;
class USkillLogicBaseCDS;
class AActor;

/**
 * Runtime data for a skill instance.
 * Stores transient state that changes during gameplay.
 * All properties persist during skill execution but are reset on skill cleanup.
 */
USTRUCT(BlueprintType)
struct COMPONENTDYNAMICSKILLS_API FSkillRuntimeData
{
	GENERATED_BODY()

	/**
	 * Reference to the skill configuration asset.
	 * Defines skill properties like cooldown, charges, effects, etc.
	 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Skill")
	TObjectPtr<USkillsDataAssetCDS> Asset = nullptr;

	/**
	 * Current cooldown remaining in seconds.
	 * 0.0 means skill is off cooldown.
	 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Skill")
	float CurrentCooldown = 0.0f;

	/**
	 * Current charges available.
	 * Max charges is determined by the skill asset.
	 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Skill")
	int32 CurrentCharges = 0;

	/**
	 * Current skill lifecycle state.
	 * Determines what actions are valid (activation, commitment, etc).
	 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Skill")
	ESkillState State = ESkillState::Idle;

	/**
	 * Timestamp when skill charging started.
	 * Used to calculate charge alpha (0.0 to 1.0).
	 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Skill")
	float ChargeStartTime = 0.0f;

	/**
	 * Target actor for this skill instance.
	 * May be null if the skill doesn't target actors.
	 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Skill")
	TObjectPtr<AActor> TargetActor = nullptr;

	/**
	 * Timer handle for active duration.
	 * Cleared when skill finishes or is interrupted.
	 */
	UPROPERTY(Transient)
	FTimerHandle ActiveTimer;

	/**
	 * Timer handle for cooldown period.
	 * Cleared when cooldown completes or skill is interrupted.
	 */
	UPROPERTY(Transient)
	FTimerHandle CooldownTimer;

	/**
	 * Blueprintable skill logic instance.
	 * Created from the skill asset's SkillLogicClass.
	 * Provides hooks for custom skill behavior.
	 */
	UPROPERTY(Transient)
	TObjectPtr<USkillLogicBaseCDS> LogicInstance = nullptr;
};
