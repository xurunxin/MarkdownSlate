#include "Widgets/MarkdownWidget.h"
#include "Delegates/Delegate.h"
#include "Styling/CoreStyle.h"

UMarkdownWidget::UMarkdownWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultFont = FCoreStyle::GetDefaultFontStyle("Regular", BodyFontSize);
}

static bool IsFontEmpty(const FSlateFontInfo& F)
{
	if (F.FontObject != nullptr) return false;
	const TSharedPtr<const FCompositeFont>& CF = F.CompositeFont;
	return !CF.IsValid() || CF->DefaultTypeface.Fonts.Num() == 0;
}

static void CopyBaseTheme(FMarkdownSlateThemeConfig& C, const UMarkdownThemeAsset& T)
{
	C.BodyFontSize = T.BodyFontSize;
	C.HeadingColor = T.HeadingColor; C.BodyTextColor = T.BodyTextColor;
	C.StrongColor = T.StrongColor; C.EmphasisColor = T.EmphasisColor;
	C.CodeBackgroundColor = T.CodeBackgroundColor; C.CodeTextColor = T.CodeTextColor;
	C.LinkColor = T.LinkColor;
	C.BlockquoteBackgroundColor = T.BlockquoteBackgroundColor;
	C.ParagraphSpacing = T.ParagraphSpacing; C.WrapTextWidth = T.WrapTextWidth;
	C.CodeCornerRadius = T.CodeCornerRadius;
	C.CodePaddingH = T.CodePaddingH; C.CodePaddingV = T.CodePaddingV;
	C.CodeBlockCornerRadius = T.CodeBlockCornerRadius;
	C.CodeBlockPaddingH = T.CodeBlockPaddingH; C.CodeBlockPaddingV = T.CodeBlockPaddingV;
	C.BlockquoteCornerRadius = T.BlockquoteCornerRadius;
	C.BlockquoteBorderWidth = T.BlockquoteBorderWidth;
	C.BlockquoteBorderColor = T.BlockquoteBorderColor;
	C.BlockquotePaddingH = T.BlockquotePaddingH; C.BlockquotePaddingV = T.BlockquotePaddingV;
	C.HorizontalRuleThickness = T.HorizontalRuleThickness;
	C.HorizontalRuleColor = T.HorizontalRuleColor;
	C.bEnableEmojiRendering = T.bEnableEmojiRendering;
	C.EmojiRenderMode = T.EmojiRenderMode; C.EmojiSizeScale = T.EmojiSizeScale;
	C.ListMarkerColor = T.ListMarkerColor; C.ListTextColor = T.ListTextColor;
	C.ListItemIndent = T.ListItemIndent;
	C.TableHeaderBgColor = T.TableHeaderBgColor;
	C.TableRowEvenBgColor = T.TableRowEvenBgColor; C.TableRowOddBgColor = T.TableRowOddBgColor;
	C.TableBorderColor = T.TableBorderColor;
	C.TableCellPaddingH = T.TableCellPaddingH; C.TableCellPaddingV = T.TableCellPaddingV;
	C.TableBorderThickness = T.TableBorderThickness;
}

