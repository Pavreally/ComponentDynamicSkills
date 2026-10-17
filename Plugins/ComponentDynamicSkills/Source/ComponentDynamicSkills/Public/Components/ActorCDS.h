#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "SkillEnumsCDS.h"
#include "SkillExecutionContextCDS.h"
#include "SkillRuntimeData.h"
#include "Executors/SkillExecutor.h"
#include "DataAsset/SkillsDataCollectionAssetCDS.h"
#include "DataAsset/SkillContextDataAssetCDS.h"
#include "ActorCDS.generated.h"

class USkillsDataAssetCDS;
class USkillLogicBaseCDS;
class USkillModifierBase;
class USkillParameterModifierDataAssetCDS;
class USkillParameterModifierCollectionAssetCDS;
class USkeletalMeshComponent;

// Event delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillActivatedCDS, FGameplayTag, SkillTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillCommittedCDS, FGameplayTag, SkillTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillExecutedCDS, FGameplayTag, SkillTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillFinishedCDS, FGameplayTag, SkillTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillInterruptedCDS, FGameplayTag, SkillTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCooldownChangedCDS, FGameplayTag, SkillTag, float, NewCooldown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChargesChangedCDS, FGameplayTag, SkillTag, int32, NewCharges);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillInterruptedReasonCDS, FGameplayTag, SkillTag, ESkillInterruptPolicy, InterruptPolicy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillTickCDS, FGameplayTag, SkillTag);

