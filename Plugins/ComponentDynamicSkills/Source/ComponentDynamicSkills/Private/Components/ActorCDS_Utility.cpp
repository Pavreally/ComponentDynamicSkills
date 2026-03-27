// Pavel Gornostaev <https://github.com/Pavreally>

#include "Components/ActorCDS.h"
#include "DataAsset/SkillLogicBaseCDS.h"
#include "DataAsset/SkillsDataAssetCDS.h"
#include "Executors/SkillExecutor.h"
#include "Interfaces/SkillContextProviderCDS.h"

#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "Engine/World.h"

USkillExecutor *UActorCDS::ResolveExecutor(ESkillExecutionType Type)
{
	// Return cached executor if available
	if (TObjectPtr<USkillExecutor> *Found = Executors.Find(Type))
	{
		return *Found;
	}

	// Create appropriate executor instance based on type
	USkillExecutor *NewExecutor = nullptr;

	switch (Type)
	{
	case ESkillExecutionType::Melee:
		NewExecutor = NewObject<UMeleeSkillExecutor>(this);
		break;

	case ESkillExecutionType::Projectile:
		NewExecutor = NewObject<UProjectileSkillExecutor>(this);
		break;

	case ESkillExecutionType::AOE:
		NewExecutor = NewObject<UAOESkillExecutor>(this);
		break;

	default:
		NewExecutor = NewObject<USkillExecutor>(this);
	}

	// Cache and return executor
	if (NewExecutor)
	{
		Executors.Add(Type, NewExecutor);
	}

	return NewExecutor;
}

bool UActorCDS::ShouldInterrupt(ESkillInterruptPolicy Policy) const
{
	// Check if skill can be interrupted based on policy
	switch (Policy)
	{
	case ESkillInterruptPolicy::Always:
		return true;
	case ESkillInterruptPolicy::OnDamage:
	case ESkillInterruptPolicy::OnMove:
	case ESkillInterruptPolicy::None:
	default:
		// Currently only "Always" is implemented as an interrupt trigger
		// OnDamage and OnMove require external damage/movement systems
		return false;
	}
}

void UActorCDS::ClearSkillTimers(FSkillRuntimeData &Skill) const
{
	// Clear both active and cooldown timers
	if (UWorld *World = GetWorld())
	{
		FTimerManager &TimerManager = World->GetTimerManager();
		if (TimerManager.IsTimerActive(Skill.ActiveTimer))
		{
			TimerManager.ClearTimer(Skill.ActiveTimer);
		}
		if (TimerManager.IsTimerActive(Skill.CooldownTimer))
		{
			TimerManager.ClearTimer(Skill.CooldownTimer);
		}
	}
}

float UActorCDS::CalculateChargeAlpha(const FSkillRuntimeData &Skill) const
{
	// Return full charge for non-chargeable skills
	if (Skill.Asset == nullptr || !Skill.Asset->bIsChargeable)
	{
		return 1.0f;
	}

	// Return default if no world or charging time not set
	const UWorld *World = GetWorld();
	if (World == nullptr || Skill.ChargeStartTime <= 0.0f)
	{
		return 1.0f;
	}

	// Calculate elapsed time and charge time window
	const float ChargeDuration = FMath::Max(0.0f, World->GetTimeSeconds() - Skill.ChargeStartTime);
	const float MinChargeTime = FMath::Max(0.0f, Skill.Asset->MinChargeTime);
	const float MaxChargeTime = FMath::Max(MinChargeTime, Skill.Asset->MaxChargeTime);

	// Normalize charge value to 0..1 range
	float ChargeAlpha = 1.0f;
	if (MaxChargeTime > MinChargeTime)
	{
		ChargeAlpha = FMath::Clamp((ChargeDuration - MinChargeTime) / (MaxChargeTime - MinChargeTime), 0.0f, 1.0f);
	}
	else if (MaxChargeTime > 0.0f)
	{
		ChargeAlpha = FMath::Clamp(ChargeDuration / MaxChargeTime, 0.0f, 1.0f);
	}

	// Apply charge curve if configured
	if (Skill.Asset->ChargeCurve != nullptr)
	{
		ChargeAlpha = Skill.Asset->ChargeCurve->GetFloatValue(ChargeAlpha);
	}

	return FMath::Clamp(ChargeAlpha, 0.0f, 1.0f);
}

int32 UActorCDS::GetMaxCharges(const USkillsDataAssetCDS *Asset) const
{
	// Return maximum charges (minimum 1 charge always)
	return Asset != nullptr ? FMath::Max(1, Asset->MaxCharges) : 1;
}

USkeletalMeshComponent *UActorCDS::ResolveSourceSkeletalMesh()
{
	if (CachedSourceSkeletalMesh != nullptr)
	{
		return CachedSourceSkeletalMesh;
	}

	AActor *Owner = GetOwner();
	if (Owner == nullptr)
	{
		return nullptr;
	}

	if (Owner->GetClass()->ImplementsInterface(USkillContextProviderCDS::StaticClass()))
	{
		CachedSourceSkeletalMesh = ISkillContextProviderCDS::Execute_GetSkillSkeletalMeshComponent(Owner);
	}

	if (CachedSourceSkeletalMesh == nullptr)
	{
		CachedSourceSkeletalMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	}

	return CachedSourceSkeletalMesh;
}

FSkillExecutionContext UActorCDS::BuildContext(FSkillRuntimeData &Skill)
{
	// Populate execution context with all relevant data
	FSkillExecutionContext Context;
	Context.Source = GetOwner();
	Context.SourceComponent = this;
	Context.Target = Skill.TargetActor;
	Context.SourceSkeletalMesh = ResolveSourceSkeletalMesh();
	Context.Data = Skill.Asset;
	Context.ChargeAlpha = CalculateChargeAlpha(Skill);
	return Context;
}

void UActorCDS::ApplyEffectInternal(const FSkillExecutionContext &Context)
{
	// Validate context and effect  
	if (!Context.Data || !Context.Data->Effect)
		return;

	// Log effect application for debugging and external system notification
	UE_LOG(LogTemp, Log, TEXT("CDS → CDE: Apply EffectTag [%s] | Skill [%s] | Charge %.2f"),
			*Context.Data->Effect->EffectTag.ToString(),
			*Context.Data->SkillTag.ToString(),
			Context.ChargeAlpha);
	
	// TODO: Pass effect tag and context to Component Dynamic Effects (CDE)
}

void UActorCDS::PlayAnimation(const FGameplayTag &AnimTag, USkillsDataAssetCDS *Asset)
{
	// Validate asset
	if (!Asset)
		return;
	
	// Find animation montage by tag
	if (const TObjectPtr<UAnimMontage> *Montage = Asset->Animations.Find(AnimTag))
	{
		// TODO: Play montage through owner's skeletal mesh component (Montage_Play)
		UE_LOG(LogTemp, Log, TEXT("CDS Animation: %s"), *AnimTag.ToString());
	}
}
