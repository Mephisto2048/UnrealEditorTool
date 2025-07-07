// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetCheckTool.h"

#include "AssetCheckToolBPLibrary.h"
#include "AssetCheckToolStyle.h"
#include "AssetCheckToolCommands.h"
#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"
#include "Misc/MessageDialog.h"

#include "LevelEditor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "ToolMenus.h"
//#include "Engine/StaticMeshConfig.h"
//#include "ForArashi/BPLibraryForArashi.h"


static const FName AssetCheckToolTabName("AssetCheckTool");

#define LOCTEXT_NAMESPACE "FAssetCheckToolModule"

void FAssetCheckToolModule::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FAssetCheckToolModule::OnPostEngineInit);

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FAssetCheckToolStyle::Initialize();
	FAssetCheckToolStyle::ReloadTextures();

	FAssetCheckToolCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FAssetCheckToolCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FAssetCheckToolModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAssetCheckToolModule::RegisterMenus));

	void RegisterSettings();
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{

		SettingsModule->RegisterSettings("Project", "Plugins", "AssetCheckTool",
			LOCTEXT("EditorSettingsName", "AssetCheckTool"),
			LOCTEXT("EditorSettingsDescription", "Configure the asset check tool settings"),
			GetMutableDefault<UAssetCheckToolConfig>());

	}
	// 创建菜单扩展实例
	// 注册内容浏览器路径选择菜单扩展
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	//右键文件夹
	TArray<FContentBrowserMenuExtender_SelectedPaths>& MenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();
	MenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateStatic(&FAssetCheckToolCommands::OnExtendContentBrowserPathSelectionMenu));

	//右键空白处
	TArray<FContentBrowserMenuExtender_SelectedPaths>& BorrowMenuExtenders = ContentBrowserModule.GetAllAssetContextMenuExtenders();
	BorrowMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateStatic(&FAssetCheckToolCommands::OnExtendContentBrowserPathSelectionMenu));


#if WITH_EDITOR
	{
		// Arashi 怪猎特用
		//FStaticMeshConfigDelegates::OnRefleshConfig.BindStatic(UBPLibraryForArashi::RefleshConfig);
		//FStaticMeshConfigDelegates::ShouldApplyConfigFromGroup.BindStatic(UBPLibraryForArashi::ShouldApplyConfigFromGroup);
	}
#endif
}

void FAssetCheckToolModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FAssetCheckToolStyle::Shutdown();

	FAssetCheckToolCommands::Unregister();

#if WITH_EDITOR
	{
		// Arashi 怪猎特用
		//FStaticMeshConfigDelegates::OnRefleshConfig.Unbind();
		//FStaticMeshConfigDelegates::ShouldApplyConfigFromGroup.Unbind();
	}

	// 取消注册内容浏览器路径选择菜单扩展
	if (FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>("ContentBrowser");
		TArray<FContentBrowserMenuExtender_SelectedPaths>& MenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();
		MenuExtenders.RemoveAll([](const FContentBrowserMenuExtender_SelectedPaths& Extender) { return true; });
	}



#endif
}

void FAssetCheckToolModule::PluginButtonClicked()
{


	FSoftObjectPath EditorPath = FSoftObjectPath("/AssetCheckTool/EUW_AssetCheckTools.EUW_AssetCheckTools");
	UObject* MyAssetEditorPath = EditorPath.TryLoad();
	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	UEditorUtilityWidgetBlueprint* EditorWidget = Cast<UEditorUtilityWidgetBlueprint>(MyAssetEditorPath);
	EditorUtilitySubsystem->SpawnAndRegisterTab(EditorWidget);
	TACraft::SendTrackingMessageV2(TEXT("AssetCheckTool"), TEXT("1.0"), TEXT("MH"), TEXT("Timi J5"), false);

}

void FAssetCheckToolModule::OnPostEngineInit()
{
	/*if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Game", "StaticMeshConfig",
			FText::FromString("Arashi Static Mesh Config"),
			FText::FromString("Configure static meshes"),
			GetMutableDefault<UStaticMeshConfig>());
	}*/
}

void FAssetCheckToolModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FAssetCheckToolCommands::Get().PluginAction, PluginCommands);
		}
	}

	{


#if ENGINE_MAJOR_VERSION >= 5
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar"); // UE5 toolbar位置
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools_AssetCheckTool");
#else
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar"); // UE4 toolbar位置
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
#endif
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FAssetCheckToolCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
	if (UToolMenus* ToolMenus = UToolMenus::Get())
	{
		// 添加资产右键菜单扩展
		FAssetCheckToolCommands::AddAssetContextMenuExtension();
	}
}




#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAssetCheckToolModule, AssetCheckTool)