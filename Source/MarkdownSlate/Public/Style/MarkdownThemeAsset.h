#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Fonts/SlateFontInfo.h"
#include "Emoji/MarkdownEmojiTypes.h"
#include "MarkdownThemeAsset.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnMarkdownThemeChanged);

UCLASS(BlueprintType)
class MARKDOWNSLATE_API UMarkdownThemeAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Font")
	FSlateFontInfo DefaultFont;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Font")
	FSlateFontInfo BoldFont;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Font")
	FSlateFontInfo ItalicFont;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Font")
	FSlateFontInfo EmojiFont;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Font")
	int32 BodyFontSize = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heading")
	FLinearColor HeadingColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
	FLinearColor BodyTextColor = FLinearColor(0.9f, 0.9f, 0.9f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Code")
	FLinearColor CodeBackgroundColor = FLinearColor(0.1f, 0.1f, 0.12f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Code")
	FLinearColor CodeTextColor = FLinearColor(0.95f, 0.75f, 0.35f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Code|Inline")
	float CodeCornerRadius = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Code|Inline")
	float CodePaddingH = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Code|Inline")
	float CodePaddingV = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Code|Block")
	float CodeBlockCornerRadius = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Code|Block")
	float CodeBlockPaddingH = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Code|Block")
	float CodeBlockPaddingV = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Link")
	FLinearColor LinkColor = FLinearColor(0.3f, 0.6f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blockquote")
	FLinearColor BlockquoteBackgroundColor = FLinearColor(0.15f, 0.15f, 0.18f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blockquote")
	float BlockquoteCornerRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blockquote")
	float BlockquoteBorderWidth = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blockquote")
	FLinearColor BlockquoteBorderColor = FLinearColor(0.3f, 0.3f, 0.35f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blockquote")
	float BlockquotePaddingH = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blockquote")
	float BlockquotePaddingV = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "List")
	FLinearColor ListMarkerColor = FLinearColor(0.7f, 0.7f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "List")
	FLinearColor ListTextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "List")
	float ListItemIndent = 16.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Table")
	FLinearColor TableHeaderBgColor = FLinearColor(0.15f, 0.15f, 0.18f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Table")
	FLinearColor TableRowEvenBgColor = FLinearColor(0.09f, 0.09f, 0.11f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Table")
	FLinearColor TableRowOddBgColor = FLinearColor(0.06f, 0.06f, 0.08f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Table")
	FLinearColor TableBorderColor = FLinearColor(0.12f, 0.12f, 0.14f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Table")
	float TableCellPaddingH = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Table")
	float TableCellPaddingV = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Table")
	float TableBorderThickness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Separator")
	float HorizontalRuleThickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Separator")
	FLinearColor HorizontalRuleColor = FLinearColor(0.35f, 0.35f, 0.4f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Emoji")
	bool bEnableEmojiRendering = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Emoji")
	EMarkdownEmojiRenderMode EmojiRenderMode = EMarkdownEmojiRenderMode::TwemojiFirst;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Emoji")
	float EmojiSizeScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Emoji")
	FString TwemojiAssetRoot = TEXT("Content/Emoji");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Emoji")
	bool bAllowTwemojiFallback = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout")
	float ParagraphSpacing = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout")
	float WrapTextWidth = 600.0f;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Incremented on every property edit; widgets compare to detect changes
	int32 Generation = 0;
};
