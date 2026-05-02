#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Render/MarkdownRenderNode.h"
#include "Emoji/MarkdownEmojiTypes.h"

struct FMarkdownSlateThemeConfig
{
	FSlateFontInfo DefaultFont;
	FSlateFontInfo BoldFont;
	FSlateFontInfo ItalicFont;

	int32 BodyFontSize = 12;

	FLinearColor HeadingColor = FLinearColor::White;
	FLinearColor BodyTextColor = FLinearColor(0.9f, 0.9f, 0.9f);
	FLinearColor CodeBackgroundColor = FLinearColor(0.1f, 0.1f, 0.12f);
	FLinearColor CodeTextColor = FLinearColor(0.95f, 0.75f, 0.35f);
	FLinearColor LinkColor = FLinearColor(0.3f, 0.6f, 1.0f);
	FLinearColor BackgroundColor = FLinearColor(0.08f, 0.08f, 0.1f);
	FLinearColor BlockquoteBackgroundColor = FLinearColor(0.15f, 0.15f, 0.18f);

	float ParagraphSpacing = 4.0f;
	float WrapTextWidth = 600.0f;

	float CodeCornerRadius = 3.0f;
	float CodePaddingH = 4.0f;
	float CodePaddingV = 1.0f;
	float CodeBlockCornerRadius = 6.0f;
	float CodeBlockPaddingH = 14.0f;
	float CodeBlockPaddingV = 10.0f;

	float BlockquoteCornerRadius = 0.0f;
	float BlockquoteBorderWidth = 4.0f;
	FLinearColor BlockquoteBorderColor = FLinearColor(0.3f, 0.3f, 0.35f);
	float BlockquotePaddingH = 14.0f;
	float BlockquotePaddingV = 6.0f;

	float HorizontalRuleThickness = 2.0f;
	FLinearColor HorizontalRuleColor = FLinearColor(0.35f, 0.35f, 0.4f);

	FLinearColor ListMarkerColor = FLinearColor(0.7f, 0.7f, 0.7f);
	FLinearColor ListTextColor = FLinearColor::White;
	float ListItemIndent = 16.0f;

	FLinearColor TableHeaderBgColor = FLinearColor(0.15f, 0.15f, 0.18f);
	FLinearColor TableRowEvenBgColor = FLinearColor(0.09f, 0.09f, 0.11f);
	FLinearColor TableRowOddBgColor = FLinearColor(0.06f, 0.06f, 0.08f);
	FLinearColor TableBorderColor = FLinearColor(0.12f, 0.12f, 0.14f);
	float TableCellPaddingH = 8.0f;
	float TableCellPaddingV = 5.0f;
	float TableBorderThickness = 1.0f;

	bool bEnableEmojiRendering = true;
	EMarkdownEmojiRenderMode EmojiRenderMode = EMarkdownEmojiRenderMode::PlatformFontFirst;
	FString TwemojiAssetRoot = TEXT("Content/Emoji");
	float EmojiSizeScale = 1.0f;
	bool bAllowTwemojiFallback = true;

	class IMarkdownEmojiAssetProvider* EmojiProvider = nullptr;
	TFunction<void(const FString&)> OnLinkClicked;

	uint32 ComputeHash() const;
	static FMarkdownSlateThemeConfig Default();
};

class MARKDOWNSLATE_API FMarkdownSlateRenderer
{
public:
	static TSharedRef<SWidget> Render(const TSharedPtr<FMarkdownRenderNode>& RootNode, const FMarkdownSlateThemeConfig& Theme);
	static TSharedRef<SWidget> RenderNode(const TSharedPtr<FMarkdownRenderNode>& Node, const FMarkdownSlateThemeConfig& Theme);
	static TSharedRef<SWidget> RenderInlines(const TSharedPtr<FMarkdownRenderNode>& Node, const FMarkdownSlateThemeConfig& Theme);
};
