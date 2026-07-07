// Pavel Gornostaev <https://github.com/Pavreally>

#include "DataAsset/SkillsDataAssetCDS.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Parameter_MinDamage, "CDS.Parameters.MinDamage");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Parameter_MaxDamage, "CDS.Parameters.MaxDamage");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Parameter_MinRadius, "CDS.Parameters.MinRadius");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Parameter_MaxRadius, "CDS.Parameters.MaxRadius");

USkillsDataAssetCDS::USkillsDataAssetCDS()
{
}

void USkillsDataAssetCDS::PostInitProperties()
{
	Super::PostInitProperties();

	if (!HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad))
	{
		if (!Parameters.Contains(TAG_Parameter_MinDamage))
			Parameters.Add(TAG_Parameter_MinDamage, 0.0f);

		if (!Parameters.Contains(TAG_Parameter_MaxDamage))
			Parameters.Add(TAG_Parameter_MaxDamage, 0.0f);

		if (!Parameters.Contains(TAG_Parameter_MinRadius))
			Parameters.Add(TAG_Parameter_MinRadius, 0.0f);

		if (!Parameters.Contains(TAG_Parameter_MaxRadius))
			Parameters.Add(TAG_Parameter_MaxRadius, 0.0f);
	}
}
