// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetAction/FixupRedirectorsWidget.h"

#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "FixUpRedirectorInAssetFolder"


void UFixupRedirectorsWidget::BatchFixupAsset(TArray<UObject*> Assets)
{
    TArray<UObjectRedirector*> redirectorsToFix;
    TArray<FAssetData> foundRedirectorsData;

    for (UObject* Asset : Assets)
    {
        FAssetData AssetData(Asset);
        foundRedirectorsData.Add(AssetData);
    }
    TArray<FString> FilterList;

    // 收集有效的重定向器数据
    TArray<FAssetData> ValidRedirectorsData;
    for (const FAssetData& FoundRedirectorsData : foundRedirectorsData)
    {
        FString PackageStr = FoundRedirectorsData.PackageName.ToString();

        if (FoundRedirectorsData.IsValid())
        {
            ValidRedirectorsData.Add(FoundRedirectorsData);
        }
    }

    // 批处理修复重定向器
    int32 TotalRedirectors = ValidRedirectorsData.Num();
    int32 ProcessedRedirectors = 0;

    GWarn->BeginSlowTask(FText::FromString("Fixing up asset redirectors..."), true);

    for (int32 i = 0; i < TotalRedirectors; i++)
    {
        TArray<UObjectRedirector*> BatchRedirectors;
        TArray<FAssetData>::ElementType Item = ValidRedirectorsData[i];
        // 在这里加载资产
        UObject* Asset = Item.GetAsset();
        if (Asset)
        {
            // 确保资产是重定向器
            if (UObjectRedirector* Redirector = Cast<UObjectRedirector>(Asset))
            {
                BatchRedirectors.Add(Redirector);
            }
        }
        
        FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
        AssetToolsModule.Get().FixupReferencers(BatchRedirectors, false, ERedirectFixupMode::LeaveFixedUpRedirectors);

        ProcessedRedirectors += BatchRedirectors.Num();
        GWarn->UpdateProgress(ProcessedRedirectors, TotalRedirectors);
    }

    GWarn->EndSlowTask();
}

void UFixupRedirectorsWidget::BatchFixupRedirectors(TArray<FString> SelectedPaths,TArray<FString> SelectedPackages)
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	// Form a filter from the paths
	FARFilter Filter;
	Filter.bRecursivePaths = true;

	Filter.PackagePaths.Reserve(SelectedPaths.Num());
	for (const FString& Path : SelectedPaths)
	{
		Filter.PackagePaths.Emplace(*Path);
	}

	if (!SelectedPaths.IsEmpty())
	{
		Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());
	}
	
	// Query for a list of assets in the selected paths
	TArray<FAssetData> AssetList;
	AssetRegistryModule.Get().GetAssets(Filter, AssetList);

	Filter.Clear();

	Filter.PackageNames.Reserve(SelectedPackages.Num());
	for (const FString& PackageName : SelectedPackages)
	{
		Filter.PackageNames.Emplace(*PackageName);
	}

	if (!SelectedPackages.IsEmpty())
	{
		Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());
	}

	AssetRegistryModule.Get().GetAssets(Filter, AssetList);

	if (AssetList.Num() == 0)
	{
		return;
	}

	FScopedSlowTask SlowTask(3, LOCTEXT("FixupRedirectorsSlowTask", "Fixing up redirectors"));
	SlowTask.MakeDialog(true);

	SlowTask.EnterProgressFrame(1, LOCTEXT("FixupRedirectors_LoadAssets", "Loading Assets..."));
	TArray<UObject*> Objects;
	AssetViewUtils::FLoadAssetsSettings Settings{
		.bFollowRedirectors = false,
		.bAllowCancel = true,
	};
	AssetViewUtils::ELoadAssetsResult Result = AssetViewUtils::LoadAssetsIfNeeded(AssetList, Objects, Settings);
	if (Result != AssetViewUtils::ELoadAssetsResult::Cancelled && !SlowTask.ShouldCancel())
	{
		TArray<UObjectRedirector*> Redirectors;
		for (UObject* Object : Objects)
		{
			if (UObjectRedirector* Redirector = Cast<UObjectRedirector>(Object))
			{
				Redirectors.Add(Redirector);
			}
		}

		SlowTask.EnterProgressFrame(1, LOCTEXT("FixupRedirectors_FixupReferencers", "Fixing up referencers..."));
		// Load the asset tools module
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		AssetToolsModule.Get().FixupReferencers(Redirectors, true, ERedirectFixupMode::PromptForDeletingRedirectors);
	}
}

void UFixupRedirectorsWidget::GetSelectedPathPackage(TArray<FString>& SelectedPaths, TArray<FString>& SelectedPackages)
{
	IContentBrowserSingleton& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
	
	TArray<FString> PathViewPaths;
	ContentBrowser.GetSelectedPathViewFolders(PathViewPaths);
	TArray<FString> FolderPaths;
	ContentBrowser.GetSelectedFolders(FolderPaths);

	//TArray<FString> SelectedPaths;
	SelectedPaths.Append(PathViewPaths);
	SelectedPaths.Append(FolderPaths);

	for (FString& SelectedPath :SelectedPaths)
	{
		SelectedPath.RemoveFromStart(TEXT("/All"));
	}
}
