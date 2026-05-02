#include "Cache/MarkdownRenderCache.h"
#include "Render/MarkdownRenderNode.h"

FMarkdownRenderCache::FMarkdownRenderCache()
	: MaxEntries(64)
{
}

FMarkdownRenderCache::FMarkdownRenderCache(int32 InMaxEntries)
	: MaxEntries(FMath::Max(1, InMaxEntries))
{
}

TSharedPtr<FMarkdownRenderNode> FMarkdownRenderCache::Get(const FMarkdownRenderCacheKey& Key) const
{
	const TSharedPtr<FMarkdownRenderNode>* Found = CacheMap.Find(Key);
	return Found ? *Found : TSharedPtr<FMarkdownRenderNode>();
}

void FMarkdownRenderCache::Put(const FMarkdownRenderCacheKey& Key, TSharedPtr<FMarkdownRenderNode> Node)
{
	if (!Node.IsValid()) return;

	if (CacheMap.Contains(Key))
	{
		CacheMap[Key] = Node;
		return;
	}

	while (CacheMap.Num() >= MaxEntries)
	{
		EvictOldest();
	}

	CacheMap.Add(Key, Node);
	InsertionOrder.Add(Key);
}

void FMarkdownRenderCache::Invalidate()
{
	CacheMap.Empty();
	InsertionOrder.Empty();
}

void FMarkdownRenderCache::SetMaxEntries(int32 InMaxEntries)
{
	MaxEntries = FMath::Max(1, InMaxEntries);
	while (CacheMap.Num() > MaxEntries)
	{
		EvictOldest();
	}
}

void FMarkdownRenderCache::EvictOldest()
{
	if (InsertionOrder.Num() == 0) return;

	FMarkdownRenderCacheKey Oldest = InsertionOrder[0];
	InsertionOrder.RemoveAt(0);
	CacheMap.Remove(Oldest);
}
