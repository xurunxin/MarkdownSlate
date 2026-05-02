#include "Streaming/MarkdownStreamingBuffer.h"

void FMarkdownStreamingBuffer::Append(const FString& Chunk)
{
	PendingChunk += Chunk;
	FString Stable = FindStableBoundary(PendingChunk);
	if (!Stable.IsEmpty())
	{
		StableText += Stable;
		PendingChunk = PendingChunk.RightChop(Stable.Len());
	}
}

FString FMarkdownStreamingBuffer::GetStableText() const { return StableText; }
FString FMarkdownStreamingBuffer::GetPendingText() const { return PendingChunk; }

void FMarkdownStreamingBuffer::Reset()
{
	StableText.Reset();
	PendingChunk.Reset();
}

FString FMarkdownStreamingBuffer::FindStableBoundary(const FString& Text) const
{
	// Find last newline — everything before it is stable
	int32 LastNL = -1;
	for (int32 i = Text.Len() - 1; i >= 0; --i)
	{
		if (Text[i] == '\n') { LastNL = i; break; }
	}
	if (LastNL >= 0) return Text.Left(LastNL + 1);

	// No newline: find last complete paragraph (double newline in source)
	// For simplicity, if no newline at all, return empty (keep all in pending)
	if (Text.Len() > 4096)
		return Text.Left(Text.Len() - 1024);

	return FString();
}
