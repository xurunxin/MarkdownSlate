#include "Slate/SMarkdownView.h"
#include "Render/MarkdownRenderBuilder.h"
#include "Parser/MarkdownParser.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

void SMarkdownView::Construct(const FArguments& InArgs)
{
	MarkdownText = InArgs._MarkdownText;
	OnLinkClicked = InArgs._OnLinkClicked;
	ThemeConfig = FMarkdownSlateThemeConfig::Default();

	ChildSlot
	[
		SAssignNew(ContentBox, SVerticalBox)
	];

	RefreshDisplay();
}

void SMarkdownView::SetMarkdownText(const FString& InMarkdownText)
{
	MarkdownText.Set(InMarkdownText);
	RefreshDisplay();
}

void SMarkdownView::SetThemeConfig(const FMarkdownSlateThemeConfig& InTheme)
{
	ThemeConfig = InTheme;
	RenderCache.Invalidate();
	RefreshDisplay();
}

void SMarkdownView::InvalidateCache()
{
	RenderCache.Invalidate();
}

void SMarkdownView::RefreshDisplay()
{
	if (!ContentBox.IsValid())
	{
		return;
	}

	ContentBox->ClearChildren();

	FString CurrentText = MarkdownText.Get();

	if (CurrentText.IsEmpty())
	{
		ContentBox->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("")))
			];
		return;
	}

	// Build cache key
	FMarkdownRenderCacheKey CacheKey;
	CacheKey.SourceHash = GetTypeHash(CurrentText);
	CacheKey.ParseFlags = 0; // MD_DIALECT_GITHUB default
	CacheKey.ThemeHash = ThemeConfig.ComputeHash();
	CacheKey.WidthBucket = FMarkdownRenderCacheKey::MakeWidthBucket(ThemeConfig.WrapTextWidth);
	CacheKey.FeatureMask = 0;
	CacheKey.SchemaVersion = 1;

	// Try cache
	TSharedPtr<FMarkdownRenderNode> CachedRoot = RenderCache.Get(CacheKey);

	if (!CachedRoot.IsValid())
	{
		FMarkdownParser Parser;
		TSharedPtr<FMarkdownBlockNode> AstRoot = Parser.Parse(CurrentText);
		FMarkdownRenderBuilder Builder;
		CachedRoot = Builder.Build(AstRoot);
		RenderCache.Put(CacheKey, CachedRoot);
	}

	ContentBox->AddSlot()
		[
			FMarkdownSlateRenderer::Render(CachedRoot, ThemeConfig)
		];
}
