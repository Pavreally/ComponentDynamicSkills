#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/Interface.h"
#include "SkillContextProviderCDS.generated.h"

/**
 * Optional interface for actors that need explicit control over the context
 * injected into CDS skill logic.
 *
 * Implement this on the owner actor of UActorCDS when the default "first
 * skeletal mesh component on the actor" lookup is not sufficient.
 */
UINTERFACE(BlueprintType)
class COMPONENTDYNAMICSKILLS_API USkillContextProviderCDS : public UInterface
{
	GENERATED_BODY()
};

class COMPONENTDYNAMICSKILLS_API ISkillContextProviderCDS
{
	GENERATED_BODY()

public:
	/**
	 * Returns the skeletal mesh component that CDS should inject into
	 * FSkillExecutionContext::SourceSkeletalMesh.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CDS|Context")
	USkeletalMeshComponent *GetSkillSkeletalMeshComponent() const;
};
