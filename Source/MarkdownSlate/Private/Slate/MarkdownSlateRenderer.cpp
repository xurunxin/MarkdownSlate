#include "Slate/MarkdownSlateRenderer.h"
#include "Table/SMarkdownTable.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Images/SImage.h"
#include "Styling/CoreStyle.h"
#include "Emoji/MarkdownEmojiScanner.h"
#include "Emoji/SMarkdownEmojiRun.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "UObject/UObjectGlobals.h"

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
	Config.EmojiFont = MakeDefaultEmojiFont(Config.BodyFontSize);
	return Config;
}

FSlateFontInfo FMarkdownSlateThemeConfig::MakeDefaultEmojiFont(int32 Size)
{
	UObject* EmojiFontObject = StaticLoadObject(
		UObject::StaticClass(),
		nullptr,
		TEXT("/MarkdownSlate/Fonts/NotoColorEmoji-Regular_Font.NotoColorEmoji-Regular_Font"));
	if (!EmojiFontObject)
	{
		EmojiFontObject = StaticLoadObject(
			UObject::StaticClass(),
			nullptr,
			TEXT("/MarkdownSlate/Fonts/NotoColorEmoji-Regular.NotoColorEmoji-Regular"));
	}

	FSlateFontInfo Font;
	Font.FontObject = EmojiFontObject;
	Font.Size = Size;
	return Font;
}

uint32 FMarkdownSlateThemeConfig::ComputeHash() const
{
	uint32 Hash = 0;
	Hash = HashCombine(Hash, GetTypeHash(BodyFontSize));
	Hash = HashCombine(Hash, GetTypeHash(WrapTextWidth));
	Hash = HashCombine(Hash, GetTypeHash(ParagraphSpacing));
	Hash = HashCombine(Hash, GetTypeHash(EmojiFont.FontObject));
	Hash = HashCombine(Hash, GetTypeHash(HeadingColor.R) ^ GetTypeHash(HeadingColor.G) ^ GetTypeHash(HeadingColor.B));
	Hash = HashCombine(Hash, GetTypeHash(BodyTextColor.R) ^ GetTypeHash(BodyTextColor.G) ^ GetTypeHash(BodyTextColor.B));
	Hash = HashCombine(Hash, GetTypeHash(LinkColor.R) ^ GetTypeHash(LinkColor.G) ^ GetTypeHash(LinkColor.B));
	return Hash;
}

static FSlateFontInfo BuildFont(const FSlateFontInfo& BaseFont, int32 Size)
{
	FSlateFontInfo Font = BaseFont;
	Font.Size = Size;
	return Font;
}

struct FInlineRenderStyle
{
	FSlateFontInfo Font;
	int32 FontSize = 12;
	FLinearColor Color = FLinearColor::White;
	bool bBold = false;
	bool bItalic = false;
	bool bFauxBold = false; // only for Strong inline; headings use native font weight
	FSlateFontInfo BoldFontOverride;
	FSlateFontInfo ItalicFontOverride;
};

static FSlateFontInfo BuildStyledFont(const FInlineRenderStyle& Style)
{
	// Use explicit BoldFont/ItalicFont if provided
	auto FontHasData = [](const FSlateFontInfo& F) {
		return F.FontObject != nullptr ||
			(F.CompositeFont.IsValid() && F.CompositeFont->DefaultTypeface.Fonts.Num() > 0);
	};
	if (Style.bBold && FontHasData(Style.BoldFontOverride))
	{
		FSlateFontInfo Font = Style.BoldFontOverride;
		Font.Size = Style.FontSize;
		return Font;
	}
	if (Style.bItalic && FontHasData(Style.ItalicFontOverride))
	{
		FSlateFontInfo Font = Style.ItalicFontOverride;
		Font.Size = Style.FontSize;
		return Font;
	}

	FSlateFontInfo Font = Style.Font;
	Font.Size = Style.FontSize;

	if (Style.bFauxBold && Font.OutlineSettings.OutlineSize <= 0.0f)
	{
		Font.OutlineSettings.OutlineSize = 0.35f;
		Font.OutlineSettings.OutlineColor = Style.Color;
		Font.OutlineSettings.bSeparateFillAlpha = true;
	}
	if (Style.bItalic)
	{
		Font.SkewAmount = 0.28f;
	}
	return Font;
}

static FInlineRenderStyle MakeBodyStyle(const FMarkdownSlateThemeConfig& Theme)
{
	FInlineRenderStyle Style;
	Style.Font = Theme.DefaultFont;
	Style.FontSize = Theme.BodyFontSize;
	Style.Color = Theme.BodyTextColor;
	return Style;
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
		else
		{
			Combined += GetInlineText(Child).ToString();
		}
	}
	return FText::FromString(Combined);
}

