// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FMfstManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	void InitContentBrowserMenuExtension();
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);
	void AddCBMenuEntry(FMenuBuilder& MenuBuilder);
	void OnDeleteUnusedAssetButtonClicked();
	void OnDeleteEmptyFolderButtonClicked();
	TArray<FString> FolderPathsSelected;
	
	void InitLevelEditorExtension();
	TSharedRef<FExtender>CustomLEMenuExtender(const TSharedRef<FUICommandList> UICommandList, const TArray<AActor*> SelectedActors);
	void AddLEMenuEntry(FMenuBuilder& MenuBuilder);
	void OnLockActorButtonClicked();
	void OnUnlockActorButtonClicked();
	TArray<AActor*> ActorsSelected;


	
	void FixUpRedirectors();
};
 