/**
 * Core component for managing skill execution in CDS (Component Dynamic Skills).
 *
 * ## Responsibilities:
 * - Register and manage skill assets.
 * - Handle skill lifecycle (activation, commitment, execution, finishing).
 * - Skills are authored as data assets and may be grouped into collections.
 * - Manage timers for charges, cooldowns, active durations, and parameter modifiers.
 * - Dispatch events for external systems.
 * - Context bindings resolve usage contexts by gameplay tags and priority.
 *
 * ## Skill execution flow:
 * 1. TryActivateSkill() - Check if skill can be activated.
 * 2. CommitSkill() - Charge validation and confirmation.
 * 3. ExecuteSkill() - Modifier + Logic + Executor + Effect.
 * 4. FinishSkill() - Cooldown/recharge and state reset.
 *
 * ## Recommended Naming Conventions:
 * - `Anim.*` — animation references such as `Anim.Cast`, `Anim.PreCast`, `Anim.Heavy`.
 * - `Effect.*` — effect references such as `Effect.Primary`, `Effect.Explosion`.
 * - `Sound.*` — sound references such as `Sound.Cast`, `Sound.Impact`.
 * - `Projectile.*` — projectile class references.
 * - `AI.*` — AI behavior parameters such as `AI.StopMovement`, `AI.RequiredDistance`.
 * - `Player.*` — player behavior parameters.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COMPONENTDYNAMICSKILLS_API UActorCDS : public UActorComponent
{
	GENERATED_BODY()

public:
	UActorCDS();

	/**
	 * Array of skill assets to register on BeginPlay.
	 * Skills are initialized automatically when the component is attached to an actor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Skills CDS|Registration")
	TArray<TObjectPtr<USkillsDataCollectionAssetCDS>> RegisteredSkillCollections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Skills CDS|Registration")
	TArray<TObjectPtr<USkillsDataAssetCDS>> RegisteredSkillAssets;

	/** Modifier collections registered on BeginPlay before skills are initialized. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Skills CDS|Parameter Modifiers|Registration")
	TArray<TObjectPtr<USkillParameterModifierCollectionAssetCDS>> RegisteredParameterModifierCollections;

	/** Individual modifier assets registered on BeginPlay before skills are initialized. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Skills CDS|Parameter Modifiers|Registration")
	TArray<TObjectPtr<USkillParameterModifierDataAssetCDS>> RegisteredParameterModifierAssets;

	/**
	 * Register a skill asset with this component.
	 * If a skill with the same tag already exists, it will be replaced.
	 *
	 * @param Asset The skill data asset to register
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Skills CDS|Skills")
	void RegisterSkill(USkillsDataAssetCDS *Asset);

	/**
	 * Register one parameter modifier asset for lookup by its ModifierTag.
	 * @param Asset Modifier data asset to register.
	 * @return True when the asset is valid and registered.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Skills CDS|Parameter Modifiers")
	bool RegisterParameterModifier(USkillParameterModifierDataAssetCDS *Asset);

	/**
	 * Register every valid modifier asset in a collection.
	 * @param Collection Collection asset to register.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Skills CDS|Parameter Modifiers")
	void RegisterParameterModifierCollection(USkillParameterModifierCollectionAssetCDS *Collection);

	/**
	 * Apply a registered modifier asset to one registered skill.
	 * Parameter tags absent from the skill are skipped. Reapplying the same modifier tag replaces its values and refreshes its duration.
	 * @param SkillTag Exact tag of the registered skill.
	 * @param ModifierTag Tag of the registered modifier asset.
	 * @return True when at least one operation targets a parameter on the skill.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Skills CDS|Parameter Modifiers")
	bool ApplyModParameters(FGameplayTag SkillTag, FGameplayTag ModifierTag);

	/**
	 * Apply a runtime-created modifier using the same evaluator as registered modifier assets.
	 * ModifierTag is optional; when omitted a new modifier instance is created. Use
	 * ApplyRuntimeModParametersWithHandle when individual removal is needed.
	 * @param SkillTag Exact tag of the registered skill.
	 * @param ModifierSpec Runtime modifier operations and duration.
	 * @return True when at least one operation targets a parameter on the skill.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Skills CDS|Parameter Modifiers")
	bool ApplyRuntimeModParameters(FGameplayTag SkillTag, const FSkillParameterModifierSpecCDS &ModifierSpec);

	/**
	 * Apply a runtime-created modifier and return its unique application handle.
	 * @param SkillTag Exact tag of the registered skill.
	 * @param ModifierSpec Runtime modifier operations and duration.
	 * @param OutModifierHandle Handle for removing this application later.
	 * @return True when at least one operation targets a parameter on the skill.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Skills CDS|Parameter Modifiers")
	bool ApplyRuntimeModParametersWithHandle(
		FGameplayTag SkillTag,
		const FSkillParameterModifierSpecCDS &ModifierSpec,
		FGuid &OutModifierHandle);

	/**
	 * Remove a tagged modifier from one skill.
	 * @param SkillTag Exact tag of the registered skill.
	 * @param ModifierTag Tag of the active modifier to remove.
	 * @return True when an active modifier was removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Skills CDS|Parameter Modifiers")
	bool RemoveModParameter(FGameplayTag SkillTag, FGameplayTag ModifierTag);

	/**
	 * Remove all active modifiers from skills whose exact tags are in SkillTags.
	 * @param SkillTags Exact skill tags whose active modifiers should be cleared.
	 * @return Number of active modifier instances removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Skills CDS|Parameter Modifiers")
	int32 RemoveModParameters(const FGameplayTagContainer &SkillTags);

	/**
	 * Get the registered base value after applying all active modifiers.
	 * Can be called from skill logic through Context.SourceComponent or from other systems such as UI.
	 * @param SkillTag Exact tag of the registered skill.
	 * @param ParameterTag Exact parameter tag defined by the skill asset.
	 * @param OutValue Effective parameter value when found.
	 * @return True when the skill and parameter were found and resolved.
	 */
	UFUNCTION(BlueprintPure, Category = "Dynamic Skills CDS|Parameter Modifiers")
	bool GetResolvedSkillParameterValue(
		FGameplayTag SkillTag,
		FGameplayTag ParameterTag,
		float &OutValue) const;

	/**
	 * Apply one caller-supplied operation to the registered base value, then apply active modifiers.
	 * This does not mutate the skill asset or store the supplied value. Can be called from skill logic
	 * through Context.SourceComponent or from other systems such as UI.
	 * @param SkillTag Exact tag of the registered skill.
	 * @param ParameterTag Exact parameter tag defined by the skill asset.
	 * @param BaseOperation Operation to apply to the asset's base value.
	 * @param Operand Value used by BaseOperation.
	 * @param OutValue Resolved value when found and valid.
	 * @return True when the skill and parameter were found and the result is finite.
	 */
	UFUNCTION(BlueprintPure, Category = "Dynamic Skills CDS|Parameter Modifiers")
	bool ResolveSkillParameterValue(
		FGameplayTag SkillTag,
		FGameplayTag ParameterTag,
		ESkillParameterOperationCDS BaseOperation,
		float Operand,
		float &OutValue) const;

	/**
	 * Remove one runtime modifier by its unique application handle.
	 * @param SkillTag Exact tag of the registered skill.
	 * @param ModifierHandle Handle returned by ApplyRuntimeModParametersWithHandle.
	 * @return True when the matching runtime modifier was removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Skills CDS|Parameter Modifiers")
	bool RemoveRuntimeModParameter(FGameplayTag SkillTag, FGuid ModifierHandle);

	/**
	 * Find skill runtime data by gameplay tag.
	 *
	 * @param Tag The gameplay tag to search for
	 * @return Pointer to runtime data, or nullptr if not found
	 */
	FSkillRuntimeData *FindSkill(FGameplayTag Tag);

	/**
	 * Find skill runtime data (const version).
	 *
	 * @param Tag The gameplay tag to search for
	 * @return Const pointer to runtime data, or nullptr if not found
	 */
	const FSkillRuntimeData *FindSkill(FGameplayTag Tag) const;

	/**
	 * Find skill data by tag and copy to output.
	 *
	 * @param Tag The gameplay tag to search for
	 * @param OutSkill Output structure for skill data
	 * @return True if skill was found, false otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "Dynamic Skills CDS|Skills")
	bool FindSkillData(FGameplayTag Tag, FSkillRuntimeData &OutSkill) const;

	/**
	 * Returns context data associated with the specified skill and context tag.
	 *
	 * @param SkillTag Skill gameplay tag.
	 * @param ContextTag Context gameplay tag.
	 * @return Context Data Asset or nullptr if not found.
	 */
	UFUNCTION(BlueprintPure, Category = "Dynamic Skills CDS|Skills")
	USkillContextDataAssetCDS *GetContextData(FGameplayTag SkillTag, FGameplayTag ContextTag) const;

	/**
	 * Attempt to activate a skill.
	 * Validates skill state, charges, and custom requirements before proceeding.
	 *
	 * @param Tag The gameplay tag of the skill to activate
	 * @param TargetActor Optional target actor for the skill (e.g., enemy, item)
	 * @param bCanStart Whether activation is allowed to proceed
	 * @return True if activation was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Skills CDS|Activation")
	bool TryActivateSkill(FGameplayTag Tag, AActor *TargetActor = nullptr, bool bCanStart = true);

	/**
	 * Commit a skill after charge completion.
	 * Consumes a charge and transitions to execution.
	 *
	 * @param Skill The skill runtime data to commit
	 */
	void CommitSkill(FSkillRuntimeData &Skill);

	/**
	 * Execute skill with full pipeline: Modifier -> Logic -> Executor -> Effect.
	 *
	 * @param Skill The skill runtime data to execute
	 */
	void ExecuteSkill(FSkillRuntimeData &Skill);

	/**
	 * Finish skill execution and transition to cooldown or idle.
	 *
	 * @param Skill The skill runtime data to finish
	 */
	void FinishSkill(FSkillRuntimeData &Skill);

	/**
	 * Interrupt a skill immediately.
	 * Behavior depends on the skill's interrupt policy and current state.
	 *
	 * @param Tag The gameplay tag of the skill to interrupt
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Skills CDS|Lifecycle")
	void InterruptSkill(FGameplayTag Tag);

	/**
	 * Interrupt a skill without checking its configured interrupt policy.
	 * Used by skill logic to cancel its own activation.
	 *
	 * @param Tag The gameplay tag of the skill to interrupt
	 * @return True if the skill was interrupted
	 */
	bool ForceInterruptSkill(FGameplayTag Tag);

	/**
	 * Check if a skill can be activated.
	 * Validates state, charges, and custom requirements.
	 *
	 * @param Skill The skill runtime data to check
	 * @return True if the skill can be activated, false otherwise
	 */
	bool CanActivateSkill(const FSkillRuntimeData &Skill) const;

	/**
	 * Check if a skill can be activated by tag.
	 *
	 * @param Tag The gameplay tag of the skill to check
	 * @return True if the skill can be activated, false otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "Dynamic Skills CDS|Validation")
	bool CanActivateSkillByTag(FGameplayTag Tag);

	/**
	 * Blueprint-overridable hook for custom activation validation.
	 * Used by derived classes to implement cost checking, resource requirements, etc.
	 *
	 * @param Skill The skill runtime data to validate
	 * @return True if custom validation passes, false otherwise
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Dynamic Skills CDS|Validation")
	bool CanActivateCustom(const FSkillRuntimeData &Skill) const;
	virtual bool CanActivateCustom_Implementation(const FSkillRuntimeData &Skill) const { return true; }

	/**
	 * Resolve an executor instance by execution type.
	 * Executors are cached and reused for performance.
	 *
	 * @param Type The execution type (Instant, Projectile, AOE)
	 * @return Executor instance, or nullptr if type is unsupported
	 */
	USkillExecutor *ResolveExecutor(ESkillExecutionType Type);

	// Event delegates for external systems
	UPROPERTY(BlueprintAssignable, Category = "Dynamic Skills CDS|Events")
	FOnSkillActivatedCDS OnSkillActivated;
	UPROPERTY(BlueprintAssignable, Category = "Dynamic Skills CDS|Events")
	FOnSkillCommittedCDS OnSkillCommitted;
	UPROPERTY(BlueprintAssignable, Category = "Dynamic Skills CDS|Events")
	FOnSkillExecutedCDS OnSkillExecuted;
	UPROPERTY(BlueprintAssignable, Category = "Dynamic Skills CDS|Events")
	FOnSkillFinishedCDS OnSkillFinished;
	UPROPERTY(BlueprintAssignable, Category = "Dynamic Skills CDS|Events")
	FOnSkillInterruptedCDS OnSkillInterrupted;
	UPROPERTY(BlueprintAssignable, Category = "Dynamic Skills CDS|Events")
	FOnCooldownChangedCDS OnCooldownChanged;
	UPROPERTY(BlueprintAssignable, Category = "Dynamic Skills CDS|Events")
	FOnChargesChangedCDS OnChargesChanged;
	UPROPERTY(BlueprintAssignable, Category = "Dynamic Skills CDS|Events")
	FOnSkillInterruptedReasonCDS OnSkillInterruptedReason;
	UPROPERTY(BlueprintAssignable, Category = "Dynamic Skills CDS|Events")
	FOnSkillTickCDS OnSkillTick;