static TSharedRef<SWidget> RenderInlineNode(const TSharedPtr<FMarkdownRenderNode>& Node, const FMarkdownSlateThemeConfig& Theme, const FInlineRenderStyle& Style);

static TSharedRef<SWidget> RenderInlineChildren(const TSharedPtr<FMarkdownRenderNode>& Node, const FMarkdownSlateThemeConfig& Theme, const FInlineRenderStyle& Style)
{
	if (!Node.IsValid() || Node->Children.Num() == 0)
	{
		return SNew(STextBlock)
			.Text(Node.IsValid() ? Node->TextContent : FText::GetEmpty())
			.Font(BuildStyledFont(Style))
			.ColorAndOpacity(FSlateColor(Style.Color));
	}

	TSharedRef<SHorizontalBox> InlineBox = SNew(SHorizontalBox);

	for (const auto& Child : Node->Children)
	{
		InlineBox->AddSlot()
			.AutoWidth()
			.Padding(FMargin(0))
			.VAlign(VAlign_Center)
			[
				RenderInlineNode(Child, Theme, Style)
			];
	}

	return InlineBox;
}

static TSharedRef<SWidget> RenderTextWithEmoji(const FString& Text, const FMarkdownSlateThemeConfig& Theme, const FInlineRenderStyle& Style)
{
	if (Theme.bEnableEmojiRendering)
	{
		FMarkdownEmojiConfig EmojiCfg;
		EmojiCfg.bEnableEmojiRendering = Theme.bEnableEmojiRendering;
		EmojiCfg.RenderMode = Theme.EmojiRenderMode;
		EmojiCfg.TwemojiAssetRoot = Theme.TwemojiAssetRoot;
		EmojiCfg.EmojiSizeScale = Theme.EmojiSizeScale;
		EmojiCfg.bAllowTwemojiFallback = Theme.bAllowTwemojiFallback;

		FMarkdownEmojiScanner Scanner(EmojiCfg);
		TArray<FMarkdownEmojiRun> Runs = Scanner.ScanText(Text);
		const bool bContainsEmoji = Runs.ContainsByPredicate([](const FMarkdownEmojiRun& Run)
		{
			return Run.bIsEmoji;
		});
		if (bContainsEmoji)
		{
			TSharedRef<SWrapBox> RunBox = SNew(SWrapBox)
				.UseAllottedSize(true);
			for (const auto& Run : Runs)
			{
				TSharedRef<SWidget> RunWidget = SNullWidget::NullWidget;
				if (Run.bIsEmoji)
				{
					RunWidget = SNew(SMarkdownEmojiRun)
						.Run(Run)
						.FontInfo(BuildStyledFont(Style))
						.EmojiFontInfo(Theme.EmojiFont)
						.FontSize(Style.FontSize)
						.TextColor(Style.Color)
						.EmojiProvider(Theme.EmojiProvider)
						.Config(EmojiCfg);
				}
				else
				{
					TSharedRef<STextBlock> PlainText = SNew(STextBlock)
						.Text(FText::FromString(Run.EmojiSequence))
						.Font(BuildStyledFont(Style))
						.ColorAndOpacity(FSlateColor(Style.Color));
					RunWidget = PlainText;
				}

				RunBox->AddSlot()
					.FillEmptySpace(false)
					.VAlign(VAlign_Center)
					[
						RunWidget
					];
			}
			return RunBox;
		}
	}

	TSharedRef<STextBlock> TextBlock = SNew(STextBlock)
		.Text(FText::FromString(Text))
		.Font(BuildStyledFont(Style))
		.ColorAndOpacity(FSlateColor(Style.Color));
	if (Theme.WrapTextWidth > 0.0f)
	{
		TextBlock->SetAutoWrapText(true);
		TextBlock->SetWrapTextAt(Theme.WrapTextWidth);
	}
	return TextBlock;
}

