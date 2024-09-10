// Copyright Epic Games, Inc. All Rights Reserved.

#include "MfstManager.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include"DebugUtil.h"
#include "EditorAssetLibrary.h"
#include "LevelEditor.h"
#include "ObjectTools.h"
#include "Selection.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "CustomUICommand/MfstUICommands.h"
#define LOCTEXT_NAMESPACE "FMfstManagerModule"

void FMfstManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogTemp, Warning, TEXT("StartupModule"));
	InitContentBrowserMenuExtension();
	
	FMfstUICommands::Register();
	InitCustomUICommands();
	
	InitLevelEditorExtension();
	InitCustomSelectionEvent();
}

void FMfstManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FMfstManagerModule::InitContentBrowserMenuExtension()
{
	//加载contentbrowser模块
	FContentBrowserModule& ContentBrowserModule =
	FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	//获得PathViewContextMenuExtenders，它是个存储delegate的Tarray
	 TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtender =
		ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	//定义我们自己的delegate
	FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	
	//绑定我的函数到我的委托上
	CustomCBMenuDelegate.BindRaw(this,&FMfstManagerModule::CustomCBMenuExtender);
	
	//添加我们自己的delegate
	ContentBrowserModuleMenuExtender.Add(CustomCBMenuDelegate);
}

TSharedRef<FExtender> FMfstManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	
	TSharedRef<FExtender> MenuExtender (new FExtender());

	if(SelectedPaths.Num()>0)
	{
		MenuExtender->AddMenuExtension(FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this,&FMfstManagerModule::AddCBMenuEntry)
			);
		FolderPathsSelected = SelectedPaths;
	}
	return MenuExtender;
}

void FMfstManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	//定义menu entry的详细信息
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Unused Assets")),
		FText::FromString(TEXT("Safely Delete Unused Assets")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this,&FMfstManagerModule::OnDeleteUnusedAssetButtonClicked)
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Empty Folders")),
		FText::FromString(TEXT("Safely Delete Empty Folders")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this,&FMfstManagerModule::OnDeleteEmptyFolderButtonClicked)
	);
}

void FMfstManagerModule::OnDeleteUnusedAssetButtonClicked()
{
	FixUpRedirectors();
	if(FolderPathsSelected.Num()>1)
	{
		DebugUtil::MessageDialog(TEXT("Only One Folder You Can Choose"));
		return;
	}
	DebugUtil::Print(TEXT("Selected Folder: ")+FolderPathsSelected[0]);

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	if(AssetsPathNames.Num() == 0)
	{
		DebugUtil::MessageDialog(TEXT("No Asset Found"));
	}

	EAppReturnType::Type Result =
		DebugUtil::MessageDialog_yesno(FString::FromInt(AssetsPathNames.Num())+TEXT(" Assets need to be check.would you want to continue?"));
	if(Result == EAppReturnType::No) return;

	 TArray<FAssetData> UnusedAssetsData;

	for(const FString& AssetPathName : AssetsPathNames)
	{
		if(AssetPathName.Contains(TEXT("Developers")) ||
			AssetPathName.Contains(TEXT("Collections"))
			//do :external文件夹
			||AssetPathName.Contains(TEXT("__ExternalActors__"))||
			AssetPathName.Contains(TEXT("__ExternalObjects__"))
		)
		{
			continue;
		}
		//todo: 检查AssetPathName路径下的资产是否存在

		//当前资产被谁引用的 数组
		TArray<FString> AssetReferencers =
			UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);

		if(AssetReferencers.Num() == 0)
		{
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			UnusedAssetsData.Add(UnusedAssetData);
		}
	}

	if(UnusedAssetsData.Num() > 0)
	{
		ObjectTools::DeleteAssets(UnusedAssetsData);
	}
	else
	{
		DebugUtil::MessageDialog(TEXT("No Unused Asset Found"));
	}
}

