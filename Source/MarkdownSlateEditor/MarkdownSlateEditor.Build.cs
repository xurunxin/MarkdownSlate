using UnrealBuildTool;

public class MarkdownSlateEditor : ModuleRules
{
	public MarkdownSlateEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"MarkdownSlate",
			"UnrealEd",
			"AssetTools",
			"ContentBrowser",
			"PropertyEditor",
			"EditorStyle",
			"ToolMenus",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Projects",
			"InputCore",
		});
	}
}
