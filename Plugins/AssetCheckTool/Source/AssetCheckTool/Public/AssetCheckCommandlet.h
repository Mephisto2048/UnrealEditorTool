// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "AssetRegistry/AssetRegistryModule.h"  // 资产注册表模块
#include "Modules/ModuleManager.h" // FModuleManager
#include "AssetCheckCommandlet.generated.h"

/**
 *
 */
UCLASS()
class ASSETCHECKTOOL_API UAssetCheckCommandlet : public UCommandlet
{
	GENERATED_BODY()

	virtual int32 Main(const FString& CmdLineParams) override;
public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ScanAllAssets"), Category = "AssetCheckTool")
	static void ScanAllAssets();
private:
	UWorld* LoadWorld(const FString& MapName);
};
