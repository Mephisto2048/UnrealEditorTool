// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetCheckCommandlet.h"

#include "FindReplaceTextureWidget.h"
#include "AssetCheck/MeshConsoleCheckWidget.h"


int32 UAssetCheckCommandlet::Main(const FString& CmdLineParams)
{
	UE_LOG(LogTemp, Display, TEXT("hello commandlet"));

	ScanAllAssets(); //加载资产

	FString PackageName;
	FString PathModeString;
	FParse::Value(*CmdLineParams, TEXT("package="), PackageName);
	FParse::Value(*CmdLineParams, TEXT("pathmode="), PathModeString);
	{
		// 加载 Editor Utility Blueprint
		UClass* BlueprintClass = LoadObject<UClass>(nullptr, TEXT("/AssetCheckTool/parts/AssetCheck/EUW_MeshConsoleCheck.EUW_MeshConsoleCheck_C"));
		if (!BlueprintClass)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load Editor Utility Blueprint!"));
			return 1;
		}

		UWorld* World = LoadWorld("/AssetCheckTool/parts/DefaultSettings/BackGroundCheck/LoadMap");
		if(!World) UE_LOG(LogTemp, Error, TEXT("Failed to get world"));
		UMeshConsoleCheckWidget* Widget = CreateWidget<UMeshConsoleCheckWidget>(World, BlueprintClass);

		if(!Widget)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to CreateWidget"));
			return 1;
		}
		bool PathMode = PathModeString.ToBool();

		Widget->MeshConsoleCheck_InBP(PackageName,PathMode);
	}

	/*{
		UMaterial* Material = LoadObject<UMaterial>(nullptr, *PackageName);
		if (!Material)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load Material"));
			return 1;
		}
		if(Material->TwoSided)
		{UE_LOG(LogTemp, Display, TEXT("true"));}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("false"));
		}
	}*/
	return 0;
}

void UAssetCheckCommandlet::ScanAllAssets()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FString> ContentPaths;
	ContentPaths.Add(TEXT("/Game"));
	AssetRegistryModule.Get().ScanPathsSynchronous(ContentPaths, true);
}

UWorld* UAssetCheckCommandlet::LoadWorld(const FString& MapName)
{
	FString Filename;
	if (FPackageName::TryConvertLongPackageNameToFilename(MapName, Filename))
	{
		UPackage* Package = LoadPackage(nullptr, *Filename, LOAD_EditorOnly);
		if (Package)
		{
			UWorld* World = UWorld::FindWorldInPackage(Package);
			if (World)
			{
				// 创建新的世界上下文
				FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Editor);
				WorldContext.SetCurrentWorld(World);
				World->AddToRoot(); // 防止被垃圾回收
				World->InitWorld(UWorld::InitializationValues().AllowAudioPlayback(false));
				//World->GetWorldSettings()->PostEditChange();
				World->UpdateWorldComponents(true, false);
				return World;
			}
		}
	}
	return nullptr;
}