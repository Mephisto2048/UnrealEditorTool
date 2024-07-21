// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetAction/QuickAssetActionUtility.h"
#include"DebugUtil.h"
#include"EditorUtilityLibrary.h"
#include"EditorAssetLibrary.h"
#include"Widgets/Notifications/SNotificationList.h"    //定义notify
#include"Framework/Notifications/NotificationManager.h"    //将notify添加到编辑器
void UQuickAssetActionUtility::TestPrint()
{
	Print();
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
}

