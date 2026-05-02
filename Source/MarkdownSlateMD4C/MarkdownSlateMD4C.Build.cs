using UnrealBuildTool;

public class MarkdownSlateMD4C : ModuleRules
{
	public MarkdownSlateMD4C(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = false;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
		});

		string Md4cPath = System.IO.Path.Combine(ModuleDirectory, "Private", "ThirdParty", "md4c", "src");
		PublicIncludePaths.Add(Md4cPath);
	}
}
