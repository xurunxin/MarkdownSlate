#pragma once

#include "CoreMinimal.h"
#include "MarkdownRenderCacheKey.h"

struct FMarkdownRenderNode;

class MARKDOWNSLATE_API FMarkdownRenderCache
{
public:
	FMarkdownRenderCache();
	explicit FMarkdownRenderCache(int32 InMaxEntries);

	TSharedPtr<FMarkdownRenderNode> Get(const FMarkdownRenderCacheKey& Key) const;
	void Put(const FMarkdownRenderCacheKey& Key, TSharedPtr<FMarkdownRenderNode> Node);
	void Invalidate();
	void SetMaxEntries(int32 InMaxEntries);

	int32 GetEntryCount() const { return CacheMap.Num(); }
	int32 GetMaxEntries() const { return MaxEntries; }

private:
	int32 MaxEntries;
	TMap<FMarkdownRenderCacheKey, TSharedPtr<FMarkdownRenderNode>> CacheMap;
	TArray<FMarkdownRenderCacheKey> InsertionOrder;

	void EvictOldest();
};
