#include "Slate/MarkdownSlateRenderer.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Styling/CoreStyle.h"
#include "Brushes/SlateRoundedBoxBrush.h"

// Cached rounded brushes — updated when theme changes, shared across all widgets
struct FRoundedBrushCache
{
	FSlateRoundedBoxBrush CodeBrush;
	FSlateRoundedBoxBrush CodeBlockBrush;
	FSlateRoundedBoxBrush BlockquoteBrush;

	FRoundedBrushCache()
		: CodeBrush(FLinearColor::Black, 0.0f)
		, CodeBlockBrush(FLinearColor::Black, 0.0f)
		, BlockquoteBrush(FLinearColor::Black, 0.0f)
	{}

	void Update(const FMarkdownSlateThemeConfig& Theme)
	{
		CodeBrush       = FSlateRoundedBoxBrush(Theme.CodeBackgroundColor, Theme.CodeCornerRadius);
		CodeBlockBrush  = FSlateRoundedBoxBrush(Theme.CodeBackgroundColor, Theme.CodeBlockCornerRadius);
		BlockquoteBrush = FSlateRoundedBoxBrush(Theme.BlockquoteBackgroundColor, Theme.BlockquoteCornerRadius);
	}

	static FRoundedBrushCache& Get()
	{
		static FRoundedBrushCache Cache;
		return Cache;
	}
};

FMarkdownSlateThemeConfig FMarkdownSlateThemeConfig::Default()
{
	FMarkdownSlateThemeConfig Config;
	Config.DefaultFont = FCoreStyle::GetDefaultFontStyle("Regular", Config.BodyFontSize);
	Config.BoldFont = FCoreStyle::GetDefaultFontStyle("Bold", Config.BodyFontSize);
	Config.ItalicFont = FCoreStyle::GetDefaultFontStyle("Italic", Config.BodyFontSize);
	Config.BoldItalicFont = FCoreStyle::GetDefaultFontStyle("BoldItalic", Config.BodyFontSize);
	Config.MonospaceFont = FCoreStyle::GetDefaultFontStyle("Mono", Config.CodeFontSize);
	return Config;
}

static FSlateFontInfo BuildFont(const FSlateFontInfo& BaseFont, int32 Size)
{
	FSlateFontInfo Font = BaseFont;
	Font.Size = Size;
	return Font;
}

// ---- inline rendering helpers ----

// Extract display text from a node: uses TextContent if set, otherwise concatenates child PlainText nodes
static FText GetInlineText(const TSharedPtr<FMarkdownRenderNode>& Node)
{
	if (!Node->TextContent.IsEmpty())
	{
		return Node->TextContent;
	}
	FString Combined;
	for (const auto& Child : Node->Children)
	{
		if (Child->Type == EMarkdownRenderNodeType::PlainText)
		{
			Combined += Child->TextContent.ToString();
		}
	}
	return FText::FromString(Combined);
}

static TSharedRef<SWidget> RenderInlines(const TSharedPtr<FMarkdownRenderNode>& Node, const FMarkdownSlateThemeConfig& Theme);

static TSharedRef<SWidget> RenderInlineNode(const TSharedPtr<FMarkdownRenderNode>& Node, const FMarkdownSlateThemeConfig& Theme)
{
	if (!Node.IsValid()) return SNullWidget::NullWidget;

	switch (Node->Type)
	{
	case EMarkdownRenderNodeType::PlainText:
		return SNew(STextBlock)
			.Text(Node->TextContent)
			.Font(BuildFont(Theme.DefaultFont, Theme.BodyFontSize))
			.ColorAndOpacity(FSlateColor(Theme.BodyTextColor));

	case EMarkdownRenderNodeType::Strong:
		return SNew(STextBlock)
			.Text(GetInlineText(Node))
			.Font(BuildFont(Theme.BoldFont, Theme.BodyFontSize))
			.ColorAndOpacity(FSlateColor(Theme.StrongColor));

	case EMarkdownRenderNodeType::Emphasis:
		return SNew(STextBlock)
			.Text(GetInlineText(Node))
			.Font(BuildFont(Theme.ItalicFont, Theme.BodyFontSize))
			.ColorAndOpacity(FSlateColor(Theme.EmphasisColor));

	case EMarkdownRenderNodeType::CodeInline:
	{
		FRoundedBrushCache::Get().Update(Theme);
		return SNew(SBorder)
			.BorderImage(&FRoundedBrushCache::Get().CodeBrush)
			.Padding(FMargin(Theme.CodePaddingH, Theme.CodePaddingV))
			[
				SNew(STextBlock)
				.Text(GetInlineText(Node))
				.Font(BuildFont(Theme.MonospaceFont, Theme.CodeFontSize))
				.ColorAndOpacity(FSlateColor(Theme.CodeTextColor))
			];
	}

	case EMarkdownRenderNodeType::Link:
	{
		FString Url = Node->LinkUrl;
		FText DisplayText = GetInlineText(Node);
		if (DisplayText.IsEmpty())
		{
			DisplayText = FText::FromString(Url);
		}

		if (Theme.OnLinkClicked)
		{
			return SNew(SButton)
				.ButtonStyle(FCoreStyle::Get(), "NoBorder")
				.OnClicked(FOnClicked::CreateLambda([Url, Theme]() -> FReply
				{
					Theme.OnLinkClicked(Url);
					return FReply::Handled();
				}))
				.ContentPadding(FMargin(0))
				.Cursor(EMouseCursor::Hand)
				[
					SNew(STextBlock)
					.Text(DisplayText)
					.Font(BuildFont(Theme.DefaultFont, Theme.BodyFontSize))
					.ColorAndOpacity(Theme.LinkColor)
				];
		}

		return SNew(STextBlock)
			.Text(DisplayText)
			.Font(BuildFont(Theme.DefaultFont, Theme.BodyFontSize))
			.ColorAndOpacity(Theme.LinkColor);
	}

	case EMarkdownRenderNodeType::Image:
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.3f))
			.Padding(FMargin(8))
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("[Image: %s]"),
					Node->ImageUrl.IsEmpty() ? *GetInlineText(Node).ToString() : *Node->ImageUrl)))
				.Font(BuildFont(Theme.ItalicFont, Theme.BodyFontSize))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
			];

	default:
		return SNew(STextBlock)
			.Text(GetInlineText(Node))
			.Font(BuildFont(Theme.DefaultFont, Theme.BodyFontSize))
			.ColorAndOpacity(FSlateColor(Theme.BodyTextColor));
	}
}

