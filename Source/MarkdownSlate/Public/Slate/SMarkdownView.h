#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Slate/MarkdownSlateRenderer.h"

DECLARE_DELEGATE_OneParam(FOnMarkdownViewLinkClickedSlate, const FString& /*Url*/);

class MARKDOWNSLATE_API SMarkdownView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMarkdownView)
		: _MarkdownText(TEXT(""))
	{}
		SLATE_ATTRIBUTE(FString, MarkdownText)
		SLATE_EVENT(FOnMarkdownViewLinkClickedSlate, OnLinkClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetMarkdownText(const FString& InMarkdownText);
	void SetThemeConfig(const FMarkdownSlateThemeConfig& InTheme);
	void RefreshDisplay();

private:
	TAttribute<FString> MarkdownText;
	TSharedPtr<SVerticalBox> ContentBox;
	FOnMarkdownViewLinkClickedSlate OnLinkClicked;
	FMarkdownSlateThemeConfig ThemeConfig;
};
