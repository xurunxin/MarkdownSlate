#pragma once

#include "Containers/Ticker.h"
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MarkdownStreamingPerfTestWidget.generated.h"

class UMarkdownWidget;

USTRUCT(BlueprintType)
struct MARKDOWNSLATE_API FMarkdownStreamingPerfResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Markdown|Perf")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Markdown|Perf")
	FString Mode;

	UPROPERTY(BlueprintReadOnly, Category = "Markdown|Perf")
	int32 TotalChars = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Markdown|Perf")
	int32 ChunkCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Markdown|Perf")
	double ElapsedMs = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Markdown|Perf")
	double AverageChunkMs = 0.0;
};

UCLASS(BlueprintType, Blueprintable)
class MARKDOWNSLATE_API UMarkdownStreamingPerfTestWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Markdown|Perf")
	FMarkdownStreamingPerfResult RunStreamingPerformanceTest(int32 RepeatBlocks = 80, int32 ChunkSize = 256, bool bUseStreaming = true);

	UFUNCTION(BlueprintCallable, Category = "Markdown|Perf")
	FMarkdownStreamingPerfResult StartDelayedTokenStreamingTest(int32 RepeatBlocks = 12, int32 TokenSize = 8, float TokenDelaySeconds = 0.03f, bool bUseEmojiMixedText = false);

	UFUNCTION(BlueprintCallable, Category = "Markdown|Perf")
	void StopDelayedTokenStreamingTest();

	UFUNCTION(BlueprintPure, Category = "Markdown|Perf")
	bool IsDelayedTokenStreamingActive() const { return bDelayedTokenStreamingActive; }

	UFUNCTION(BlueprintPure, Category = "Markdown|Perf")
	FMarkdownStreamingPerfResult GetLastDelayedStreamingResult() const { return LastDelayedStreamingResult; }

	UFUNCTION(BlueprintCallable, Category = "Markdown|Perf")
	FMarkdownStreamingPerfResult ShowEmojiMixedTextTest(bool bUseStreaming = true, int32 TokenSize = 24);

	UFUNCTION(BlueprintPure, Category = "Markdown|Perf")
	FString GetPerfMarkdownText(int32 RepeatBlocks = 80) const;

	UFUNCTION(BlueprintPure, Category = "Markdown|Perf")
	FString GetEmojiMixedMarkdownText() const;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Markdown|Perf")
	TObjectPtr<UMarkdownWidget> StreamingMarkdown;

private:
	static FString BuildPerfMarkdown(int32 RepeatBlocks);
	static FString BuildEmojiMixedMarkdown();
	FMarkdownStreamingPerfResult ApplyMarkdownPayload(const FString& Payload, int32 ChunkSize, bool bUseStreaming, const FString& Mode);
	void AppendNextDelayedToken();
	bool TickDelayedTokenStreaming(float DeltaTime);
	void ClearDelayedTokenTicker();

	UPROPERTY(Transient)
	FString DelayedTokenPayload;

	FTSTicker::FDelegateHandle DelayedTokenTickerHandle;
	FMarkdownStreamingPerfResult LastDelayedStreamingResult;
	int32 DelayedTokenOffset = 0;
	int32 DelayedTokenSize = 1;
	double DelayedTokenStartSeconds = 0.0;
	bool bDelayedTokenStreamingActive = false;
};
