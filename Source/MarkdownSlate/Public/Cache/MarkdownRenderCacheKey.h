#pragma once

#include "CoreMinimal.h"

struct MARKDOWNSLATE_API FMarkdownRenderCacheKey
{
	uint32 SourceHash = 0;
	uint32 ParseFlags = 0;
	uint32 ThemeHash = 0;
	int32  WidthBucket = 0;
	uint32 FeatureMask = 0;
	uint32 SchemaVersion = 1;

	FMarkdownRenderCacheKey() = default;

	bool operator==(const FMarkdownRenderCacheKey& Other) const
	{
		return SourceHash == Other.SourceHash
			&& ParseFlags == Other.ParseFlags
			&& ThemeHash == Other.ThemeHash
			&& WidthBucket == Other.WidthBucket
			&& FeatureMask == Other.FeatureMask
			&& SchemaVersion == Other.SchemaVersion;
	}

	bool operator!=(const FMarkdownRenderCacheKey& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FMarkdownRenderCacheKey& Key)
	{
		return HashCombine(HashCombine(HashCombine(HashCombine(HashCombine(
			Key.SourceHash,
			Key.ParseFlags),
			Key.ThemeHash),
			Key.WidthBucket),
			Key.FeatureMask),
			Key.SchemaVersion);
	}

	static int32 MakeWidthBucket(float Width, float BucketSize = 50.0f)
	{
		if (Width <= 0.0f) return 0;
		return FMath::FloorToInt(Width / BucketSize);
	}
};
