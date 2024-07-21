// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetAction/QuickAssetActionUtility.h"
#include"DebugUtil.h"
#include"EditorUtilityLibrary.h"
#include"EditorAssetLibrary.h"

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
		
		if(OldName.StartsWith(*PrefixFound))
		{
			Print(OldName + TEXT(" has current prefix"));
			continue;
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