static TSharedRef<SWidget> RenderInlineNode(const TSharedPtr<FMarkdownRenderNode>& Node, const FMarkdownSlateThemeConfig& Theme, const FInlineRenderStyle& Style)
{
	if (!Node.IsValid()) return SNullWidget::NullWidget;

	switch (Node->Type)
	{
	case EMarkdownRenderNodeType::PlainText:
	{
		return RenderTextWithEmoji(Node->TextContent.ToString(), Theme, Style);
	}

	case EMarkdownRenderNodeType::Strong:
	{
		FInlineRenderStyle StrongStyle = Style;
		StrongStyle.bBold = true;
		StrongStyle.bFauxBold = true;
		StrongStyle.BoldFontOverride = Theme.BoldFont;
		StrongStyle.Color = Style.Color; // inherit parent color
		return RenderInlineChildren(Node, Theme, StrongStyle);
	}

	case EMarkdownRenderNodeType::Emphasis:
	{
		FInlineRenderStyle EmphasisStyle = Style;
		EmphasisStyle.bItalic = true;
		EmphasisStyle.ItalicFontOverride = Theme.ItalicFont;
		EmphasisStyle.Color = Style.Color; // inherit parent color
		return RenderInlineChildren(Node, Theme, EmphasisStyle);
	}

	case EMarkdownRenderNodeType::Strikethrough:
		{
			FInlineRenderStyle StrikeStyle = Style;
			StrikeStyle.Color = Style.Color;
			return SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Center).VAlign(VAlign_Center)
				[
					RenderInlineChildren(Node, Theme, StrikeStyle)
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill).VAlign(VAlign_Center)
				[
					SNew(SBox)
					.HeightOverride(1.0f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(Style.Color)
					]
				];
		}

	case EMarkdownRenderNodeType::CodeInline:
	{
		FRoundedBrushCache::Get().Update(Theme);
		FInlineRenderStyle CodeStyle = Style;
		CodeStyle.Font = Theme.DefaultFont;
		CodeStyle.FontSize = FMath::Max(8, (int32)(Theme.BodyFontSize * 0.92f));
		CodeStyle.Color = Theme.CodeTextColor;
		return SNew(SBorder)
			.BorderImage(&FRoundedBrushCache::Get().CodeBrush)
			.Padding(FMargin(Theme.CodePaddingH, Theme.CodePaddingV))
			[
				RenderTextWithEmoji(GetInlineText(Node).ToString(), Theme, CodeStyle)
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
		FInlineRenderStyle LinkStyle = Style;
		LinkStyle.Color = Theme.LinkColor;

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
					Node->Children.Num() > 0
						? RenderInlineChildren(Node, Theme, LinkStyle)
						: static_cast<TSharedRef<SWidget>>(SNew(STextBlock)
							.Text(DisplayText)
							.Font(BuildStyledFont(LinkStyle))
							.ColorAndOpacity(Theme.LinkColor))
				];
		}

		return Node->Children.Num() > 0
			? RenderInlineChildren(Node, Theme, LinkStyle)
			: static_cast<TSharedRef<SWidget>>(SNew(STextBlock)
				.Text(DisplayText)
				.Font(BuildStyledFont(LinkStyle))
				.ColorAndOpacity(Theme.LinkColor));
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
				.Font(BuildFont(Theme.DefaultFont, Theme.BodyFontSize))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
			];

	default:
		return RenderInlineChildren(Node, Theme, Style);
	}
}

TSharedRef<SWidget> FMarkdownSlateRenderer::RenderInlines(const TSharedPtr<FMarkdownRenderNode>& Node, const FMarkdownSlateThemeConfig& Theme)
{
	if (!Node.IsValid() || Node->Children.Num() == 0)
	{
		return SNew(STextBlock).Text(Node.IsValid() ? Node->TextContent : FText::GetEmpty());
	}

	TSharedRef<SWrapBox> InlineBox = SNew(SWrapBox)
		.UseAllottedSize(true);

	for (const auto& Child : Node->Children)
	{
		InlineBox->AddSlot()
			.FillEmptySpace(false)
			.Padding(FMargin(0))
			.VAlign(VAlign_Center)
			[
				RenderInlineNode(Child, Theme, MakeBodyStyle(Theme))
			];
	}

	return InlineBox;
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
				.Font(BuildFont(Theme.DefaultFont, FMath::Max(8, (int32)(Theme.BodyFontSize * 0.92f))))
				.ColorAndOpacity(FSlateColor(Theme.CodeTextColor))
				.AutoWrapText(false)
			]
		];
}

