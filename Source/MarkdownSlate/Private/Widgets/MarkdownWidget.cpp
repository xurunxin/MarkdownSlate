#include "Widgets/MarkdownWidget.h"
#include "Delegates/Delegate.h"
#include "Styling/CoreStyle.h"
#include "UObject/UObjectIterator.h"

UMarkdownWidget::UMarkdownWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

static bool IsFontEmpty(const FSlateFontInfo& F)
{
	if (F.FontObject != nullptr) return false;
	const TSharedPtr<const FCompositeFont>& CF = F.CompositeFont;
	return !CF.IsValid() || CF->DefaultTypeface.Fonts.Num() == 0;
}

static void CopyThemeToConfig(FMarkdownSlateThemeConfig& C, const UMarkdownThemeAsset& T)
{
	C.DefaultFont = T.DefaultFont;
	C.BoldFont = T.BoldFont;
	C.ItalicFont = T.ItalicFont;
	C.BodyFontSize = T.BodyFontSize;

	C.HeadingColor = T.HeadingColor;
	C.BodyTextColor = T.BodyTextColor;

	C.CodeBackgroundColor = T.CodeBackgroundColor;
	C.CodeTextColor = T.CodeTextColor;
	C.CodeCornerRadius = T.CodeCornerRadius;
	C.CodePaddingH = T.CodePaddingH;
	C.CodePaddingV = T.CodePaddingV;
	C.CodeBlockCornerRadius = T.CodeBlockCornerRadius;
	C.CodeBlockPaddingH = T.CodeBlockPaddingH;
	C.CodeBlockPaddingV = T.CodeBlockPaddingV;

	C.LinkColor = T.LinkColor;

	C.BlockquoteBackgroundColor = T.BlockquoteBackgroundColor;
	C.BlockquoteCornerRadius = T.BlockquoteCornerRadius;
	C.BlockquoteBorderWidth = T.BlockquoteBorderWidth;
	C.BlockquoteBorderColor = T.BlockquoteBorderColor;
	C.BlockquotePaddingH = T.BlockquotePaddingH;
	C.BlockquotePaddingV = T.BlockquotePaddingV;

	C.HorizontalRuleThickness = T.HorizontalRuleThickness;
	C.HorizontalRuleColor = T.HorizontalRuleColor;

	C.ListMarkerColor = T.ListMarkerColor;
	C.ListTextColor = T.ListTextColor;
	C.ListItemIndent = T.ListItemIndent;

	C.TableHeaderBgColor = T.TableHeaderBgColor;
	C.TableRowEvenBgColor = T.TableRowEvenBgColor;
	C.TableRowOddBgColor = T.TableRowOddBgColor;
	C.TableBorderColor = T.TableBorderColor;
	C.TableCellPaddingH = T.TableCellPaddingH;
	C.TableCellPaddingV = T.TableCellPaddingV;
	C.TableBorderThickness = T.TableBorderThickness;

	C.bEnableEmojiRendering = T.bEnableEmojiRendering;
	C.EmojiRenderMode = T.EmojiRenderMode;
	C.EmojiSizeScale = T.EmojiSizeScale;
	C.TwemojiAssetRoot = T.TwemojiAssetRoot;
	C.bAllowTwemojiFallback = T.bAllowTwemojiFallback;

	C.ParagraphSpacing = T.ParagraphSpacing;
	C.WrapTextWidth = T.WrapTextWidth;
}

FMarkdownSlateThemeConfig UMarkdownWidget::BuildThemeConfig() const
{
	FMarkdownSlateThemeConfig Config;

	if (Theme)
		CopyThemeToConfig(Config, *Theme);
	else
		Config = FMarkdownSlateThemeConfig::Default();

	if (IsFontEmpty(Config.DefaultFont))
		Config.DefaultFont = FCoreStyle::GetDefaultFontStyle("Regular", Config.BodyFontSize);
	Config.DefaultFont.Size = Config.BodyFontSize;

	return Config;
}

#if WITH_EDITOR
static void OnThemePropertyChanged(UObject* Object, FPropertyChangedEvent& Event)
{
	// ReSharper disable once CppTooWideScopeInitStatement
	UMarkdownThemeAsset* ThemeAsset = Cast<UMarkdownThemeAsset>(Object);
	if (!ThemeAsset) return;

	// Notify all widgets referencing this theme
	for (TObjectIterator<UMarkdownWidget> It; It; ++It)
	{
		if (It->Theme == ThemeAsset)
		{
			It->SynchronizeProperties();
		}
	}
}
#endif

TSharedRef<SWidget> UMarkdownWidget::RebuildWidget()
{
	auto Config = BuildThemeConfig();
	Config.OnLinkClicked = [this](const FString& Url) { OnNativeLinkClicked(Url); };
	MySlateWidget = SNew(SMarkdownView)
		.MarkdownText(MarkdownText)
		.OnLinkClicked(FOnMarkdownViewLinkClickedSlate::CreateUObject(this, &UMarkdownWidget::OnNativeLinkClicked));
	MySlateWidget->SetThemeConfig(Config);

#if WITH_EDITOR
	FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddStatic(&OnThemePropertyChanged);
#endif

	return MySlateWidget.ToSharedRef();
}

void UMarkdownWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	bool bThemeChanged = false;
	if (Theme && Theme->Generation != LastThemeGeneration)
	{
		LastThemeGeneration = Theme->Generation;
		bThemeChanged = true;
	}
	else if (!Theme)
	{
		LastThemeGeneration = -1;
	}

	if (MySlateWidget.IsValid())
	{
		if (bThemeChanged || LastThemeGeneration == -1)
		{
			auto Config = BuildThemeConfig();
			Config.OnLinkClicked = [this](const FString& Url) { OnNativeLinkClicked(Url); };
			MySlateWidget->SetThemeConfig(Config);
		}
		MySlateWidget->SetMarkdownText(MarkdownText);
	}
}

void UMarkdownWidget::ReleaseSlateResources(bool bReleaseChildren)
{
#if WITH_EDITOR
	FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
#endif
	Super::ReleaseSlateResources(bReleaseChildren);
	MySlateWidget.Reset();
}

void UMarkdownWidget::SetMarkdownText(const FString& InText)
{
	MarkdownText = InText;
	if (MySlateWidget.IsValid()) MySlateWidget->SetMarkdownText(InText);
}

void UMarkdownWidget::BeginStreamingMarkdown()
{
	MarkdownText.Reset();
	if (MySlateWidget.IsValid()) MySlateWidget->BeginStreamingMarkdown();
}

void UMarkdownWidget::AppendMarkdownChunk(const FString& Chunk)
{
	if (Chunk.IsEmpty())
	{
		return;
	}

	MarkdownText += Chunk;
	if (MySlateWidget.IsValid()) MySlateWidget->AppendMarkdownChunk(Chunk);
}

void UMarkdownWidget::EndStreamingMarkdown()
{
	if (MySlateWidget.IsValid()) MySlateWidget->EndStreamingMarkdown();
}

void UMarkdownWidget::RefreshDisplayMarkdown()
{
	if (MySlateWidget.IsValid())
	{
		auto Config = BuildThemeConfig();
		Config.OnLinkClicked = [this](const FString& Url) { OnNativeLinkClicked(Url); };
		MySlateWidget->SetThemeConfig(Config);
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
