#include "Widgets/MarkdownStreamingPerfTestWidget.h"

#include "Containers/Ticker.h"
#include "Widgets/MarkdownWidget.h"

FString UMarkdownStreamingPerfTestWidget::BuildPerfMarkdown(int32 RepeatBlocks)
{
	RepeatBlocks = FMath::Max(1, RepeatBlocks);

	FString Text;
	Text.Reserve(RepeatBlocks * 640);
	for (int32 Index = 0; Index < RepeatBlocks; ++Index)
	{
		Text += FString::Printf(TEXT("## Streaming section %d\n\n"), Index + 1);
		Text += TEXT("This paragraph simulates chatbot output with **bold** terms, `inline code`, links, and enough length to exercise wrapping.\n\n");
		Text += TEXT("- Clinical finding: stable\n- Recommendation: continue monitoring\n- Follow-up: schedule review\n\n");
		Text += TEXT("| Item | Value | Note |\n| --- | ---: | --- |\n");
		Text += FString::Printf(TEXT("| Row %d | %d | generated markdown payload |\n\n"), Index + 1, Index * 7);
		Text += TEXT("```txt\nstream chunk payload\n```\n\n");
	}
	return Text;
}

FString UMarkdownStreamingPerfTestWidget::BuildEmojiMixedMarkdown()
{
	return FString(
		TEXT("# Emoji mixed fallback test\n\n")
		TEXT("Face and expression: 😀 😃 😄 😁 😆 😅 😂 🙂 😉 😊 🥹 😎 🤔 😴\n\n")
		TEXT("People and skin tones: 👍 👍🏻 👍🏽 👍🏿 👋🏽 🙏🏼 🧑 🧑‍⚕️ 👩‍⚕️ 👨‍⚕️ 🧑‍💻 👩‍🔬\n\n")
		TEXT("Medical and health: 🩺 💊 💉 🩹 🧬 🦠 🫀 🫁 🧠 🏥 🚑 ⚕️\n\n")
		TEXT("Objects and symbols: ✅ ❌ ⚠️ ℹ️ ❤️ 🧡 💛 💚 💙 💜 ⭐ ✨ 🔥 💧\n\n")
		TEXT("Food, nature, travel: 🍎 🍵 🌡️ 🌈 ☀️ 🌙 🌍 🚗 ✈️ 🏠\n\n")
		TEXT("Flags and sequences: 🇨🇳 🇺🇸 🇯🇵 🇪🇺 🏳️‍🌈 🏴‍☠️\n\n")
		TEXT("Markdown inline mix: **bold 😀**, `code 🧪`, [link ❤️](https://example.com).\n"));
}

FString UMarkdownStreamingPerfTestWidget::GetPerfMarkdownText(int32 RepeatBlocks) const
{
	return BuildPerfMarkdown(RepeatBlocks);
}

FString UMarkdownStreamingPerfTestWidget::GetEmojiMixedMarkdownText() const
{
	return BuildEmojiMixedMarkdown();
}

FMarkdownStreamingPerfResult UMarkdownStreamingPerfTestWidget::ApplyMarkdownPayload(const FString& Payload, int32 ChunkSize, bool bUseStreaming, const FString& Mode)
{
	FMarkdownStreamingPerfResult Result;
	Result.Mode = Mode;

	if (!StreamingMarkdown)
	{
		return Result;
	}

	ChunkSize = FMath::Max(1, ChunkSize);
	Result.TotalChars = Payload.Len();
	Result.ChunkCount = FMath::DivideAndRoundUp(Result.TotalChars, ChunkSize);

	StreamingMarkdown->TakeWidget();

	const double StartSeconds = FPlatformTime::Seconds();
	if (bUseStreaming)
	{
		StreamingMarkdown->BeginStreamingMarkdown();
		for (int32 Offset = 0; Offset < Payload.Len(); Offset += ChunkSize)
		{
			StreamingMarkdown->AppendMarkdownChunk(Payload.Mid(Offset, ChunkSize));
		}
		StreamingMarkdown->EndStreamingMarkdown();
	}
	else
	{
		FString Accumulated;
		Accumulated.Reserve(Payload.Len());
		for (int32 Offset = 0; Offset < Payload.Len(); Offset += ChunkSize)
		{
			Accumulated += Payload.Mid(Offset, ChunkSize);
			StreamingMarkdown->SetMarkdownText(Accumulated);
		}
	}

	Result.ElapsedMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;
	Result.AverageChunkMs = Result.ChunkCount > 0 ? Result.ElapsedMs / Result.ChunkCount : 0.0;
	Result.bSuccess = StreamingMarkdown->MarkdownText.Len() == Payload.Len();

	UE_LOG(LogTemp, Display, TEXT("Markdown streaming perf mode=%s chars=%d chunks=%d elapsed=%.3fms avg=%.3fms"),
		*Result.Mode, Result.TotalChars, Result.ChunkCount, Result.ElapsedMs, Result.AverageChunkMs);

	return Result;
}

FMarkdownStreamingPerfResult UMarkdownStreamingPerfTestWidget::RunStreamingPerformanceTest(int32 RepeatBlocks, int32 ChunkSize, bool bUseStreaming)
{
	return ApplyMarkdownPayload(
		BuildPerfMarkdown(RepeatBlocks),
		ChunkSize,
		bUseStreaming,
		bUseStreaming ? TEXT("streaming") : TEXT("full-reset"));
}

