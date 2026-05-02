#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class MARKDOWNSLATEEDITOR_API SMarkdownPreviewTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMarkdownPreviewTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FString MarkdownText;
};

class FMarkdownPreviewTab
{
public:
	static const FName TabId;
	static TSharedRef<class SDockTab> CreateTab(const FSpawnTabArgs& Args);
	static void RegisterTabSpawner();
};