static TSharedRef<SWidget> CreateListItemWidget(const FMarkdownRenderNode& Node, int32 Index, bool bOrdered, int32 StartIndex, const FMarkdownSlateThemeConfig& Theme)
{
	FString Prefix;
	if (Node.bIsTaskItem)
	{
		Prefix.Empty();
	}
	else if (bOrdered)
	{
		Prefix = FString::Printf(TEXT("%d. "), StartIndex + Index);
	}
	else
	{
		Prefix = TEXT("\u2022 ");
	}

	TSharedRef<SWrapBox> InlineBox = SNew(SWrapBox)
		.UseAllottedSize(true);

	// Marker (bullet / checkbox / number)
	if (Node.bIsTaskItem)
	{
		InlineBox->AddSlot()
			.FillEmptySpace(false)
			.VAlign(VAlign_Center)
			.Padding(FMargin(0, 0, 4, 0))
			[
				SNew(SCheckBox)
				.IsChecked((Node.TaskMark == 'x' || Node.TaskMark == 'X') ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.Visibility(EVisibility::HitTestInvisible)
			];
	}
	else
	{
		InlineBox->AddSlot()
			.FillEmptySpace(false)
			.VAlign(VAlign_Center)
			.Padding(FMargin(0, 0, 2, 0))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Prefix))
				.Font(BuildFont(Theme.DefaultFont, Theme.BodyFontSize))
				.ColorAndOpacity(FSlateColor(Theme.ListMarkerColor))
			];
	}

	// Inline content — use ListTextColor for body text
	FMarkdownSlateThemeConfig ListTheme = Theme;
	ListTheme.BodyTextColor = Theme.ListTextColor;

	for (const auto& Child : Node.Children)
	{
		if (Child->Type == EMarkdownRenderNodeType::UnorderedList ||
			Child->Type == EMarkdownRenderNodeType::OrderedList ||
			Child->Type == EMarkdownRenderNodeType::CodeBlock ||
			Child->Type == EMarkdownRenderNodeType::Table)
		{
			continue;
		}
		InlineBox->AddSlot()
			.FillEmptySpace(false)
			.VAlign(VAlign_Center)
			[
				RenderInlineNode(Child, ListTheme, MakeBodyStyle(ListTheme))
			];
	}

	// Collect block-level children (nested lists, code blocks)
	TArray<TSharedPtr<FMarkdownRenderNode>> BlockChildren;
	for (const auto& Child : Node.Children)
	{
		if (Child->Type == EMarkdownRenderNodeType::UnorderedList ||
			Child->Type == EMarkdownRenderNodeType::OrderedList ||
			Child->Type == EMarkdownRenderNodeType::CodeBlock ||
			Child->Type == EMarkdownRenderNodeType::Table)
		{
			BlockChildren.Add(Child);
		}
	}

	if (BlockChildren.Num() > 0)
	{
		TSharedRef<SVerticalBox> SubListBox = SNew(SVerticalBox);
		for (const auto& BC : BlockChildren)
		{
			if (BC->Type == EMarkdownRenderNodeType::UnorderedList ||
				BC->Type == EMarkdownRenderNodeType::OrderedList)
			{
				bool bOL = (BC->Type == EMarkdownRenderNodeType::OrderedList);
				int32 StIdx = BC->OrderedListStart;
				for (int32 i = 0; i < BC->Children.Num(); ++i)
				{
					SubListBox->AddSlot().AutoHeight().Padding(0, 1)
					[
						CreateListItemWidget(*BC->Children[i], i, bOL, StIdx, ListTheme)
					];
				}
			}
			else
			{
				SubListBox->AddSlot().AutoHeight()
				[
					FMarkdownSlateRenderer::RenderNode(BC, ListTheme)
				];
			}
		}

		TSharedRef<SVerticalBox> Outer = SNew(SVerticalBox);
		Outer->AddSlot().AutoHeight()[InlineBox];
		Outer->AddSlot().AutoHeight().Padding(Theme.ListItemIndent, 0, 0, 0)[SubListBox];
		return Outer;
	}

	return InlineBox;
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
		case 1: FontSize = Theme.BodyFontSize * 2.0f; break;
		case 2: FontSize = Theme.BodyFontSize * 1.7f; break;
		case 3: FontSize = Theme.BodyFontSize * 1.5f; break;
		case 4: FontSize = Theme.BodyFontSize * 1.3f; break;
		case 5: FontSize = Theme.BodyFontSize * 1.15f; break;
		default: FontSize = Theme.BodyFontSize * 1.05f; break;
		}

		// If heading has inline children, render them with heading font
		if (Node->Children.Num() > 0)
		{
			FInlineRenderStyle HeadingStyle = MakeBodyStyle(Theme);
			HeadingStyle.FontSize = FontSize;
			HeadingStyle.Color = Theme.HeadingColor;
			HeadingStyle.bBold = true;
			return RenderInlineChildren(Node, Theme, HeadingStyle);
		}

		return SNew(STextBlock)
			.Text(Node->TextContent)
			.Font(BuildStyledFont(FInlineRenderStyle{Theme.DefaultFont, FontSize, Theme.HeadingColor, true, false}))
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
						.Font(BuildFont(Theme.DefaultFont, Theme.BodyFontSize))
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
	AppendChildren(VBox, RootNode, Theme);

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(8)
		[
			VBox
		];
}

void FMarkdownSlateRenderer::AppendChildren(const TSharedRef<SVerticalBox>& VBox, const TSharedPtr<FMarkdownRenderNode>& RootNode, const FMarkdownSlateThemeConfig& Theme)
{
	if (!RootNode.IsValid())
	{
		return;
	}

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
					.Padding(Theme.ListItemIndent, 1, 0, 1)
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
		case EMarkdownRenderNodeType::Table:
			VBox->AddSlot()
				.AutoHeight()
				.Padding(0, 4)
				[
					SNew(SMarkdownTable)
					.TableModel(Child->TableModel)
					.ThemeConfig(Theme)
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
}
