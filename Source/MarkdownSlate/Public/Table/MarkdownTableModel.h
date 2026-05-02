#pragma once

#include "CoreMinimal.h"

struct FMarkdownRenderNode;

struct MARKDOWNSLATE_API FMarkdownTableCell
{
	TSharedPtr<FMarkdownRenderNode> ContentNode;
	bool bIsHeader = false;
	int32 ColSpan = 1;
	int32 RowSpan = 1;
};

struct MARKDOWNSLATE_API FMarkdownTableModel
{
	TArray<TArray<FMarkdownTableCell>> Rows;
	int32 ColumnCount = 0;
	bool bHasColumnHeader = false;

	void SetDimensions(int32 InRows, int32 InCols)
	{
		Rows.SetNum(InRows);
		for (int32 r = 0; r < InRows; ++r)
		{
			Rows[r].SetNum(InCols);
		}
		ColumnCount = InCols;
	}

	FMarkdownTableCell& At(int32 Row, int32 Col)
	{
		return Rows[Row][Col];
	}

	bool IsValid() const { return Rows.Num() > 0 && ColumnCount > 0; }
	int32 NumRows() const { return Rows.Num(); }
};
