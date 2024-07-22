// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetAction/QuickAssetActionUtility.h"

#include "AssetSelection.h"
#include "AssetToolsModule.h"
#include"DebugUtil.h"
#include"EditorUtilityLibrary.h"
#include"EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"

void UQuickAssetActionUtility::TestPrint()
{
	Print(TEXT("debug_print"));
}

void UQuickAssetActionUtility::TestMessageDialog()
{
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Debug_MessageDialog")));
}

void UQuickAssetActionUtility::TestNotification()
{
	FNotificationInfo NotifyInfo(FText::FromString(TEXT("Debug notify")));
	NotifyInfo.SubText = FText::FromString(TEXT("this is subtext"));
	FSlateNotificationManager::Get().AddNotification(NotifyInfo);
}
void UQuickAssetActionUtility::AutoPrefix()
{
	TArray<UObject*>SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;
	for(UObject* SelectedObject:SelectedObjects)
	{
		if(!SelectedObject)continue;
		
		FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());
		
		if(!PrefixFound)
		{
			Print(SelectedObject->GetName()+TEXT("`s class is not found"));
			continue;
		}
		else if(PrefixFound->IsEmpty())
		{
			Print(SelectedObject->GetName()+TEXT("`s class is empty"));
			continue;
		}
		
		FString OldName = SelectedObject->GetName();
		/* 当前资产已经有正确的前缀了 */
		if(OldName.StartsWith(*PrefixFound))
		{
			Print(OldName + TEXT(" has current prefix"));
			continue;
		}
		/* 如果当前资产是材质实例 */
		if(SelectedObject->IsA<UMaterialInstanceConstant>())
		{
			OldName.RemoveFromEnd(TEXT("_Inst"));
			OldName.RemoveFromStart(TEXT("M_"));
		}
		const FString NewName = *PrefixFound + OldName;

		UEditorUtilityLibrary::RenameAsset(SelectedObject,NewName);

		Counter++;
	}
	
	if(Counter > 0)
	{
		ShowNotify(TEXT("Successfully rename" + FString::FromInt(Counter)  ));
	}

}

void UQuickAssetActionUtility::DeleteUnusedAssets()
{
	FixUpRedirectors();
	TArray<FAssetData> SelectedAssetsData= UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetsData;
	for(const FAssetData& SelectedAssetData:SelectedAssetsData)
	{
		TArray<FString> Ref = UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAssetData.GetObjectPathString());
		if(Ref.IsEmpty())
		{
			UnusedAssetsData.Add(SelectedAssetData);
		}
	}

	if(UnusedAssetsData.IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("No Unused Assets Found")));
		return;
	}

	int32 DeleteCount = ObjectTools::DeleteAssets(UnusedAssetsData);
	if(DeleteCount == 0)return;
	ShowNotify(TEXT("Successfully Delete " + FString::FromInt(DeleteCount) + " Unused Assets"));
}

void UQuickAssetActionUtility::FixUpRedirectors()
{
	//存储需要修复的重定向器
	TArray<UObjectRedirector*> RedirectorsToFixArray;
	//加载AssetRegistry模块:使用FModuleManager加载并获取FAssetRegistryModule模块的引用，这个模块用于访问资产注册表
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	//设置过滤器:创建一个FARFilter对象，设置过滤器参数以递归地搜索/Game目录下的所有资产，并筛选出类名为ObjectRedirector的资产
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassNames.Emplace("ObjectRedirector");
	//获取资产数据:使用过滤器从资产注册表中获取符合条件的资产数据，存储在OutRedirectors数组中
	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);
	//筛选和添加重定向器:遍历OutRedirectors数组中的每个FAssetData对象。
	//尝试将资产数据转换为UObjectRedirector类型，如果成功则添加到RedirectorsToFixArray数组中
	for (const FAssetData& RedirectorData : OutRedirectors)
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}
	//加载AssetTools模块并修复重定向器:
	//使用FModuleManager加载并获取FAssetToolsModule模块的引用。
	//调用FixupReferencers方法，传入RedirectorsToFixArray数组，修复这些重定向器的引用
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
}

