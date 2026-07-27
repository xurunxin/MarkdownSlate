#include "Table/SMarkdownTable.h"
#include "Table/MarkdownTableLayout.h"
#include "Slate/MarkdownSlateRenderer.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

void SMarkdownTable::Construct(const FArguments& InArgs)
{
	TSharedPtr<FMarkdownTableModel> Model = InArgs._TableModel;
	const FMarkdownSlateThemeConfig& Theme = InArgs._ThemeConfig;

	if (!Model.IsValid() || !Model->IsValid())
	{
		ChildSlot[SNew(STextBlock).Text(FText::FromString(TEXT("")))]; return;
	}

	TSharedRef<SGridPanel> Grid = SNew(SGridPanel);
	const int32 NumCols = FMath::Max(Model->ColumnCount, 1);
	TArray<float> ColumnDesiredWidths;
	ColumnDesiredWidths.Init(0.0f, NumCols);
	TArray<TArray<TSharedRef<SWidget>>> CellWidgets;
	CellWidgets.Reserve(Model->NumRows());

	for (int32 Row = 0; Row < Model->NumRows(); ++Row)
	{
		bool bIsHeader = Model->bHasColumnHeader && Row == 0;
		TArray<TSharedRef<SWidget>>& RowWidgets = CellWidgets.AddDefaulted_GetRef();
		RowWidgets.Reserve(Model->ColumnCount);
		for (int32 Col = 0; Col < Model->ColumnCount; ++Col)
		{
			const FMarkdownTableCell& Cell = Model->Rows[Row][Col];
			FLinearColor BgColor = bIsHeader
				? Theme.TableHeaderBgColor
				: (Row % 2 == 0 ? Theme.TableRowEvenBgColor : Theme.TableRowOddBgColor);

			const TSharedRef<SWidget> CellContent = Cell.ContentNode.IsValid()
				? FMarkdownSlateRenderer::RenderInlines(Cell.ContentNode, Theme)
				: static_cast<TSharedRef<SWidget>>(
					SNew(STextBlock).Text(FText::GetEmpty())
					.Font([&]{ FSlateFontInfo F = Theme.DefaultFont; F.Size = Theme.BodyFontSize; return F; }()));
			CellContent->SlatePrepass();
			ColumnDesiredWidths[Col] = FMath::Max(
				ColumnDesiredWidths[Col],
				CellContent->GetDesiredSize().X + Theme.TableCellPaddingH * 2.0f);

			RowWidgets.Add(
				SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BgColor)
					.Padding(FMargin(Theme.TableCellPaddingH, Theme.TableCellPaddingV))
				[
					CellContent
				]);
		}
	}

	const float MinimumColumnWidth = Theme.BodyFontSize * 2.0f + Theme.TableCellPaddingH * 2.0f;
	const TArray<float> ColumnFillWeights = FMarkdownTableLayout::NormalizeColumnWidths(
		ColumnDesiredWidths,
		MinimumColumnWidth);
	for (int32 Col = 0; Col < NumCols; ++Col)
	{
		Grid->SetColumnFill(Col, ColumnFillWeights.IsValidIndex(Col) ? ColumnFillWeights[Col] : 1.0f / NumCols);
	}

	for (int32 Row = 0; Row < CellWidgets.Num(); ++Row)
	{
		for (int32 Col = 0; Col < CellWidgets[Row].Num(); ++Col)
		{
			Grid->AddSlot(Col, Row)
			[
				CellWidgets[Row][Col]
			];
		}
	}

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(Theme.TableBorderColor)
		.Padding(FMargin(Theme.TableBorderThickness))
		[
			Grid
		]
	];
}
