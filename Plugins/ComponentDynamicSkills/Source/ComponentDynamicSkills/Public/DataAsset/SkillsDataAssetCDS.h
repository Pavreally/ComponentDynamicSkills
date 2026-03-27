// Pavel Gornostaev <https://github.com/Pavreally>

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SkillEnumsCDS.h"
#include "SkillExecutionContextCDS.h"
#include "DataAsset/EffectDataCDS.h"

class USkillLogicBaseCDS;
#include "Modifiers/SkillModifierBase.h"
#include "SkillsDataAssetCDS.generated.h"

class AActor;
class UAnimMontage;
class UCurveFloat;

UCLASS(BlueprintType)
class COMPONENTDYNAMICSKILLS_API USkillsDataAssetCDS : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Core", meta = (ToolTip = "Gameplay tag that uniquely identifies this skill"))
	FGameplayTag SkillTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Core", meta = (ToolTip = "Execution classification used to resolve executors (Melee/Projectile/AOE)"))
	ESkillExecutionType ExecutionType = ESkillExecutionType::Melee;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Core", meta = (ClampMin = "0.0", ToolTip = "Resource cost to use the skill (game-specific semantics)"))
	float Cost = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Core", meta = (ClampMin = "0.0", ToolTip = "Cooldown duration in seconds after the skill finishes"))
	float Cooldown = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Core", meta = (ClampMin = "1", ToolTip = "Maximum number of charges available for this skill"))
	int32 MaxCharges = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Core", meta = (ClampMin = "0.0", ToolTip = "Time in seconds to recharge a single charge"))
	float RechargeTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Core", meta = (ToolTip = "If true, charges will automatically start recharging when below max"))
	bool bAutoRecharge = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Charge", meta = (ToolTip = "If true, skill supports charging before commit/execution"))
	bool bIsChargeable = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Charge", meta = (ClampMin = "0.0", EditCondition = "bIsChargeable", EditConditionHides, ToolTip = "Minimum charge time in seconds (when chargeable)"))
	float MinChargeTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Charge", meta = (ClampMin = "0.0", EditCondition = "bIsChargeable", EditConditionHides, ToolTip = "Maximum charge time in seconds (when chargeable)"))
	float MaxChargeTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Charge", meta = (EditCondition = "bIsChargeable", EditConditionHides, ToolTip = "Optional curve mapping charge alpha over time"))
	TObjectPtr<UCurveFloat> ChargeCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Behavior", meta = (ToolTip = "Configured interrupt policy for this skill (None, OnDamage, OnMove, Always)"))
	ESkillInterruptPolicy InterruptPolicy = ESkillInterruptPolicy::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Behavior", meta = (ToolTip = "Additional gameplay tags associated with this skill (for filtering/queries)"))
	FGameplayTagContainer SkillTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|References", meta = (ToolTip = "Optional effect data applied when the skill executes"))
	TObjectPtr<UEffectDataCDS> Effect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|References", meta = (ToolTip = "Projectile actor class used by projectile-type skills"))
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Animation", meta = (ToolTip = "Optional mapping of animation montages keyed by tags"))
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> Animations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Parameters", meta = (ToolTip = "Minimum damage applied by the skill"))
	float MinDamage = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Parameters", meta = (ToolTip = "Maximum damage applied by the skill"))
	float MaxDamage = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Parameters", meta = (ToolTip = "Minimum radius for AOE effects"))
	float MinRadius = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Parameters", meta = (ToolTip = "Maximum radius for AOE effects"))
	float MaxRadius = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Modifier", meta = (ToolTip = "Optional modifier object applied to this skill's calculations"))
	TObjectPtr<USkillModifierBase> Modifier = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Behavior", meta = (ToolTip = "Duration of the active phase in seconds (0 = instant finish)"))
	float ActiveDuration = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Logic", meta = (ToolTip = "Blueprint class for skill logic implementation"))
	TSubclassOf<USkillLogicBaseCDS> SkillLogicClass;
};
