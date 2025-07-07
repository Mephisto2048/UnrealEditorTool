// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetCheckToolCommands.h"

#include "IContentBrowserSingleton.h"
#include "AssetBatch/AssetTagWidget.h"
#include "AssetCheck/MeshConsoleCheckWidget.h"

#undef LOCTEXT_NAMESPACE

#define LOCTEXT_NAMESPACE "FAssetCheckToolModule"

void FAssetCheckToolCommands::RegisterCommands()
{
	//UI_COMMAND(PluginAction, "AssetCheckTool", "Execute AssetCheckTool action", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginAction, "AssetCheckTool", "Execute AssetCheckTool action", EUserInterfaceActionType::Button, FInputChord());

}

//打开widget
void OpenWidget(const FString& Path)
{
	FSoftObjectPath EditorPath = FSoftObjectPath(Path);
	UObject* MyAssetEditorPath = EditorPath.TryLoad();
	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	UEditorUtilityWidgetBlueprint* EditorWidget = Cast<UEditorUtilityWidgetBlueprint>(MyAssetEditorPath);
	EditorUtilitySubsystem->SpawnAndRegisterTab(EditorWidget);
	TACraft::SendTrackingMessageV2(TEXT("AssetCheckTool"), TEXT("1.0"), TEXT("MH"), TEXT("Timi J5"), false);
}
void OpenAssetTagWidget(const FString& Path)
{
	FSoftObjectPath EditorPath = FSoftObjectPath(Path);
	UObject* MyAssetEditorPath = EditorPath.TryLoad();
	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	UEditorUtilityWidgetBlueprint* EditorWidgetBP = Cast<UEditorUtilityWidgetBlueprint>(MyAssetEditorPath);
	UEditorUtilityWidget* EditorWidget = EditorUtilitySubsystem->SpawnAndRegisterTab(EditorWidgetBP);

	UAssetTagWidget* AssetTagWidget = Cast<UAssetTagWidget>(EditorWidget);
	AssetTagWidget->CheckSelectedAssetsTag_InBP();
	TACraft::SendTrackingMessageV2(TEXT("AssetCheckTool"), TEXT("1.0"), TEXT("MH"), TEXT("Timi J5"), false);
}

void OpenReCheckWidget(const FString& Path)
{
	FSoftObjectPath EditorPath = FSoftObjectPath(Path);
	UObject* MyAssetEditorPath = EditorPath.TryLoad();
	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	UEditorUtilityWidgetBlueprint* EditorWidgetBP = Cast<UEditorUtilityWidgetBlueprint>(MyAssetEditorPath);
	UEditorUtilityWidget* EditorWidget = EditorUtilitySubsystem->SpawnAndRegisterTab(EditorWidgetBP);

	UMeshConsoleCheckWidget* AssetTagWidget = Cast<UMeshConsoleCheckWidget>(EditorWidget);
	AssetTagWidget->MeshAutoCheck_InBP();
	TACraft::SendTrackingMessageV2(TEXT("AssetCheckTool"), TEXT("1.0"), TEXT("MH"), TEXT("Timi J5"), false);
}

// 创建菜单扩展实例
// 声明方法


/*
TSharedRef<FExtender> FAssetCheckToolCommands::OnExtendContentBrowserPathSelectionMenu(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> Extender(new FExtender());

	Extender->AddMenuExtension(
		"NewFolder", // 插入到 "新建文件夹" 部分
		EExtensionHook::After, // 插入到 "新建文件夹" 的下方
		nullptr,
		FMenuExtensionDelegate::CreateLambda([SelectedPaths](FMenuBuilder& MenuBuilder)
		{
			// 创建一个名为 "AssetCheckTool" 的子菜单
			MenuBuilder.AddSubMenu(
				LOCTEXT("AssetCheckToolMenuLabel", "创建规范目录"),
				LOCTEXT("AssetCheckToolMenuTooltip", "Tools for asset checking and management."),

				FNewMenuDelegate::CreateLambda([SelectedPaths](FMenuBuilder& SubMenuBuilder)
				{
					// 在 "AssetCheckTool" 子菜单下添加 "Create Folders in Selected Path"
					CreateCreateFoldersContentBrowserPathMenu(SubMenuBuilder, SelectedPaths);
				}),
				false,
				FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.NewFolderIcon")
			);
		}));

	return Extender;
}*/


