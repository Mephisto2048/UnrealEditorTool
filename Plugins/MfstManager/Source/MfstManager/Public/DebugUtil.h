#pragma once
#include"Widgets/Notifications/SNotificationList.h"    //定义notify
#include"Framework/Notifications/NotificationManager.h"    //将notify添加到编辑器
void Print(const FString& Message)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Cyan, Message);
	}
}

void ShowNotify(const FString& String)
{
	FNotificationInfo NotifyInfo(FText::FromString(String));
	NotifyInfo.SubText = FText::FromString(TEXT("this is subtext"));
	FSlateNotificationManager::Get().AddNotification(NotifyInfo);
}