void FMfstManagerModule::OnDeleteEmptyFolderButtonClicked()
{
	FixUpRedirectors();
	DebugUtil::Print(TEXT("OnDeleteEmptyFolderButtonClicked"));
	//数组存储了资产和目录
	TArray<FString>FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0],true,true);
	uint32 Count = 0;

	FString EmptyFolderPathsName;
	TArray<FString> EmptyFoldersPathsArray;

	for(const FString& FolderPath:FolderPathsArray)
	{
		if(FolderPath.Contains(TEXT("Developers")) ||
			FolderPath.Contains(TEXT("Collections"))
			||FolderPath.Contains(TEXT("__ExternalActors__"))||
			FolderPath.Contains(TEXT("__ExternalObjects__"))
		)
		{
			continue;
		}

		//如果 不是文件夹 是资产就跳过
		if(!UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) continue;

		//如果文件夹里有资产就跳过
		if(UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath)) continue;

		EmptyFolderPathsName.Append(FolderPath);
		EmptyFolderPathsName.Append(TEXT("\n"));
		EmptyFoldersPathsArray.Add(FolderPath);
	}
	if(EmptyFoldersPathsArray.Num() == 0)
	{
		DebugUtil::MessageDialog(TEXT("No empty folder found"));
		return;
	}
	EAppReturnType::Type Result =
		DebugUtil::MessageDialog_yesno(TEXT("Empty folders found in\n") + EmptyFolderPathsName + TEXT("\nWould you want to continue?"));
	if(Result == EAppReturnType::No) return;

	for(const FString& EmptyFolderPath:EmptyFoldersPathsArray)
	{
		if (UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath))
		{
			Count++;
		}
		else
		{
			DebugUtil::Print(TEXT("Failed to delete") + EmptyFolderPath);
		}
	}

	if(Count>0)
	{
		DebugUtil::ShowNotify(FString::FromInt(Count)+TEXT(" Folders deleted successfully"));
	}
}

void FMfstManagerModule::InitLevelEditorExtension()
{
	FLevelEditorModule& LevelEditorModule =
	FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	TSharedRef<FUICommandList> ExistingLevelCommands = LevelEditorModule.GetGlobalLevelEditorActions();
	ExistingLevelCommands->Append(CustomUICommandList.ToSharedRef());
	
	TArray<FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors>& LevelEditorMenuExtender =
	   LevelEditorModule.GetAllLevelViewportContextMenuExtenders();

	FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors CustomLEMenuDelegate;
	
	CustomLEMenuDelegate.BindRaw(this,&FMfstManagerModule::CustomLEMenuExtender);
	
	LevelEditorMenuExtender.Add(CustomLEMenuDelegate);
}

TSharedRef<FExtender> FMfstManagerModule::CustomLEMenuExtender(const TSharedRef<FUICommandList> UICommandList,
	const TArray<AActor*> SelectedActors)
{
	TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender());
	if(SelectedActors.Num()>0)
	{
		MenuExtender->AddMenuExtension(FName("ActorOptions"),
			EExtensionHook::Before,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this,&FMfstManagerModule::AddLEMenuEntry)
			);
	}
	return MenuExtender;
}

void FMfstManagerModule::AddLEMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Lock Actors")),
		FText::FromString(TEXT("Lock Actors!")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this,&FMfstManagerModule::OnLockActorButtonClicked)
	);
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Unlock Actors")),
		FText::FromString(TEXT("Unlock Actors!")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this,&FMfstManagerModule::OnUnlockActorButtonClicked)
	);
}

void FMfstManagerModule::OnLockActorButtonClicked()
{
	if(!GetEditorActorSubSystem()) return;
	TArray<AActor*> ActorsSelected = EditorActorSubsystem->GetSelectedLevelActors();
	if(ActorsSelected.Num() == 0)
	{
		DebugUtil::ShowNotify(TEXT("No actors selected"));
		return;
	}
	FString LockedActorName;
	for(AActor* ActorSelected:ActorsSelected)
	{
		LockActorSelection(ActorSelected);
		EditorActorSubsystem->SetActorSelectionState(ActorSelected,false);
		LockedActorName.Append(ActorSelected->GetActorLabel());
		LockedActorName.Append(TEXT("\n"));
	}
	DebugUtil::ShowNotify(LockedActorName+TEXT(" has been locked"));
}

