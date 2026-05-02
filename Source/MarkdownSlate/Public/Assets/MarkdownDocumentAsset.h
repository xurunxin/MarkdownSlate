#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MarkdownDocumentAsset.generated.h"

class UMarkdownThemeAsset;

UCLASS(BlueprintType)
class MARKDOWNSLATE_API UMarkdownDocumentAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Markdown")
	FString MarkdownContent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Markdown")
	FName SourceFilename;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme")
	TObjectPtr<UMarkdownThemeAsset> OverrideTheme;
};
