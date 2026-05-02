#include "Table/SMarkdownTable.h"
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
	int32 NumCols = FMath::Max(Model->ColumnCount, 1);
	for (int32 c = 0; c < NumCols; ++c)
		Grid->SetColumnFill(c, 1.0f / NumCols);

	for (int32 Row = 0; Row < Model->NumRows(); ++Row)
	{
		bool bIsHeader = Model->bHasColumnHeader && Row == 0;
		for (int32 Col = 0; Col < Model->ColumnCount; ++Col)
		{
			const FMarkdownTableCell& Cell = Model->Rows[Row][Col];
			FLinearColor BgColor = bIsHeader
				? Theme.TableHeaderBgColor
				: (Row % 2 == 0 ? Theme.TableRowEvenBgColor : Theme.TableRowOddBgColor);

			Grid->AddSlot(Col, Row)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BgColor)
					.Padding(FMargin(Theme.TableCellPaddingH, Theme.TableCellPaddingV))
					[
						Cell.ContentNode.IsValid()
							? FMarkdownSlateRenderer::RenderInlines(Cell.ContentNode, Theme)
							: static_cast<TSharedRef<SWidget>>(
								SNew(STextBlock).Text(FText::GetEmpty())
								.Font([&]{ FSlateFontInfo F = Theme.DefaultFont; F.Size = Theme.BodyFontSize; return F; }()))
					]
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
