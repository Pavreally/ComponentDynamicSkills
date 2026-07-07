// Pavel Gornostaev <https://github.com/Pavreally>

#include "Components/ActorCDS.h"
#include "DataAsset/SkillsDataAssetCDS.h"
#include "DataAsset/SkillLogicBaseCDS.h"
#include "Engine/World.h"

UActorCDS::UActorCDS()
{
	// Disable component ticking - all timers are managed via FTimerManager
	PrimaryComponentTick.bCanEverTick = false;
	SetComponentTickEnabled(false);
}

void UActorCDS::BeginPlay()
{
	Super::BeginPlay();
	RegisterSkillsFromCollections();
}

void UActorCDS::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up all active timers before component destruction
	for (FSkillRuntimeData &Skill : Skills)
	{
		ClearSkillTimers(Skill);
		Skill.State = ESkillState::Idle;
		Skill.CurrentCooldown = 0.0f;
		Skill.ChargeStartTime = 0.0f;
	}

	Super::EndPlay(EndPlayReason);
}
