#include "Slate/SMarkdownView.h"
#include "Emoji/MarkdownEmojiAtlas.h"
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
	bIsStreamingMarkdown = false;
	StreamingBuffer.Reset();
	StreamingFullText = InMarkdownText;
	RenderedStableTextLen = 0;
	MarkdownText.Set(InMarkdownText);
	RefreshDisplay();
}

void SMarkdownView::SetThemeConfig(const FMarkdownSlateThemeConfig& InTheme)
{
	ThemeConfig = InTheme;
	if (ThemeConfig.bEnableEmojiRendering)
	{
		if (!EmojiProvider.IsValid())
		{
			EmojiProvider = MakeShared<FMarkdownAtlasEmojiProvider>();
		}
		if (!EmojiProvider->SupportsAtlasRendering())
		{
			EmojiProvider->GetAtlasPtr()->AutoLoadAtlas(ThemeConfig.TwemojiAssetRoot);
		}
		if (EmojiProvider->SupportsAtlasRendering())
		{
			ThemeConfig.EmojiProvider = EmojiProvider.Get();
		}
	}
	else
	{
		ThemeConfig.EmojiProvider = nullptr;
	}
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

	bIsStreamingMarkdown = false;
	StreamingContentBox.Reset();
	PendingContentBox.Reset();
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

	ContentBox->AddSlot()
		[
			RenderMarkdownText(CurrentText)
		];
}

TSharedPtr<FMarkdownRenderNode> SMarkdownView::BuildRenderRoot(const FString& SourceText)
{
	if (SourceText.IsEmpty())
	{
		return TSharedPtr<FMarkdownRenderNode>();
	}

	FMarkdownRenderCacheKey CacheKey;
	CacheKey.SourceHash = GetTypeHash(SourceText);
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
		TSharedPtr<FMarkdownBlockNode> AstRoot = Parser.Parse(SourceText);
		FMarkdownRenderBuilder Builder;
		CachedRoot = Builder.Build(AstRoot);
		RenderCache.Put(CacheKey, CachedRoot);
	}

	return CachedRoot;
}

TSharedRef<SWidget> SMarkdownView::RenderMarkdownText(const FString& SourceText)
{
	TSharedPtr<FMarkdownRenderNode> Root = BuildRenderRoot(SourceText);
	return Root.IsValid()
		? FMarkdownSlateRenderer::Render(Root, ThemeConfig)
		: static_cast<TSharedRef<SWidget>>(SNew(STextBlock).Text(FText::FromString(TEXT(""))));
}

void SMarkdownView::BeginStreamingMarkdown()
{
	bIsStreamingMarkdown = true;
	StreamingBuffer.Reset();
	StreamingFullText.Reset();
	RenderedStableTextLen = 0;
	StreamingContentBox.Reset();
	PendingContentBox.Reset();
	MarkdownText.Set(FString());

	if (!ContentBox.IsValid())
	{
		return;
	}

	ContentBox->ClearChildren();
	ContentBox->AddSlot()
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.Padding(8)
			[
				SAssignNew(StreamingContentBox, SVerticalBox)
			]
		];
}

void SMarkdownView::AppendMarkdownChunk(const FString& Chunk)
{
	if (Chunk.IsEmpty())
	{
		return;
	}

	if (!bIsStreamingMarkdown)
	{
		BeginStreamingMarkdown();
	}

	StreamingFullText += Chunk;
	MarkdownText.Set(StreamingFullText);
	StreamingBuffer.Append(Chunk);

	AppendStableStreamingText(StreamingBuffer.GetStableText());
	UpdatePendingStreamingText(StreamingBuffer.GetPendingText());
}

void SMarkdownView::EndStreamingMarkdown()
{
	if (!bIsStreamingMarkdown)
	{
		return;
	}

	const FString PendingText = StreamingBuffer.GetPendingText();
	if (!PendingText.IsEmpty())
	{
		AppendStableStreamingText(StreamingBuffer.GetStableText() + PendingText);
		UpdatePendingStreamingText(FString());
	}

	bIsStreamingMarkdown = false;
	StreamingBuffer.Reset();
	RenderedStableTextLen = StreamingFullText.Len();
	MarkdownText.Set(StreamingFullText);
}

void SMarkdownView::AppendStableStreamingText(const FString& StableText)
{
	if (!StreamingContentBox.IsValid() || StableText.Len() <= RenderedStableTextLen)
	{
		return;
	}

	if (PendingContentBox.IsValid())
	{
		PendingContentBox->SetContent(SNullWidget::NullWidget);
	}

	const FString NewStableText = StableText.Mid(RenderedStableTextLen);
	TSharedPtr<FMarkdownRenderNode> Root = BuildRenderRoot(NewStableText);
	if (Root.IsValid())
	{
		FMarkdownSlateRenderer::AppendChildren(StreamingContentBox.ToSharedRef(), Root, ThemeConfig);
	}
	RenderedStableTextLen = StableText.Len();
}

void SMarkdownView::UpdatePendingStreamingText(const FString& PendingText)
{
	if (!StreamingContentBox.IsValid())
	{
		return;
	}

	if (PendingText.IsEmpty())
	{
		if (PendingContentBox.IsValid())
		{
			PendingContentBox->SetContent(SNullWidget::NullWidget);
		}
		return;
	}

	if (!PendingContentBox.IsValid())
	{
		StreamingContentBox->AddSlot()
			.AutoHeight()
			.Padding(0, ThemeConfig.ParagraphSpacing * 0.25f)
			[
				SAssignNew(PendingContentBox, SBox)
			];
	}

	TSharedRef<SVerticalBox> PendingVBox = SNew(SVerticalBox);
	if (TSharedPtr<FMarkdownRenderNode> PendingRoot = BuildRenderRoot(PendingText))
	{
		FMarkdownSlateRenderer::AppendChildren(PendingVBox, PendingRoot, ThemeConfig);
	}
	PendingContentBox->SetContent(PendingVBox);
}
