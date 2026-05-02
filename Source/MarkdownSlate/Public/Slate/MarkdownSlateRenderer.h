#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Render/MarkdownRenderNode.h"

struct FMarkdownSlateThemeConfig
{
	FSlateFontInfo DefaultFont;
	FSlateFontInfo BoldFont;
	FSlateFontInfo ItalicFont;
	FSlateFontInfo BoldItalicFont;
	FSlateFontInfo MonospaceFont;

	int32 H1FontSize = 28;
	int32 H2FontSize = 22;
	int32 H3FontSize = 19;
	int32 H4FontSize = 16;
	int32 H5FontSize = 14;
	int32 H6FontSize = 13;
	int32 BodyFontSize = 12;
	int32 CodeFontSize = 11;

	FLinearColor HeadingColor = FLinearColor::White;
	FLinearColor BodyTextColor = FLinearColor(0.9f, 0.9f, 0.9f);
	FLinearColor StrongColor = FLinearColor::White;
	FLinearColor EmphasisColor = FLinearColor(0.9f, 0.9f, 0.9f);
	FLinearColor CodeBackgroundColor = FLinearColor(0.1f, 0.1f, 0.12f);
	FLinearColor CodeTextColor = FLinearColor(0.95f, 0.75f, 0.35f);
	FLinearColor LinkColor = FLinearColor(0.2f, 0.5f, 1.0f);
	FLinearColor BackgroundColor = FLinearColor(0.08f, 0.08f, 0.1f);
	FLinearColor BlockquoteBackgroundColor = FLinearColor(0.15f, 0.15f, 0.18f);

	float ParagraphSpacing = 4.0f;
	float WrapTextWidth = 600.0f;

	// Inline code
	float CodeCornerRadius = 3.0f;
	float CodePaddingH = 4.0f;
	float CodePaddingV = 1.0f;

	// Code block
	float CodeBlockCornerRadius = 6.0f;
	float CodeBlockPaddingH = 14.0f;
	float CodeBlockPaddingV = 10.0f;

	// Blockquote
	float BlockquoteCornerRadius = 0.0f;
	float BlockquoteBorderWidth = 4.0f;
	FLinearColor BlockquoteBorderColor = FLinearColor(0.3f, 0.3f, 0.35f);
	float BlockquotePaddingH = 14.0f;
	float BlockquotePaddingV = 6.0f;

	// Horizontal rule
	float HorizontalRuleThickness = 2.0f;
	FLinearColor HorizontalRuleColor = FLinearColor(0.35f, 0.35f, 0.4f);

	TFunction<void(const FString&)> OnLinkClicked;

	static FMarkdownSlateThemeConfig Default();
};

class MARKDOWNSLATE_API FMarkdownSlateRenderer
{
public:
	static TSharedRef<SWidget> Render(const TSharedPtr<FMarkdownRenderNode>& RootNode, const FMarkdownSlateThemeConfig& Theme);
	static TSharedRef<SWidget> RenderNode(const TSharedPtr<FMarkdownRenderNode>& Node, const FMarkdownSlateThemeConfig& Theme);
};
