// Some copyright should be here...

using System.IO;
using UnrealBuildTool;


public class AssetCheckTool : ModuleRules
{
	public AssetCheckTool(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		OptimizeCode = CodeOptimization.Never;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);

		PrivateIncludePaths.AddRange(
			new string[] {
				//Path.Combine(PluginDirectory,)
				// ... add other private include paths required here ...
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{

				"Core","Foliage","UnrealEd","AssetTools","LevelEditor","Engine", "FoliageEdit",	"Blutility", "UMGEditor", "UMG","EditorSubsystem","Landscape","SourceControl","Json", "JsonUtilities","ContentBrowser","EditorStyle","ToolMenus","MaterialEditor",
				// ... add other public dependencies that you statically link with here ...
			}
			);
		if (Target.Version.MajorVersion >= 5) //ue5版本依赖模块
		{
			PublicDependencyModuleNames.Add("EditorFramework");
		}

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"HTTP",
				"Projects",
				"InputCore",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"EditorScriptingUtilities",
				"FoliageEdit",
				"MeshDescription",
				"Json",
				"JsonUtilities", "SkeletalMeshUtilitiesCommon", "ClothingSystemRuntimeCommon", "Paper2D",
				// ... add private dependencies that you statically link with here ...
			}
			);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
