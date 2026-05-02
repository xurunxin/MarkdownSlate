#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Slate/SMarkdownView.h"
#include "Style/MarkdownThemeAsset.h"
#include "MarkdownWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMarkdownLinkClickedBP, const FString&, Url);

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Markdown View"))
class MARKDOWNSLATE_API UMarkdownWidget : public UWidget
{
	GENERATED_BODY()

public:
	UMarkdownWidget(const FObjectInitializer& ObjectInitializer);

	// Content
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Content", meta = (MultiLine = true))
	FString MarkdownText;

	// Theme — all styling comes from this Data Asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Theme")
	TObjectPtr<UMarkdownThemeAsset> Theme;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Markdown|Events")
	FOnMarkdownLinkClickedBP OnLinkClicked;

	UFUNCTION(BlueprintCallable, Category = "Markdown")
	void SetMarkdownText(const FString& InText);

	UFUNCTION(BlueprintCallable, Category = "Markdown")
	void RefreshDisplayMarkdown();

	virtual void SynchronizeProperties() override;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

private:
	TSharedPtr<SMarkdownView> MySlateWidget;
	int32 LastThemeGeneration = -1;
	void OnNativeLinkClicked(const FString& Url);
	FMarkdownSlateThemeConfig BuildThemeConfig() const;
};
