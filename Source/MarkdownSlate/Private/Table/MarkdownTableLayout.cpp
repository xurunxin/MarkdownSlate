#include "Table/MarkdownTableLayout.h"

TArray<float> FMarkdownTableLayout::NormalizeColumnWidths(const TArray<float>& DesiredWidths, const float MinimumWidth)
{
	TArray<float> Weights;
	Weights.Reserve(DesiredWidths.Num());

	const float SafeMinimumWidth = FMath::Max(0.0f, MinimumWidth);
	float TotalWidth = 0.0f;
	for (const float DesiredWidth : DesiredWidths)
	{
		const float SafeWidth = FMath::Max(DesiredWidth, SafeMinimumWidth);
		Weights.Add(SafeWidth);
		TotalWidth += SafeWidth;
	}

	if (TotalWidth <= KINDA_SMALL_NUMBER)
	{
		const float EqualWeight = Weights.Num() > 0 ? 1.0f / Weights.Num() : 0.0f;
		for (float& Weight : Weights)
		{
			Weight = EqualWeight;
		}
		return Weights;
	}

	for (float& Weight : Weights)
	{
		Weight /= TotalWidth;
	}
	return Weights;
}
