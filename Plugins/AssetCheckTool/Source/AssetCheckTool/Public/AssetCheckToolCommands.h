// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "AssetCheckToolStyle.h"
#include "CoreMinimal.h"

#include "IAssetTools.h"
#include "CTGTracker.inl"
#include "Modules/ModuleManager.h"
#include "ContentBrowserModule.h"
#include "AssetCheckToolCommands.h"
#include "AssetToolsModule.h"
#include "Misc/MessageDialog.h"

#if ENGINE_MAJOR_VERSION >= 5
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorStyleSet.h"
#else
#include "AssetRegistryModule.h"
#endif


#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "EditorStyle.h"
#include "ToolMenus.h"
#include "EditorAssetLibrary.h"
#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"
#include "ContentBrowserMenuContexts.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"


class FAssetCheckToolCommands : public TCommands<FAssetCheckToolCommands>
{
public:

	FAssetCheckToolCommands()
		: TCommands<FAssetCheckToolCommands>(TEXT("AssetCheckTool"), NSLOCTEXT("Contexts", "AssetCheckTool", "AssetCheckTool Plugin"), NAME_None, FAssetCheckToolStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;

	// 创建菜单扩展实例

	//static TSharedRef<FExtender> OnExtendContentBrowserPathSelectionMenu(const TArray<FString>& SelectedPaths);
	static TSharedRef<FExtender> OnExtendContentBrowserPathSelectionMenu(const TArray<FString>& SelectedPaths);
	// 对资产右键
	static void AddAssetContextMenuExtension();

	static void CreateFoldersInSelectedPath(const TArray<FString>& SelectedPaths);
	static void CreateCreateFoldersContentBrowserPathMenu(FMenuBuilder& MenuBuilder, TArray<FString> SelectedPaths);



	// 创建菜单扩展实例end
};