void FMfstManagerModule::OnUnlockActorButtonClicked()
{
	if(!GetEditorActorSubSystem()) return;
	TArray<AActor*> AllActors = EditorActorSubsystem->GetAllLevelActors();
	TArray<AActor*> AllLockedActors;
	
	for(AActor* Actor:AllActors)
	{
		if(IsActorSelectionLocked(Actor))
		{
			AllLockedActors.Add(Actor);
		}
	}
	if(AllLockedActors.Num()==0)
	{
		DebugUtil::ShowNotify(TEXT("No Actors Locked"));
		return;
	}
	
	FString UnlockedActorName;
	for(AActor* LockedActor:AllLockedActors)
	{
		UnlockActorSelection(LockedActor);
		UnlockedActorName.Append(LockedActor->GetActorLabel());
		UnlockedActorName.Append(TEXT("\n"));
	}
	DebugUtil::MessageDialog(UnlockedActorName+TEXT(" has been unlocked"));
}

void FMfstManagerModule::InitCustomSelectionEvent()
{
	USelection* UserSelection = GEditor->GetSelectedActors();
	UserSelection->SelectObjectEvent.AddRaw(this,&FMfstManagerModule::OnActorSelected);
}

void FMfstManagerModule::OnActorSelected(UObject* SelectedObject)
{
	if(!GetEditorActorSubSystem()) return;
	if(AActor* SelectedActor = Cast<AActor>(SelectedObject))
	{
		if(IsActorSelectionLocked(SelectedActor))
		{
			EditorActorSubsystem->SetActorSelectionState(SelectedActor,false);
		}
	}
}

void FMfstManagerModule::LockActorSelection(AActor* InActor)
{
	if(!InActor) return;
	if(!InActor->ActorHasTag(FName("Locked")))
	{
		InActor->Tags.Add(FName("Locked"));
	}
}

void FMfstManagerModule::UnlockActorSelection(AActor* InActor)
{
	if(!InActor) return;
	if(InActor->ActorHasTag(FName("Locked")))
	{
		InActor->Tags.Remove(FName("Locked"));
	}
}

bool FMfstManagerModule::IsActorSelectionLocked(AActor* InActor)
{
	if(!InActor) return false;
	return InActor->ActorHasTag(FName("Locked"));
}

void FMfstManagerModule::InitCustomUICommands()
{
	CustomUICommandList = MakeShareable(new FUICommandList());

	CustomUICommandList->MapAction(
		FMfstUICommands::Get().LockActorSelection,
		FExecuteAction::CreateRaw(this,&FMfstManagerModule::OnSelectionLockHotKeyPress)
	);

	CustomUICommandList->MapAction(
		FMfstUICommands::Get().UnlockActorSelection,
		FExecuteAction::CreateRaw(this,&FMfstManagerModule::OnSelectionUnlockHotKeyPress)
	);
}

void FMfstManagerModule::OnSelectionLockHotKeyPress()
{
	OnLockActorButtonClicked();
}

void FMfstManagerModule::OnSelectionUnlockHotKeyPress()
{
	OnLockActorButtonClicked();
}

bool FMfstManagerModule::GetEditorActorSubSystem()
{
	if(!EditorActorSubsystem)
	{
		EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}
	return EditorActorSubsystem != nullptr;
}

void FMfstManagerModule::FixUpRedirectors()
{
	//存储需要修复的重定向器
	TArray<UObjectRedirector*> RedirectorsToFixArray;
	
	//加载AssetRegistry模块:使用FModuleManager加载并获取FAssetRegistryModule模块的引用，这个模块用于访问资产注册表
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	
	//设置过滤器:创建一个FARFilter对象，设置过滤器参数以递归地搜索/Game目录下的所有资产，并筛选出类名为ObjectRedirector的资产
	FARFilter Filter;
	Filter.bRecursivePaths = true;    //是否递归搜索路径,也就是是否搜索子路径
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassPaths.Emplace("ObjectRedirector");
	
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

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMfstManagerModule, MfstManager)









