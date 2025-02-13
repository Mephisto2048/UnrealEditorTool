// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class ISceneOutliner;
class ISceneOutlinerColumn;
class UEditorActorSubsystem;

class FMfstManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	/* ContentBrowserMenu */
	void InitContentBrowserMenuExtension();
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);
	void AddCBMenuEntry(FMenuBuilder& MenuBuilder);
	
	void OnDeleteUnusedAssetButtonClicked();
	void OnDeleteEmptyFolderButtonClicked();
	void OnAdvanceDeletionButtonClicked();
	TArray<FString> FolderPathsSelected;
	
	/* CustomEditorTab */
	void RegisterAdvanceDeletionTab();
	TSharedRef<SDockTab> OnSpawnAdvanceDeletionTab(const FSpawnTabArgs&);
	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSeletedFolder();
	
	/* LevelEditorExtension */
	void InitLevelEditorExtension();
	TSharedRef<FExtender>CustomLEMenuExtender(const TSharedRef<FUICommandList> UICommandList, const TArray<AActor*> SelectedActors);
	void AddLEMenuEntry(FMenuBuilder& MenuBuilder);
	
	void OnLockActorButtonClicked();
	void OnUnlockActorButtonClicked();

	void RefreshSceneOutliner();
	
	/*  */
	void InitCustomSelectionEvent();
	void OnActorSelected(UObject* SelectedObject);
	void LockActorSelection(AActor* InActor);
	void UnlockActorSelection(AActor* InActor);
	
	/* CustomEditorUICommand */
	void InitCustomUICommands();
	TSharedPtr<FUICommandList> CustomUICommandList;
	void OnSelectionLockHotKeyPress();
	void OnSelectionUnlockHotKeyPress();

	/* OutlinerExtension */
	void InitSceneOutlinerColumnExtension();
	TSharedRef<ISceneOutlinerColumn> OnCreateSelectionLockColumn(ISceneOutliner& SceneOutliner); 
	/* Other */
	bool GetEditorActorSubSystem();
	UEditorActorSubsystem* EditorActorSubsystem;

	void FixUpRedirectors();
	//for slate
public:
	
	static bool DeleteSingleAssetForAssetList(const FAssetData& AssetData);
	static bool DeleteMulAssetsForAssetList(const TArray< FAssetData>& AssetsData);
	static void ListUnusedAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& InAssetsData,TArray<TSharedPtr<FAssetData>>& OutAssetsData);
	static void ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& InAssetsData,TArray<TSharedPtr<FAssetData>>& OutAssetsData);
	static void SyncCBToClickedAssetForAssetList(const FString& AssetPath);
	//for outliner
	bool IsActorSelectionLocked(AActor* InActor);
	void ProcessLockingForOutliner(AActor* ActorToProcess, bool bShouldLock);
};
 