#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Fonts/SlateFontInfo.h"
#include "MarkdownThemeAsset.generated.h"

UCLASS(BlueprintType)
class MARKDOWNSLATE_API UMarkdownThemeAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Fonts
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Font")
	FSlateFontInfo DefaultFont;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Font")
	FSlateFontInfo BoldFont;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Font")
	FSlateFontInfo ItalicFont;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Font")
	FSlateFontInfo BoldItalicFont;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Font")
	FSlateFontInfo MonospaceFont;

	// Heading
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heading")
	FLinearColor HeadingColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heading")
	int32 H1FontSize = 28;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heading")
	int32 H2FontSize = 22;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heading")
	int32 H3FontSize = 19;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heading")
	int32 H4FontSize = 16;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heading")
	int32 H5FontSize = 14;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heading")
	int32 H6FontSize = 13;

	// Body
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
	int32 BodyFontSize = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
	FLinearColor BodyTextColor = FLinearColor(0.9f, 0.9f, 0.9f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
	FLinearColor StrongColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
	FLinearColor EmphasisColor = FLinearColor(0.9f, 0.9f, 0.9f);

	// Code
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Code")
	int32 CodeFontSize = 11;

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

	// Link
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Link")
	FLinearColor LinkColor = FLinearColor(0.25f, 0.55f, 1.0f);

	// Background
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Background")
	FLinearColor BackgroundColor = FLinearColor(0.08f, 0.08f, 0.1f);

	// Blockquote
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

	// Horizontal Rule
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Separator")
	float HorizontalRuleThickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Separator")
	FLinearColor HorizontalRuleColor = FLinearColor(0.35f, 0.35f, 0.4f);

	// Layout
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout")
	float ParagraphSpacing = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout")
	float WrapTextWidth = 600.0f;
};
