#include "Factories/MarkdownThemeAssetFactory.h"
#include "Style/MarkdownThemeAsset.h"

UMarkdownThemeAssetFactory::UMarkdownThemeAssetFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UMarkdownThemeAsset::StaticClass();
}

UObject* UMarkdownThemeAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName,
	EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UMarkdownThemeAsset>(InParent, InClass, InName, Flags);
}
