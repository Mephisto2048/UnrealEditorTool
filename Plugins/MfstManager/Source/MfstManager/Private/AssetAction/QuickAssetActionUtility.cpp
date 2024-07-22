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
		/* ��ǰ�ʲ��Ѿ�����ȷ��ǰ׺�� */
		if(OldName.StartsWith(*PrefixFound))
		{
			Print(OldName + TEXT(" has current prefix"));
			continue;
		}
		/* �����ǰ�ʲ��ǲ���ʵ�� */
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
	//�洢��Ҫ�޸����ض�����
	TArray<UObjectRedirector*> RedirectorsToFixArray;
	//����AssetRegistryģ��:ʹ��FModuleManager���ز���ȡFAssetRegistryModuleģ������ã����ģ�����ڷ����ʲ�ע���
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	//���ù�����:����һ��FARFilter�������ù����������Եݹ������/GameĿ¼�µ������ʲ�����ɸѡ������ΪObjectRedirector���ʲ�
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassNames.Emplace("ObjectRedirector");
	//��ȡ�ʲ�����:ʹ�ù��������ʲ�ע����л�ȡ�����������ʲ����ݣ��洢��OutRedirectors������
	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);
	//ɸѡ������ض�����:����OutRedirectors�����е�ÿ��FAssetData����
	//���Խ��ʲ�����ת��ΪUObjectRedirector���ͣ�����ɹ�����ӵ�RedirectorsToFixArray������
	for (const FAssetData& RedirectorData : OutRedirectors)
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}
	//����AssetToolsģ�鲢�޸��ض�����:
	//ʹ��FModuleManager���ز���ȡFAssetToolsModuleģ������á�
	//����FixupReferencers����������RedirectorsToFixArray���飬�޸���Щ�ض�����������
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
}