TSharedRef<FExtender> FAssetCheckToolCommands::OnExtendContentBrowserPathSelectionMenu(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> Extender(new FExtender());

	Extender->AddMenuExtension(
		"NewFolder", // 插入到 "新建文件夹" 部分
		EExtensionHook::After, // 插入到 "新建文件夹" 的下方
		nullptr,
		FMenuExtensionDelegate::CreateLambda([SelectedPaths](FMenuBuilder& MenuBuilder)
		{
			// 直接在菜单中添加一个菜单项
			MenuBuilder.AddMenuEntry(
				LOCTEXT("CreateFoldersMenuLabel", "创建规范目录"),
				LOCTEXT("CreateFoldersMenuTooltip", "辅助用户快速创建符合项目规范的文件夹."),

#if ENGINE_MAJOR_VERSION >= 5
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.NewFolderIcon"),
#else
				FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.NewFolderIcon"),
#endif

				FUIAction(FExecuteAction::CreateLambda([SelectedPaths]()
				{
					OpenWidget("/AssetCheckTool/parts/AssetBatch/EUW_CreateStandardFolders.EUW_CreateStandardFolders");
					TACraft::SendTrackingMessageV2(TEXT("AssetCheckTool"), TEXT("1.0"), TEXT("MH"), TEXT("Timi J5"), false);
				}))
			);
		}));
		Extender->AddMenuExtension(
		"NewFolder", // 插入到 "新建文件夹" 部分
		EExtensionHook::After, // 插入到 "新建文件夹" 的下方
		nullptr,
		FMenuExtensionDelegate::CreateLambda([SelectedPaths](FMenuBuilder& MenuBuilder)
		{

			// 直接在菜单中添加一个菜单项
			MenuBuilder.AddMenuEntry(
				LOCTEXT("RenameAssetsMenuLabel", "批量重命名"),
				LOCTEXT("RenameAssetsMenuTooltip", "批量替换字符实现重命名或移动文件操作."),

#if ENGINE_MAJOR_VERSION >= 5
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.NewFolderIcon"),
#else
				FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.NewFolderIcon"),
#endif
				FUIAction(FExecuteAction::CreateLambda([SelectedPaths]()
				{
					OpenWidget("/AssetCheckTool/parts/AssetBatch/EUW_AssetRenameTool.EUW_AssetRenameTool");
					TACraft::SendTrackingMessageV2(TEXT("AssetCheckTool"), TEXT("1.0"), TEXT("MH"), TEXT("Timi J5"), false);
				}))
			);
		}));
	return Extender;
}


//对资产右键的菜单
void FAssetCheckToolCommands::AddAssetContextMenuExtension()
{
        if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu"))
        {
                // 查找或添加菜单部分
                FToolMenuSection& Section = Menu->AddSection("AssetCheckToolActions", LOCTEXT("AssetCheckToolMenuHeading", "AssetCheckTool"));
                Section.AddMenuEntry(
                        "RenameAssetsMenuLabelForAsset",
                        LOCTEXT("RenameAssetsMenuLabel", "批量重命名"),
                        LOCTEXT("RenameAssetsMenuTooltip", "批量替换字符实现重命名或移动文件操作."),

#if ENGINE_MAJOR_VERSION >= 5
                        FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.NewFolderIcon"),
#else
                                FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.NewFolderIcon"),
#endif
                        FUIAction(FExecuteAction::CreateLambda([]()
                        {
                                // 执行自定义操作
                                OpenWidget("/AssetCheckTool/parts/AssetBatch/EUW_AssetRenameTool.EUW_AssetRenameTool");
                                TACraft::SendTrackingMessageV2(TEXT("AssetCheckTool"), TEXT("1.0"), TEXT("MH"), TEXT("Timi J5"), false);
                        }))
                );

                Section.AddMenuEntry(
                        "TagAssetsMenuLabelForAsset",
                        LOCTEXT("TagAssetsMenuLabel", "批量标签设置"),
                        LOCTEXT("TagAssetsMenuTooltip", "批量查看及设置资产标签操作."),

#if ENGINE_MAJOR_VERSION >= 5
                        FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.NewFolderIcon"),
#else
                                FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.NewFolderIcon"),
#endif
                        FUIAction(FExecuteAction::CreateLambda([]()
                        {
                                // 执行自定义操作
                                OpenAssetTagWidget("/AssetCheckTool/parts/AssetBatch/EUW_AssetTagTool.EUW_AssetTagTool");
                                TACraft::SendTrackingMessageV2(TEXT("AssetCheckTool"), TEXT("1.0"), TEXT("MH"), TEXT("Timi J5"), false);
                        }))
                );

        		Section.AddMenuEntry(
						"ReCheckMenuLabelForAsset",
						LOCTEXT("TagAssetsMenuLabel", "资产规范检测"),
						LOCTEXT("TagAssetsMenuTooltip", "批量检测选中资产是否符合规范."),

#if ENGINE_MAJOR_VERSION >= 5
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.NewFolderIcon"),
#else
						FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.NewFolderIcon"),
#endif
						FUIAction(FExecuteAction::CreateLambda([]()
						{
								// 执行自定义操作
								OpenReCheckWidget("/AssetCheckTool/parts/AssetCheck/EUW_MeshConsoleCheck.EUW_MeshConsoleCheck");
								TACraft::SendTrackingMessageV2(TEXT("AssetCheckTool"), TEXT("1.0"), TEXT("MH"), TEXT("Timi J5"), false);
						}))
				);
        }
        else
        {
                UE_LOG(LogTemp, Error, TEXT("Failed to extend menu: ContentBrowser.AssetContextMenu"));
        }
}

