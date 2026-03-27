// Pavel Gornostaev <https://github.com/Pavreally>

#pragma once

#include "CoreMinimal.h"
#include "SkillEnumsCDS.generated.h"

/**
 * Determines which executor system handles skill execution.
 * Executors implement attack patterns (melee swings, projectile trajectories, AOE patterns).
 */
UENUM(BlueprintType)
enum class ESkillExecutionType : uint8
{
	/** Direct melee attack (sword swing, punch, etc.) */
	Melee,
	
	/** Projectile-based attack (fireball, arrow, etc.) */
	Projectile,
	
	/** Area-of-effect attack (explosion radius, healing aura, etc.) */
	AOE
};

/**
 * Lifecycle state of a skill during execution.
 * Determines what actions and transitions are valid.
 */
UENUM(BlueprintType)
enum class ESkillState : uint8
{
	/** Skill is ready to be activated. */
	Idle,
	
	/** Skill has been activated but is waiting for commit (used for chargeable skills). */
	PreCast,
	
	/** Skill is charging (held by player). */
	Charging,
	
	/** Skill is executing (effects being applied). */
	Casting,
	
	/** Skill is in active duration (effects ongoing). */
	Active,
	
	/** Skill is in cooldown or recharge period. */
	Cooldown
};

/**
 * Determines when a skill can be interrupted during execution.
 * Interruption policies prevent uncontrolled skill stacking and resource abuse.
 */
UENUM(BlueprintType)
enum class ESkillInterruptPolicy : uint8
{
	/** Skill cannot be interrupted under any circumstances. */
	None,
	
	/** Skill can be interrupted by damage. */
	OnDamage,
	
	/** Skill can be interrupted by character movement. */
	OnMove,
	
	/** Skill can be interrupted by any action. */
	Always
};
