// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ISettingsModule.h"
#include "AssetCheckToolConfig.h"
#include "CTGTracker.inl"
#include "Modules/ModuleManager.h"

#if ENGINE_MAJOR_VERSION >= 5
#include "Engine/StaticMesh.h" //ue5
#else
#include "Engine/StaticMeshConfig.h" //ue4
#endif



class FToolBarBuilder;
class FMenuBuilder;

class ASSETCHECKTOOL_API FAssetCheckToolModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** This function will be bound to Command. */
	void PluginButtonClicked();

private:
	void OnPostEngineInit();
	void RegisterMenus();


private:
	TSharedPtr<class FUICommandList> PluginCommands;
	FDelegateHandle ShouldApplyConfigFromGroupHandle;

	// 创建菜单扩展实例
	//TSharedRef<FExtender> OnExtendContentBrowserPathSelectionMenu(const TArray<FString>& SelectedPaths);
	//void CreateRemoveColliderContentBrowserPathMenu(FMenuBuilder& MenuBuilder, TArray<FString> SelectedPaths);
	// 创建菜单扩展实例end
};