static TSharedRef<SWidget> RenderInlines(const TSharedPtr<FMarkdownRenderNode>& Node, const FMarkdownSlateThemeConfig& Theme)
{
	if (!Node.IsValid() || Node->Children.Num() == 0)
	{
		return SNew(STextBlock).Text(Node.IsValid() ? Node->TextContent : FText::GetEmpty());
	}

	TSharedRef<SWrapBox> WrapBox = SNew(SWrapBox)
		.Orientation(Orient_Horizontal)
		.UseAllottedSize(true);

	for (const auto& Child : Node->Children)
	{
		WrapBox->AddSlot()
			.Padding(FMargin(0))
			.VAlign(VAlign_Center)
			[
				RenderInlineNode(Child, Theme)
			];
	}

	return WrapBox;
}

// ---- block rendering ----

static TSharedRef<SWidget> CreateCodeBlockWidget(const FMarkdownRenderNode& Node, const FMarkdownSlateThemeConfig& Theme)
{
	FString CodeText;
	for (const auto& Child : Node.Children)
	{
		if (Child->Type == EMarkdownRenderNodeType::PlainText)
		{
			CodeText += Child->TextContent.ToString();
		}
	}

	FRoundedBrushCache::Get().Update(Theme);

	return SNew(SBorder)
		.BorderImage(&FRoundedBrushCache::Get().CodeBlockBrush)
		.Padding(FMargin(Theme.CodeBlockPaddingH, Theme.CodeBlockPaddingV))
		[
			SNew(SScrollBox)
			.Orientation(Orient_Horizontal)
			+ SScrollBox::Slot()
			[
				SNew(STextBlock)
				.Text(FText::FromString(CodeText))
				.Font(BuildFont(Theme.MonospaceFont, Theme.CodeFontSize))
				.ColorAndOpacity(FSlateColor(Theme.CodeTextColor))
				.AutoWrapText(false)
			]
		];
}

static TSharedRef<SWidget> CreateListItemWidget(const FMarkdownRenderNode& Node, int32 Index, bool bOrdered, int32 StartIndex, const FMarkdownSlateThemeConfig& Theme)
{
	FString Prefix;
	if (bOrdered)
	{
		Prefix = FString::Printf(TEXT("%d. "), StartIndex + Index);
	}
	else
	{
		Prefix = TEXT("\u2022 ");
	}

	TSharedRef<SWrapBox> WrapBox = SNew(SWrapBox)
		.Orientation(Orient_Horizontal)
		.UseAllottedSize(true);

	// List marker
	WrapBox->AddSlot()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Prefix))
			.Font(BuildFont(Theme.DefaultFont, Theme.BodyFontSize))
			.ColorAndOpacity(FSlateColor(Theme.BodyTextColor))
		];

	// Inline content
	for (const auto& Child : Node.Children)
	{
		WrapBox->AddSlot()
			.VAlign(VAlign_Center)
			[
				RenderInlineNode(Child, Theme)
			];
	}

	return WrapBox;
}

