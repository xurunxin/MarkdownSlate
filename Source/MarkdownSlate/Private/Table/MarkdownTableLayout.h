#pragma once

#include "CoreMinimal.h"

struct FMarkdownTableLayout
{
	static TArray<float> NormalizeColumnWidths(const TArray<float>& DesiredWidths, float MinimumWidth);
};
