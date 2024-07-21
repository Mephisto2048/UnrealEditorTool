// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "QuickAssetActionUtility.generated.h"

/**
 * 
 */
UCLASS()
class MFSTMANAGER_API UQuickAssetActionUtility : public UAssetActionUtility
{
	GENERATED_BODY()
public:
	UFUNCTION(CallInEditor)
	void TestPrint();
	UFUNCTION(CallInEditor)
	void TestMessageDialog();
	UFUNCTION(CallInEditor)
	void TestNotification();

	UFUNCTION(CallInEditor)
	void AutoPrefix();
private:
	TMap<UClass*, FString>PrefixMap =
	{
		{UBlueprint::StaticClass(), TEXT("BP_")}
	};
};
