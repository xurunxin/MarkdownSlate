#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Slate/SMarkdownView.h"
#include "Style/MarkdownThemeAsset.h"
#include "Fonts/SlateFontInfo.h"
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

	// Theme preset (optional override)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Theme")
	TObjectPtr<UMarkdownThemeAsset> ThemePreset;

	// Fonts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Font")
	FSlateFontInfo DefaultFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Font")
	FSlateFontInfo BoldFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Font")
	FSlateFontInfo ItalicFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Font")
	FSlateFontInfo BoldItalicFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Font")
	FSlateFontInfo MonospaceFont;

	// Heading Sizes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Heading")
	int32 H1FontSize = 28;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Heading")
	int32 H2FontSize = 22;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Heading")
	int32 H3FontSize = 19;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Heading")
	int32 H4FontSize = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Heading")
	int32 H5FontSize = 14;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Heading")
	int32 H6FontSize = 13;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Heading")
	FLinearColor HeadingColor = FLinearColor::White;

	// Body
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Body")
	int32 BodyFontSize = 12;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Body")
	FLinearColor BodyTextColor = FLinearColor(0.9f, 0.9f, 0.9f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Body")
	FLinearColor StrongColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Body")
	FLinearColor EmphasisColor = FLinearColor(0.9f, 0.9f, 0.9f);

	// Code
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Code")
	int32 CodeFontSize = 11;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Code")
	FLinearColor CodeBackgroundColor = FLinearColor(0.1f, 0.1f, 0.12f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Code")
	FLinearColor CodeTextColor = FLinearColor(0.95f, 0.75f, 0.35f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Code|Inline")
	float CodeCornerRadius = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Code|Inline")
	float CodePaddingH = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Code|Inline")
	float CodePaddingV = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Code|Block")
	float CodeBlockCornerRadius = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Code|Block")
	float CodeBlockPaddingH = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Code|Block")
	float CodeBlockPaddingV = 10.0f;

	// Link
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Link")
	FLinearColor LinkColor = FLinearColor(0.25f, 0.55f, 1.0f);

	// Background
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Background")
	FLinearColor BackgroundColor = FLinearColor(0.08f, 0.08f, 0.1f);

	// Blockquote
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Blockquote")
	FLinearColor BlockquoteBackgroundColor = FLinearColor(0.15f, 0.15f, 0.18f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Blockquote")
	float BlockquoteCornerRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Blockquote")
	float BlockquoteBorderWidth = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Blockquote")
	FLinearColor BlockquoteBorderColor = FLinearColor(0.3f, 0.3f, 0.35f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Blockquote")
	float BlockquotePaddingH = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Blockquote")
	float BlockquotePaddingV = 6.0f;

	// Horizontal Rule
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Separator")
	float HorizontalRuleThickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Separator")
	FLinearColor HorizontalRuleColor = FLinearColor(0.35f, 0.35f, 0.4f);

	// Layout
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Layout")
	float ParagraphSpacing = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markdown|Layout")
	float WrapTextWidth = 600.0f;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Markdown|Events")
	FOnMarkdownLinkClickedBP OnLinkClicked;

	UFUNCTION(BlueprintCallable, Category = "Markdown")
	void SetMarkdownText(const FString& InText);

	UFUNCTION(BlueprintCallable, Category = "Markdown")
	void RefreshDisplayMarkdown();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

private:
	TSharedPtr<SMarkdownView> MySlateWidget;

	void OnNativeLinkClicked(const FString& Url);
	FMarkdownSlateThemeConfig BuildThemeConfig() const;
};
