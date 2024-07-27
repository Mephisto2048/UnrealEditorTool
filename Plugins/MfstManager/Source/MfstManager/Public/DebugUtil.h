#pragma once
#include"Widgets/Notifications/SNotificationList.h"    //定义notify
#include"Framework/Notifications/NotificationManager.h"    //将notify添加到编辑器

namespace DebugUtil
{
	static void Print(const FString& Message)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Cyan, Message);
		}
	}

	static void MessageDialog(const FString& Message)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
	}
	
	static void ShowNotify(const FString& String)
	{
		FNotificationInfo NotifyInfo(FText::FromString(String));
		NotifyInfo.SubText = FText::FromString(TEXT("this is subtext"));
		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}
}
