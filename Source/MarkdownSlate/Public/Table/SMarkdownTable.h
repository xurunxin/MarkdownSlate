#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Table/MarkdownTableModel.h"
#include "Slate/MarkdownSlateRenderer.h"

class MARKDOWNSLATE_API SMarkdownTable : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMarkdownTable)
		: _TableModel(nullptr)
		, _ThemeConfig()
	{}
		SLATE_ARGUMENT(TSharedPtr<FMarkdownTableModel>, TableModel)
		SLATE_ARGUMENT(FMarkdownSlateThemeConfig, ThemeConfig)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
