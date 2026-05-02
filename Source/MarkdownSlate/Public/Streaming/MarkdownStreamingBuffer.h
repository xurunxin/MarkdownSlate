#pragma once

#include "CoreMinimal.h"

class MARKDOWNSLATE_API FMarkdownStreamingBuffer
{
public:
	void Append(const FString& Chunk);
	FString GetStableText() const;
	FString GetPendingText() const;
	void Reset();
	bool HasPending() const { return !PendingChunk.IsEmpty(); }

private:
	FString StableText;
	FString PendingChunk;

	FString FindStableBoundary(const FString& Text) const;
};
