#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MarkdownSlateSettings.generated.h"

class UMarkdownThemeAsset;

UCLASS(config=Engine, defaultconfig, meta=(DisplayName="MarkdownSlate"))
class MARKDOWNSLATE_API UMarkdownSlateSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, config, Category = "MarkdownSlate")
	bool bEnableCache = true;

	UPROPERTY(EditAnywhere, config, Category = "MarkdownSlate")
	TSoftObjectPtr<UMarkdownThemeAsset> DefaultTheme;

	virtual FName GetCategoryName() const override { return FName(TEXT("Plugins")); }
	virtual FName GetSectionName() const override { return FName(TEXT("MarkdownSlate")); }
};