TSharedRef<SWidget> FMarkdownSlateRenderer::RenderNode(const TSharedPtr<FMarkdownRenderNode>& Node, const FMarkdownSlateThemeConfig& Theme)
{
	if (!Node.IsValid())
	{
		return SNew(STextBlock).Text(FText::FromString(TEXT("")));
	}

	switch (Node->Type)
	{
	case EMarkdownRenderNodeType::Heading:
	{
		int32 FontSize = Theme.BodyFontSize;
		switch (Node->HeadingLevel)
		{
		case 1: FontSize = Theme.H1FontSize; break;
		case 2: FontSize = Theme.H2FontSize; break;
		case 3: FontSize = Theme.H3FontSize; break;
		case 4: FontSize = Theme.H4FontSize; break;
		case 5: FontSize = Theme.H5FontSize; break;
		default: FontSize = Theme.H6FontSize; break;
		}

		// If heading has inline children, render them with heading font
		if (Node->Children.Num() > 0)
		{
			TSharedRef<SWrapBox> WrapBox = SNew(SWrapBox)
				.Orientation(Orient_Horizontal)
				.UseAllottedSize(true);

			for (const auto& Child : Node->Children)
			{
				if (Child->Type == EMarkdownRenderNodeType::PlainText)
				{
					WrapBox->AddSlot().VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(Child->TextContent)
						.Font(BuildFont(Theme.BoldFont, FontSize))
						.ColorAndOpacity(FSlateColor(Theme.HeadingColor))
					];
				}
				else
				{
					WrapBox->AddSlot().VAlign(VAlign_Center)
					[
						RenderInlineNode(Child, Theme)
					];
				}
			}
			return WrapBox;
		}

		return SNew(STextBlock)
			.Text(Node->TextContent)
			.Font(BuildFont(Theme.BoldFont, FontSize))
			.ColorAndOpacity(FSlateColor(Theme.HeadingColor))
			.WrapTextAt(Theme.WrapTextWidth);
	}

	case EMarkdownRenderNodeType::Paragraph:
		if (Node->Children.Num() > 0)
		{
			return RenderInlines(Node, Theme);
		}
		return SNew(STextBlock)
			.Text(Node->TextContent)
			.Font(BuildFont(Theme.DefaultFont, Theme.BodyFontSize))
			.ColorAndOpacity(FSlateColor(Theme.BodyTextColor))
			.WrapTextAt(Theme.WrapTextWidth);

	case EMarkdownRenderNodeType::Blockquote:
	{
		FRoundedBrushCache::Get().Update(Theme);
		return SNew(SBorder)
			.BorderImage(&FRoundedBrushCache::Get().BlockquoteBrush)
			.Padding(FMargin(Theme.BlockquotePaddingH, Theme.BlockquotePaddingV,
			                 Theme.BlockquotePaddingH, Theme.BlockquotePaddingV))
			[
				Node->Children.Num() > 0
					? RenderInlines(Node, Theme)
					: static_cast<TSharedRef<SWidget>>(
						SNew(STextBlock)
						.Text(Node->TextContent)
						.Font(BuildFont(Theme.ItalicFont, Theme.BodyFontSize))
						.ColorAndOpacity(FSlateColor(Theme.BodyTextColor))
						.WrapTextAt(Theme.WrapTextWidth))
			];
	}

	case EMarkdownRenderNodeType::CodeBlock:
		return CreateCodeBlockWidget(*Node, Theme);

	case EMarkdownRenderNodeType::HorizontalRule:
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(Theme.HorizontalRuleColor)
			.Padding(FMargin(0, Theme.HorizontalRuleThickness * 0.5f));

	default:
		return SNew(STextBlock).Text(Node->TextContent);
	}
}

TSharedRef<SWidget> FMarkdownSlateRenderer::Render(const TSharedPtr<FMarkdownRenderNode>& RootNode, const FMarkdownSlateThemeConfig& Theme)
{
	if (!RootNode.IsValid())
	{
		return SNew(STextBlock).Text(FText::FromString(TEXT("")));
	}

	TSharedRef<SVerticalBox> VBox = SNew(SVerticalBox);

	for (const auto& Child : RootNode->Children)
	{
		switch (Child->Type)
		{
		case EMarkdownRenderNodeType::UnorderedList:
		case EMarkdownRenderNodeType::OrderedList:
		{
			bool bOrdered = (Child->Type == EMarkdownRenderNodeType::OrderedList);
			int32 StartIdx = Child->OrderedListStart;
			for (int32 i = 0; i < Child->Children.Num(); ++i)
			{
				VBox->AddSlot()
					.AutoHeight()
					.Padding(0, 1)
					[
						CreateListItemWidget(*Child->Children[i], i, bOrdered, StartIdx, Theme)
					];
			}
			break;
		}
		case EMarkdownRenderNodeType::CodeBlock:
			VBox->AddSlot()
				.AutoHeight()
				.Padding(0, 4)
				[
					CreateCodeBlockWidget(*Child, Theme)
				];
			break;
		default:
			VBox->AddSlot()
				.AutoHeight()
				.Padding(0, Theme.ParagraphSpacing * 0.25f)
				[
					RenderNode(Child, Theme)
				];
			break;
		}
	}

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(8)
		[
			VBox
		];
}