FMarkdownSlateThemeConfig UMarkdownWidget::BuildThemeConfig() const
{
	FMarkdownSlateThemeConfig Config;
	if (ThemePreset) { CopyBaseTheme(Config, *ThemePreset); Config.DefaultFont = DefaultFont; }
	else
	{
		Config.DefaultFont = DefaultFont;
		Config.BodyFontSize = BodyFontSize;
		Config.HeadingColor = HeadingColor; Config.BodyTextColor = BodyTextColor;
		Config.StrongColor = StrongColor; Config.EmphasisColor = EmphasisColor;
		Config.CodeBackgroundColor = CodeBackgroundColor; Config.CodeTextColor = CodeTextColor;
		Config.LinkColor = LinkColor;
		Config.BlockquoteBackgroundColor = BlockquoteBackgroundColor;
		Config.ParagraphSpacing = ParagraphSpacing; Config.WrapTextWidth = WrapTextWidth;
		Config.CodeCornerRadius = CodeCornerRadius;
		Config.CodePaddingH = CodePaddingH; Config.CodePaddingV = CodePaddingV;
		Config.CodeBlockCornerRadius = CodeBlockCornerRadius;
		Config.CodeBlockPaddingH = CodeBlockPaddingH; Config.CodeBlockPaddingV = CodeBlockPaddingV;
		Config.BlockquoteCornerRadius = BlockquoteCornerRadius;
		Config.BlockquoteBorderWidth = BlockquoteBorderWidth;
		Config.BlockquoteBorderColor = BlockquoteBorderColor;
		Config.BlockquotePaddingH = BlockquotePaddingH; Config.BlockquotePaddingV = BlockquotePaddingV;
		Config.HorizontalRuleThickness = HorizontalRuleThickness;
		Config.HorizontalRuleColor = HorizontalRuleColor;
		Config.bEnableEmojiRendering = bEnableEmojiRendering;
		Config.EmojiRenderMode = EmojiRenderMode; Config.EmojiSizeScale = EmojiSizeScale;
		Config.ListMarkerColor = ListMarkerColor; Config.ListTextColor = ListTextColor;
		Config.ListItemIndent = ListItemIndent;
		Config.TableHeaderBgColor = TableHeaderBgColor;
		Config.TableRowEvenBgColor = TableRowEvenBgColor; Config.TableRowOddBgColor = TableRowOddBgColor;
		Config.TableBorderColor = TableBorderColor;
		Config.TableCellPaddingH = TableCellPaddingH; Config.TableCellPaddingV = TableCellPaddingV;
		Config.TableBorderThickness = TableBorderThickness;
	}

	// Ensure DefaultFont is valid
	if (IsFontEmpty(Config.DefaultFont))
		Config.DefaultFont = FCoreStyle::GetDefaultFontStyle("Regular", Config.BodyFontSize);
	Config.DefaultFont.Size = Config.BodyFontSize;

	return Config;
}

TSharedRef<SWidget> UMarkdownWidget::RebuildWidget()
{
	auto Config = BuildThemeConfig();
	Config.OnLinkClicked = [this](const FString& Url) { OnNativeLinkClicked(Url); };
	MySlateWidget = SNew(SMarkdownView)
		.MarkdownText(MarkdownText)
		.OnLinkClicked(FOnMarkdownViewLinkClickedSlate::CreateUObject(this, &UMarkdownWidget::OnNativeLinkClicked));
	MySlateWidget->SetThemeConfig(Config);
	return MySlateWidget.ToSharedRef();
}

void UMarkdownWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	if (MySlateWidget.IsValid())
	{
		auto Config = BuildThemeConfig();
		Config.OnLinkClicked = [this](const FString& Url) { OnNativeLinkClicked(Url); };
		MySlateWidget->SetThemeConfig(Config);
		MySlateWidget->SetMarkdownText(MarkdownText);
	}
}

void UMarkdownWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MySlateWidget.Reset();
}

void UMarkdownWidget::SetMarkdownText(const FString& InText)
{
	MarkdownText = InText;
	if (MySlateWidget.IsValid()) MySlateWidget->SetMarkdownText(InText);
}

void UMarkdownWidget::RefreshDisplayMarkdown()
{
	if (MySlateWidget.IsValid())
	{
		auto Config = BuildThemeConfig();
		Config.OnLinkClicked = [this](const FString& Url) { OnNativeLinkClicked(Url); };
		MySlateWidget->SetThemeConfig(Config);
		MySlateWidget->RefreshDisplay();
	}
}

void UMarkdownWidget::OnNativeLinkClicked(const FString& Url)
{
	OnLinkClicked.Broadcast(Url);
}

#if WITH_EDITOR
const FText UMarkdownWidget::GetPaletteCategory()
{
	return NSLOCTEXT("MarkdownSlate", "PaletteCategory", "Markdown");
}
#endif