FMarkdownStreamingPerfResult UMarkdownStreamingPerfTestWidget::StartDelayedTokenStreamingTest(int32 RepeatBlocks, int32 TokenSize, float TokenDelaySeconds, bool bUseEmojiMixedText)
{
	StopDelayedTokenStreamingTest();

	LastDelayedStreamingResult = FMarkdownStreamingPerfResult();
	LastDelayedStreamingResult.Mode = bUseEmojiMixedText ? TEXT("delayed-token-emoji") : TEXT("delayed-token");

	if (!StreamingMarkdown)
	{
		return LastDelayedStreamingResult;
	}

	DelayedTokenPayload = bUseEmojiMixedText ? BuildEmojiMixedMarkdown() : BuildPerfMarkdown(RepeatBlocks);
	DelayedTokenOffset = 0;
	DelayedTokenSize = FMath::Max(1, TokenSize);
	DelayedTokenStartSeconds = FPlatformTime::Seconds();
	bDelayedTokenStreamingActive = true;

	LastDelayedStreamingResult.bSuccess = true;
	LastDelayedStreamingResult.TotalChars = DelayedTokenPayload.Len();
	LastDelayedStreamingResult.ChunkCount = FMath::DivideAndRoundUp(DelayedTokenPayload.Len(), DelayedTokenSize);

	StreamingMarkdown->TakeWidget();
	StreamingMarkdown->BeginStreamingMarkdown();
	AppendNextDelayedToken();

	const float Delay = FMath::Max(0.0f, TokenDelaySeconds);
	if (bDelayedTokenStreamingActive)
	{
		if (Delay <= 0.0f)
		{
			while (bDelayedTokenStreamingActive)
			{
				AppendNextDelayedToken();
			}
		}
		else
		{
			DelayedTokenTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
				FTickerDelegate::CreateUObject(this, &UMarkdownStreamingPerfTestWidget::TickDelayedTokenStreaming),
				Delay);
		}
	}

	return LastDelayedStreamingResult;
}

void UMarkdownStreamingPerfTestWidget::StopDelayedTokenStreamingTest()
{
	ClearDelayedTokenTicker();

	if (bDelayedTokenStreamingActive && StreamingMarkdown)
	{
		StreamingMarkdown->EndStreamingMarkdown();
	}

	bDelayedTokenStreamingActive = false;
	DelayedTokenPayload.Reset();
	DelayedTokenOffset = 0;
}

void UMarkdownStreamingPerfTestWidget::ClearDelayedTokenTicker()
{
	if (DelayedTokenTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(DelayedTokenTickerHandle);
		DelayedTokenTickerHandle.Reset();
	}
}

bool UMarkdownStreamingPerfTestWidget::TickDelayedTokenStreaming(float DeltaTime)
{
	AppendNextDelayedToken();
	if (!bDelayedTokenStreamingActive)
	{
		DelayedTokenTickerHandle.Reset();
		return false;
	}
	return true;
}

void UMarkdownStreamingPerfTestWidget::AppendNextDelayedToken()
{
	if (!bDelayedTokenStreamingActive || !StreamingMarkdown)
	{
		StopDelayedTokenStreamingTest();
		return;
	}

	if (DelayedTokenOffset < DelayedTokenPayload.Len())
	{
		StreamingMarkdown->AppendMarkdownChunk(DelayedTokenPayload.Mid(DelayedTokenOffset, DelayedTokenSize));
		DelayedTokenOffset += DelayedTokenSize;
	}

	if (DelayedTokenOffset >= DelayedTokenPayload.Len())
	{
		StreamingMarkdown->EndStreamingMarkdown();
		LastDelayedStreamingResult.ElapsedMs = (FPlatformTime::Seconds() - DelayedTokenStartSeconds) * 1000.0;
		LastDelayedStreamingResult.AverageChunkMs = LastDelayedStreamingResult.ChunkCount > 0 ? LastDelayedStreamingResult.ElapsedMs / LastDelayedStreamingResult.ChunkCount : 0.0;
		LastDelayedStreamingResult.bSuccess = StreamingMarkdown->MarkdownText.Len() == LastDelayedStreamingResult.TotalChars;
		bDelayedTokenStreamingActive = false;

		UE_LOG(LogTemp, Display, TEXT("Markdown delayed token perf mode=%s chars=%d chunks=%d elapsed=%.3fms avg=%.3fms"),
			*LastDelayedStreamingResult.Mode,
			LastDelayedStreamingResult.TotalChars,
			LastDelayedStreamingResult.ChunkCount,
			LastDelayedStreamingResult.ElapsedMs,
			LastDelayedStreamingResult.AverageChunkMs);
	}
}

FMarkdownStreamingPerfResult UMarkdownStreamingPerfTestWidget::ShowEmojiMixedTextTest(bool bUseStreaming, int32 TokenSize)
{
	return ApplyMarkdownPayload(
		BuildEmojiMixedMarkdown(),
		TokenSize,
		bUseStreaming,
		bUseStreaming ? TEXT("emoji-mixed-streaming") : TEXT("emoji-mixed-full"));
}
