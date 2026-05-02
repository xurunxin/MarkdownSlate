#include "Menus/MarkdownSlateMenus.h"
#include "ToolMenus.h"

void FMarkdownSlateMenus::Register()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus) return;

	UToolMenu* Menu = ToolMenus->ExtendMenu("MainFrame.MainMenu.Window");
	if (!Menu) return;

	FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
	Section.AddMenuEntry(
		"MarkdownPreviewTab",
		NSLOCTEXT("MarkdownSlate", "MarkdownPreview", "Markdown Preview"),
		NSLOCTEXT("MarkdownSlate", "MarkdownPreviewTooltip", "Open Markdown Preview tab"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			FGlobalTabmanager::Get()->TryInvokeTab(FName("MarkdownPreview"));
		}))
	);
}
