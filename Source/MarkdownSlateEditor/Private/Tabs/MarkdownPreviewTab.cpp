#include "Tabs/MarkdownPreviewTab.h"
#include "Slate/SMarkdownView.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "MarkdownSlate"

void SMarkdownPreviewTab::Construct(const FArguments& InArgs)
{
	MarkdownText = TEXT("# Markdown Preview\n\nEnter markdown text to preview.");

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.Padding(8)
			[
				SNew(SMarkdownView)
				.MarkdownText(MarkdownText)
			]
		]
	];
}

TSharedRef<SDockTab> FMarkdownPreviewTab::CreateTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("MarkdownPreviewTitle", "Markdown Preview"))
		[
			SNew(SMarkdownPreviewTab)
		];
}

#undef LOCTEXT_NAMESPACE