//单按钮模式
void FAssetCheckToolCommands::CreateFoldersInSelectedPath(const TArray<FString>& SelectedPaths)
{
	// 遍历选定的路径
	for (const FString& Path : SelectedPaths)
	{
		// 构造新文件夹的完整路径
		FString NewFolderPath = FPaths::Combine(Path, TEXT("NewFolder"));

		// 检查路径是否已经存在
		if (!IFileManager::Get().DirectoryExists(*NewFolderPath))
		{
			// 创建新文件夹
			if (IFileManager::Get().MakeDirectory(*NewFolderPath, true))
			{
				UE_LOG(LogTemp, Log, TEXT("Created folder: %s"), *NewFolderPath);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to create folder: %s"), *NewFolderPath);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Folder already exists: %s"), *NewFolderPath);
		}
	}
}


// submenu模式
void FAssetCheckToolCommands::CreateCreateFoldersContentBrowserPathMenu(FMenuBuilder& MenuBuilder, TArray<FString> SelectedPaths)
{
    MenuBuilder.AddMenuEntry
    (
	    LOCTEXT("CreateFoldersPathTabTitle", "Create Material, Mesh, Texture"),
		LOCTEXT("CreateFoldersPathTooltipText", "Creates Material, Mesh, and Texture folders in the selected paths."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([SelectedPaths]()
        {
            FString FormattedSelectedPaths;
            for (int32 i = 0; i < SelectedPaths.Num(); ++i)
            {
                FormattedSelectedPaths.Append(SelectedPaths[i]);
                if (i < SelectedPaths.Num() - 1)
                {
                    FormattedSelectedPaths.Append(LINE_TERMINATOR);
                }
            }
            FFormatNamedArguments Args;
            Args.Add(TEXT("Paths"), FText::FromString(FormattedSelectedPaths));
            const EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, FText::Format(LOCTEXT("CreateFoldersConfirmation", "Are you sure you want to create Material, Mesh, and Texture folders in the following paths?\n\n{Paths}"), Args));
            if (Result == EAppReturnType::Yes)
            {
            	TArray<FString> Folders;
                for (const FString& Folder : SelectedPaths)
                {
                    FString MaterialFolderPath = FPaths::Combine(Folder, TEXT("Material"));
                    FString MeshFolderPath = FPaths::Combine(Folder, TEXT("Mesh"));
                    FString TextureFolderPath = FPaths::Combine(Folder, TEXT("Texture"));
                	Folders.Add(MaterialFolderPath);
                	Folders.Add(MeshFolderPath);
                	Folders.Add(TextureFolderPath);
                }
            	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
            	for(const FString Folder : Folders)
            	{

						UEditorAssetLibrary::MakeDirectory(*Folder);

            	}

            }
        }))
    );
}


#undef LOCTEXT_NAMESPACE
