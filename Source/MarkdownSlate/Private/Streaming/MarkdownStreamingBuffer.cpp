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
	// Markdown blocks can depend on following lines (tables, lists, fenced code),
	// so a single newline is not a safe permanent rendering boundary.
	for (int32 i = Text.Len() - 2; i >= 0; --i)
	{
		if (Text[i] == '\n' && Text[i + 1] == '\n')
		{
			return Text.Left(i + 2);
		}
	}

	if (Text.Len() > 4096)
	{
		return Text.Left(Text.Len() - 1024);
	}

	return FString();
}
