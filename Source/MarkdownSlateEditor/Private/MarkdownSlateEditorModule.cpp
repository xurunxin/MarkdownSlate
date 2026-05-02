#include "MarkdownSlateEditor.h"
#include "Modules/ModuleManager.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetTools/AssetTypeActions_MarkdownDocument.h"
#include "AssetTools/AssetTypeActions_MarkdownTheme.h"
#include "Tabs/MarkdownPreviewTab.h"
#include "Menus/MarkdownSlateMenus.h"
#include "Widgets/Docking/SDockTab.h"

const FName FMarkdownPreviewTab::TabId = TEXT("MarkdownPreview");

void FMarkdownSlateEditorModule::StartupModule()
{
	// Register AssetTypeActions
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_MarkdownDocument));
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_MarkdownTheme));

	// Register Preview Tab
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FMarkdownPreviewTab::TabId,
		FOnSpawnTab::CreateStatic(&FMarkdownPreviewTab::CreateTab))
		.SetDisplayName(NSLOCTEXT("MarkdownSlate", "MarkdownPreviewTab", "Markdown Preview"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	// Register ToolMenus
	FMarkdownSlateMenus::Register();
}

void FMarkdownSlateEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		// AssetTypeActions auto-unregister on module shutdown
	}

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FMarkdownPreviewTab::TabId);
}

IMPLEMENT_MODULE(FMarkdownSlateEditorModule, MarkdownSlateEditor)
