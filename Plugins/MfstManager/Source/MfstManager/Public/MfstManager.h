// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

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
	
	/*  */
	void InitCustomSelectionEvent();
	void OnActorSelected(UObject* SelectedObject);
	void LockActorSelection(AActor* InActor);
	void UnlockActorSelection(AActor* InActor);
	bool IsActorSelectionLocked(AActor* InActor);

	/* CustomEditorUICommand */
	void InitCustomUICommands();
	TSharedPtr<FUICommandList> CustomUICommandList;
	void OnSelectionLockHotKeyPress();
	void OnSelectionUnlockHotKeyPress();
	
	bool GetEditorActorSubSystem();
	UEditorActorSubsystem* EditorActorSubsystem;
	
	void FixUpRedirectors();
};
 