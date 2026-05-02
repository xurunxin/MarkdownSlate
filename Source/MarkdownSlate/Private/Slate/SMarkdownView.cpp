#include "Slate/SMarkdownView.h"
#include "Render/MarkdownRenderBuilder.h"
#include "Parser/MarkdownParser.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

void SMarkdownView::Construct(const FArguments& InArgs)
{
	MarkdownText = InArgs._MarkdownText;
	OnLinkClicked = InArgs._OnLinkClicked;
	ThemeConfig = FMarkdownSlateThemeConfig::Default();

	ChildSlot
	[
		SAssignNew(ContentBox, SVerticalBox)
	];

	RefreshDisplay();
}

void SMarkdownView::SetMarkdownText(const FString& InMarkdownText)
{
	MarkdownText.Set(InMarkdownText);
	RefreshDisplay();
}

void SMarkdownView::SetThemeConfig(const FMarkdownSlateThemeConfig& InTheme)
{
	ThemeConfig = InTheme;
	RefreshDisplay();
}

void SMarkdownView::RefreshDisplay()
{
	if (!ContentBox.IsValid())
	{
		return;
	}

	ContentBox->ClearChildren();

	FString CurrentText = MarkdownText.Get();

	if (CurrentText.IsEmpty())
	{
		ContentBox->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("")))
			];
		return;
	}

	FMarkdownParser Parser;
	TSharedPtr<FMarkdownBlockNode> AstRoot = Parser.Parse(CurrentText);

	FMarkdownRenderBuilder Builder;
	TSharedPtr<FMarkdownRenderNode> RenderRoot = Builder.Build(AstRoot);

	TSharedRef<SWidget> RenderedWidget = FMarkdownSlateRenderer::Render(RenderRoot, ThemeConfig);

	ContentBox->AddSlot()
		[
			RenderedWidget
		];
}
