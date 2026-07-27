#include "Slate/SMarkdownView.h"
#include "Emoji/MarkdownEmojiAtlas.h"
#include "Render/MarkdownRenderBuilder.h"
#include "Parser/MarkdownParser.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
bool IsEscapedMarkdownChar(const FString& Text, int32 Index)
{
	int32 SlashCount = 0;
	for (int32 i = Index - 1; i >= 0 && Text[i] == TEXT('\\'); --i)
	{
		++SlashCount;
	}
	return (SlashCount % 2) != 0;
}

int32 CountUnescapedToken(const FString& Text, const TCHAR* Token)
{
	const FString TokenText(Token);
	int32 Count = 0;
	for (int32 Index = 0; Index != INDEX_NONE && Index < Text.Len();)
	{
		Index = Text.Find(TokenText, ESearchCase::CaseSensitive, ESearchDir::FromStart, Index);
		if (Index == INDEX_NONE)
		{
			break;
		}
		if (!IsEscapedMarkdownChar(Text, Index))
		{
			++Count;
		}
		Index += TokenText.Len();
	}
	return Count;
}

bool HasUnclosedLinkOrImage(const FString& Text)
{
	int32 OpenBracketCount = 0;
	bool bInLinkDestination = false;

	for (int32 Index = 0; Index < Text.Len(); ++Index)
	{
		const TCHAR Ch = Text[Index];
		if (IsEscapedMarkdownChar(Text, Index))
		{
			continue;
		}

		if (bInLinkDestination)
		{
			if (Ch == TEXT(')'))
			{
				bInLinkDestination = false;
			}
			continue;
		}

		if (Ch == TEXT('['))
		{
			++OpenBracketCount;
		}
		else if (Ch == TEXT(']') && OpenBracketCount > 0)
		{
			--OpenBracketCount;
			if (Index + 1 < Text.Len() && Text[Index + 1] == TEXT('('))
			{
				bInLinkDestination = true;
				++Index;
			}
		}
	}

	return OpenBracketCount > 0 || bInLinkDestination;
}

bool IsMarkdownListLine(const FString& Line)
{
	int32 Index = 0;
	while (Index < Line.Len() && FChar::IsWhitespace(Line[Index]))
	{
		++Index;
	}

	if (Index + 1 < Line.Len() &&
		(Line[Index] == TEXT('-') || Line[Index] == TEXT('+') || Line[Index] == TEXT('*')) &&
		FChar::IsWhitespace(Line[Index + 1]))
	{
		return true;
	}

	int32 DigitIndex = Index;
	while (DigitIndex < Line.Len() && FChar::IsDigit(Line[DigitIndex]))
	{
		++DigitIndex;
	}

	return DigitIndex > Index &&
		DigitIndex + 1 < Line.Len() &&
		(Line[DigitIndex] == TEXT('.') || Line[DigitIndex] == TEXT(')')) &&
		FChar::IsWhitespace(Line[DigitIndex + 1]);
}

bool ContainsMarkdownListLine(const FString& Text)
{
	TArray<FString> Lines;
	Text.ParseIntoArrayLines(Lines, false);
	for (const FString& Line : Lines)
	{
		if (IsMarkdownListLine(Line))
		{
			return true;
		}
	}
	return false;
}

bool IsMarkdownTableDividerLine(const FString& Line)
{
	if (!Line.Contains(TEXT("|")))
	{
		return false;
	}

	TArray<FString> Cells;
	Line.ParseIntoArray(Cells, TEXT("|"), true);
	if (Cells.Num() < 2)
	{
		return false;
	}

	for (FString Cell : Cells)
	{
		Cell.TrimStartAndEndInline();
		bool bContainsDash = false;
		for (const TCHAR Character : Cell)
		{
			if (Character == TEXT('-'))
			{
				bContainsDash = true;
				continue;
			}
			if (Character != TEXT(':') && !FChar::IsWhitespace(Character))
			{
				return false;
			}
		}
		if (!bContainsDash)
		{
			return false;
		}
	}

	return true;
}

bool ContainsMarkdownTableCandidate(const FString& Text)
{
	TArray<FString> Lines;
	Text.ParseIntoArrayLines(Lines, false);
	for (int32 Index = 1; Index < Lines.Num(); ++Index)
	{
		if (Lines[Index - 1].Contains(TEXT("|")) && IsMarkdownTableDividerLine(Lines[Index]))
		{
			return true;
		}
	}
	return false;
}

bool ContainsMarkdownInlineSyntax(const FString& Text)
{
	return CountUnescapedToken(Text, TEXT("`")) > 0 ||
		CountUnescapedToken(Text, TEXT("*")) > 0 ||
		CountUnescapedToken(Text, TEXT("_")) > 0 ||
		CountUnescapedToken(Text, TEXT("~")) > 0 ||
		CountUnescapedToken(Text, TEXT("[")) > 0;
}
}

bool MarkdownSlate::ShouldRenderPendingStreamingTextAsPlainText(const FString& PendingText)
{
	if (PendingText.IsEmpty())
	{
		return false;
	}

	if (!PendingText.Contains(TEXT("\n")))
	{
		return true;
	}

	if (ContainsMarkdownListLine(PendingText))
	{
		return true;
	}

	if (ContainsMarkdownTableCandidate(PendingText))
	{
		return true;
	}

	if (ContainsMarkdownInlineSyntax(PendingText))
	{
		return true;
	}

	return HasUnclosedLinkOrImage(PendingText);
}

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
		.AutoHeight()
		[
			SAssignNew(StreamingContentBox, SVerticalBox)
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
		StreamingContentBox->RemoveSlot(PendingContentBox.ToSharedRef());
		PendingContentBox.Reset();
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
			StreamingContentBox->RemoveSlot(PendingContentBox.ToSharedRef());
			PendingContentBox.Reset();
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

	if (MarkdownSlate::ShouldRenderPendingStreamingTextAsPlainText(PendingText))
	{
		FSlateFontInfo Font = ThemeConfig.DefaultFont;
		Font.Size = ThemeConfig.BodyFontSize;
		PendingContentBox->SetContent(
			FMarkdownSlateRenderer::RenderTextWithEmoji(
				PendingText,
				ThemeConfig,
				Font,
				ThemeConfig.BodyFontSize,
				ThemeConfig.BodyTextColor)
		);
		return;
	}

	TSharedRef<SVerticalBox> PendingVBox = SNew(SVerticalBox);
	if (TSharedPtr<FMarkdownRenderNode> PendingRoot = BuildRenderRoot(PendingText))
	{
		FMarkdownSlateRenderer::AppendChildren(PendingVBox, PendingRoot, ThemeConfig);
	}
	PendingContentBox->SetContent(PendingVBox);
}