protected:
	/**
	 * Maps skill gameplay tags to their indices in the Skills array.
	 * Used for fast O(1) lookup of skills by tag.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Dynamic Skills CDS|Runtime")
	TMap<FGameplayTag, int32> SkillLookup;

	/**
	 * Array of all registered skills with their runtime data.
	 * This is the canonical storage for skill state.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Dynamic Skills CDS|Runtime")
	TArray<FSkillRuntimeData> Skills;

	/** Registered modifier definitions indexed by their unique modifier tag. */
	UPROPERTY(VisibleAnywhere, Category = "Dynamic Skills CDS|Parameter Modifiers|Runtime")
	TMap<FGameplayTag, TObjectPtr<USkillParameterModifierDataAssetCDS>> ParameterModifierLookup;

	/**
	 * Cache of executor instances by execution type.
	 * Executors are lazily instantiated and reused.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Dynamic Skills CDS|Runtime")
	TMap<ESkillExecutionType, TObjectPtr<USkillExecutor>> Executors;

private:
	/**
	 * Initialize component lifecycle hooks.
	 */
	virtual void BeginPlay() override;

	/**
	 * Register skills from configured collections and direct assets.
	 */
	void RegisterSkillsFromCollections();
	/** Register modifier definitions from the configured collections and direct assets. */
	void RegisterParameterModifiersFromCollections();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Apply a validated modifier spec to runtime data for one skill.
	 * @param SkillTag Exact tag of the target skill.
	 * @param ModifierSpec Modifier operations and duration to copy into runtime.
	 * @param bRequireModifierTag Whether this call requires a stable modifier tag.
	 * @param OutModifierHandle Optional output handle for the active application.
	 * @return True when the modifier was applied to at least one parameter.
	 */
	bool ApplyParameterModifierSpec(
		FGameplayTag SkillTag,
		const FSkillParameterModifierSpecCDS &ModifierSpec,
		bool bRequireModifierTag,
		FGuid *OutModifierHandle);
	/**
	 * Clear all active parameter modifiers and their timers for one skill.
	 * @param Skill Runtime entry whose modifier records should be cleared.
	 */
	void ClearSkillParameterModifiers(FSkillRuntimeData &Skill) const;
	/**
	 * Clear the expiration timer owned by one active modifier.
	 * @param Modifier Active modifier whose timer should be cleared.
	 */
	void ClearParameterModifierTimer(FActiveSkillParameterModifierCDS &Modifier) const;
	/**
	 * Build a copy of the skill's base parameter map with active modifiers resolved.
	 * @param Skill Runtime entry whose asset and modifiers are evaluated.
	 * @param OutParameters Receives the resolved parameter map.
	 */
	void BuildResolvedParameters(const FSkillRuntimeData &Skill, TMap<FGameplayTag, float> &OutParameters) const;
	/**
	 * Resolve one known parameter from an explicit starting value and active modifiers.
	 * @param Skill Runtime entry containing the parameter definition and active modifiers.
	 * @param ParameterTag Exact parameter tag to evaluate.
	 * @param BaseValue Starting value before active modifiers.
	 * @param OutValue Receives the effective value when successful.
	 * @return True when the parameter exists and resolves to a finite value.
	 */
	bool ResolveParameterValue(const FSkillRuntimeData &Skill, FGameplayTag ParameterTag, float BaseValue, float &OutValue) const;
	/**
	 * Apply one arithmetic operation to a base value and validate the result.
	 * @param BaseValue Initial numeric value.
	 * @param Operation Arithmetic operation to apply.
	 * @param Operand Value used by the operation.
	 * @param OutValue Receives the result when finite.
	 * @return True when the operation is valid and the result is finite.
	 */
	bool ApplyParameterOperation(
		float BaseValue,
		ESkillParameterOperationCDS Operation,
		float Operand,
		float &OutValue) const;

	/**
	 * Remove an active modifier when its timer expires.
	 * @param SkillTag Exact tag of the skill owning the modifier.
	 * @param ApplicationId Unique identity of the expiring application.
	 */
	UFUNCTION()
	void HandleParameterModifierExpired(FGameplayTag SkillTag, FGuid ApplicationId);

	/**
	 * Check if a skill should be interrupted based on policy.
	 *
	 * @param Policy The skill's interrupt policy
	 * @return True if the skill should be interrupted, false otherwise
	 */
	bool ShouldInterrupt(ESkillInterruptPolicy Policy) const;

	/**
	 * Get the maximum number of charges for a skill.
	 *
	 * @param Asset The skill data asset
	 * @return The maximum charges (minimum 1)
	 */
	int32 GetMaxCharges(const USkillsDataAssetCDS *Asset) const;

	/**
	 * Start skill recharge cycle if auto-recharge is enabled.
	 *
	 * @param Skill The skill to start recharging
	 */
	void StartRecharge(FSkillRuntimeData &Skill);

	/**
	 * Apply skill effect by passing effect tag to external systems.
	 * This maintains independence from ComponentDynamicEffects (CDE).
	 *
	 * @param Context The execution context containing skill and effect data
	 */
	void ApplyEffectInternal(const FSkillExecutionContext &Context);

	/** Shared implementation for policy-checked and forced interruption. */
	bool InterruptSkillInternal(FGameplayTag Tag, bool bForce);

	/**
	 * Trigger animation montage associated with the skill.
	 *
	 * @param AnimTag The animation tag to play
	 * @param Asset The skill data asset containing animation data
	 */
	void PlayAnimation(const FGameplayTag &AnimTag, USkillsDataAssetCDS *Asset);

	/**
	 * Clear active and cooldown timers for a skill.
	 *
	 * @param Skill The skill whose timers should be cleared
	 */
	void ClearSkillTimers(FSkillRuntimeData &Skill) const;

	/**
	 * Calculate normalized charge progress (0.0 to 1.0).
	 * Takes into account charge curve if configured.
	 *
	 * @param Skill The skill to calculate charge for
	 * @return Normalized charge value (0.0 to 1.0)
	 */
	float CalculateChargeAlpha(const FSkillRuntimeData &Skill) const;

	/**
	 * Build execution context from skill runtime data.
	 * Populates source, source mesh, target, charge alpha, and other context information.
	 *
	 * @param Skill The skill to build context from
	 * @return Populated execution context
	 */
	FSkillExecutionContext BuildContext(FSkillRuntimeData &Skill);

	/**
	 * Resolve and cache the skeletal mesh used as the source for skill context.
	 * Prefers ISkillContextProviderCDS when implemented by the owner actor.
	 */
	USkeletalMeshComponent *ResolveSourceSkeletalMesh();

	/**
	 * Resolve the best context asset for a skill by gameplay-tag lookup.
	 */
	const USkillContextDataAssetCDS *ResolveSkillContext(const USkillsDataAssetCDS *Asset, const FGameplayTagContainer &CandidateTags) const;

	// Timer callbacks
	UFUNCTION()
	void HandleChargeTimeout(FGameplayTag Tag);

	UFUNCTION()
	void HandleActiveFinished(FGameplayTag Tag);

	UFUNCTION()
	void HandleCooldownFinished(FGameplayTag Tag);

	UFUNCTION()
	void HandleRechargeFinished(FGameplayTag Tag);

	/**
	 * Cached source skeletal mesh used for building skill execution contexts.
	 */
	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> CachedSourceSkeletalMesh = nullptr;
};
