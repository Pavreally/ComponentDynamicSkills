// Pavel Gornostaev <https://github.com/Pavreally>

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SkillExecutionContextCDS.h"
#include "SkillModifierBase.generated.h"

/**
 * Minimal base class for author-defined skill modifiers.
 * Runtime application stays in CDS execution flow and is not implemented here.
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class COMPONENTDYNAMICSKILLS_API USkillModifierBase : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Modify skill execution context before executor runs.
	 * Use this to apply scaling factors, adjust targeting, or modify damage.
	 * This hook is called BEFORE the executor system and skill logic.
	 *
	 * @param Context The execution context to modify. Changes persist through execution pipeline.
	 */
	virtual void ModifyExecution(const FSkillExecutionContext &Context) {}
};
