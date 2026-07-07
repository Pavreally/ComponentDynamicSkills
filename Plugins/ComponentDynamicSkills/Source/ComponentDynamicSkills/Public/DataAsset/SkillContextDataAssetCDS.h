// Pavel Gornostaev <https://github.com/Pavreally>

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "NiagaraSystem.h"
#include "SkillContextDataAssetCDS.generated.h"

class AActor;
class UAnimMontage;
class UCurveFloat;
class UMaterialInterface;
class UNiagaraSystem;
class USoundBase;
class UEffectDataCDS;

UCLASS(BlueprintType)
class COMPONENTDYNAMICSKILLS_API USkillContextDataAssetCDS : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|Bool")
	TMap<FGameplayTag, bool> BoolParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|Float")
	TMap<FGameplayTag, float> FloatParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|Int")
	TMap<FGameplayTag, int32> IntParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|Vector")
	TMap<FGameplayTag, FVector> VectorParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|Name")
	TMap<FGameplayTag, FName> NameParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|String")
	TMap<FGameplayTag, FString> StringParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|Text")
	TMap<FGameplayTag, FText> TextParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|GameplayTag")
	TMap<FGameplayTag, FGameplayTag> GameplayTagParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|GameplayTagContainer")
	TMap<FGameplayTag, FGameplayTagContainer> GameplayTagContainerParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|Animation")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> AnimationParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|Sound")
	TMap<FGameplayTag, TObjectPtr<USoundBase>> SoundParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|Niagara")
	TMap<FGameplayTag, TObjectPtr<UNiagaraSystem>> NiagaraParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|Effect")
	TMap<FGameplayTag, TObjectPtr<UEffectDataCDS>> EffectParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|Decal")
	TMap<FGameplayTag, TObjectPtr<UMaterialInterface>> DecalsParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|Actor")
	TMap<FGameplayTag, TSubclassOf<AActor>> ActorParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|Curve")
	TMap<FGameplayTag, TObjectPtr<UCurveFloat>> CurveParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|Table")
	TMap<FGameplayTag, FDataTableRowHandle> TableParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context|Parameters|OtherAssets")
	TMap<FGameplayTag, TObjectPtr<UObject>> AssetParameters;
};
