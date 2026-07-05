#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Slate/MarkdownSlateRenderer.h"
#include "Cache/MarkdownRenderCache.h"
#include "Emoji/MarkdownEmojiAssetProvider.h"
#include "Streaming/MarkdownStreamingBuffer.h"

DECLARE_DELEGATE_OneParam(FOnMarkdownViewLinkClickedSlate, const FString& /*Url*/);

class SBox;

class MARKDOWNSLATE_API SMarkdownView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMarkdownView)
		: _MarkdownText(TEXT(""))
	{}
		SLATE_ATTRIBUTE(FString, MarkdownText)
		SLATE_EVENT(FOnMarkdownViewLinkClickedSlate, OnLinkClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetMarkdownText(const FString& InMarkdownText);
	void SetThemeConfig(const FMarkdownSlateThemeConfig& InTheme);
	void RefreshDisplay();
	void InvalidateCache();
	void BeginStreamingMarkdown();
	void AppendMarkdownChunk(const FString& Chunk);
	void EndStreamingMarkdown();

private:
	TAttribute<FString> MarkdownText;
	TSharedPtr<SVerticalBox> ContentBox;
	TSharedPtr<SVerticalBox> StreamingContentBox;
	TSharedPtr<SBox> PendingContentBox;
	FOnMarkdownViewLinkClickedSlate OnLinkClicked;
	FMarkdownSlateThemeConfig ThemeConfig;
	TSharedPtr<FMarkdownAtlasEmojiProvider> EmojiProvider;
	FMarkdownRenderCache RenderCache;
	FMarkdownStreamingBuffer StreamingBuffer;
	FString StreamingFullText;
	int32 RenderedStableTextLen = 0;
	bool bIsStreamingMarkdown = false;

	TSharedPtr<FMarkdownRenderNode> BuildRenderRoot(const FString& SourceText);
	TSharedRef<SWidget> RenderMarkdownText(const FString& SourceText);
	void AppendStableStreamingText(const FString& StableText);
	void UpdatePendingStreamingText(const FString& PendingText);
};
