// Copyright Epic Games, Inc. All Rights Reserved.
// -*- coding: utf-8 -*-

#include "AssetCheckToolBPLibrary.h"

//#include <FoliageType_InstancedCrossStaticMesh.h>
//#include <Engine/CrossStaticMeshActor.h>

#include "AssetViewUtils.h"
#include "AssetCheckToolConfig.h"
#include "ClothingAsset.h"
#include "EditorAssetLibrary.h"
#include "JsonObjectConverter.h"
#include "LandscapeStreamingProxy.h"
#include "LODUtilities.h"
#include "LODUtilities.h"
#include "MoveRule.h"

#include "Engine/SkeletalMeshLODSettings.h"
#include "Engine/SkinnedAssetAsyncCompileUtils.h"

#include "Rendering/SkeletalMeshModel.h"


TArray<FMoveAction> UAssetCheckToolBPLibrary::MoveAction;

#define LOCTEXT_NAMESPACE "AssetCheckTool"

UAssetCheckToolBPLibrary::UAssetCheckToolBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}


/*
void UMyCustomUMGWidget::NativeConstruct()
{
	SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(STextBlock)
		.Text(FText::FromString("Hello, Slate!"))
	];
}

void UMyEditorUtilityWidget::NativeConstruct()
{
	if (CustomUMGWidgetClass)
	{
		UMyCustomUMGWidget* TestWidget = CreateWidget<UMyCustomUMGWidget>(GetWorld(), CustomUMGWidgetClass);
		if (TestWidget)
		{
			TestWidget->AddToViewport();
		}
	}
}
*/

void UAssetCheckToolBPLibrary::GetAutoComputeLODScreenSize(UStaticMesh* StaticMesh,   bool& AutoComputeLODScreenSize)
{

	AutoComputeLODScreenSize = StaticMesh->bAutoComputeLODScreenSize;


}

void UAssetCheckToolBPLibrary::GetCustomizedCollision(UStaticMesh* StaticMesh, bool& CustomizedCollision, FString& CollisionMesh)
{
	CustomizedCollision = StaticMesh->bCustomizedCollision;
	CollisionMesh = "";
	if (IsValid(StaticMesh->ComplexCollisionMesh))
		CollisionMesh = StaticMesh->ComplexCollisionMesh->GetName();

}


void UAssetCheckToolBPLibrary::SetLODScreenSize(UStaticMesh* StaticMesh, int32 LODIndex,float Value)
{
	FStaticMeshSourceModel* SrcModel = &StaticMesh->GetSourceModel(LODIndex);
	SrcModel->ScreenSize = Value;
	// Now rebuild the mesh with it's LODs
	StaticMesh->CreateBodySetup();
	StaticMesh->Build();
	StaticMesh->PostEditChange();
	// 获取 StaticMesh 所在的包
	UPackage* Package = StaticMesh->GetOutermost();

	// 标记包已被修改
	Package->MarkPackageDirty();
}

void UAssetCheckToolBPLibrary::GetMaterialSlotName(UStaticMesh* StaticMesh, int32 MatIndex, FString& SlotName)
{
	TArray<FStaticMaterial> FoundStaticMaterials = StaticMesh->GetStaticMaterials();

	if (FoundStaticMaterials.IsValidIndex(MatIndex))
	{
		SlotName = FoundStaticMaterials[MatIndex].MaterialSlotName.ToString();
	}
	else
	{
		SlotName = FString("Invalid Material Index");
	}
}

void UAssetCheckToolBPLibrary::SetMaximumSize(UTexture* Texture,  float value)
{
	Texture->MaxTextureSize = value;

}

int32 UAssetCheckToolBPLibrary::GetTextureResourceSize(UTexture* Texture)
{
	FTexturePlatformData** PlatformDataPtr = Texture->GetRunningPlatformData();
	const FStreamableRenderResourceState SRRState = Texture->GetStreamableResourceState();
	const int32 ActualMipBias = SRRState.IsValid() ? (SRRState.ResidentFirstLODIdx() + SRRState.AssetLODBias) : Texture->GetCachedLODBias();
	const int64 ResourceSize = PlatformDataPtr && *PlatformDataPtr ? (*PlatformDataPtr)->GetPayloadSize(ActualMipBias) : Texture->GetResourceSizeBytes(EResourceSizeMode::Exclusive);
	return FMath::DivideAndRoundNearest<int64>(ResourceSize, (int64)1024);
}



int32 UAssetCheckToolBPLibrary::GetTextureMaxLODSize(const UTexture* Texture)
{
	if (!Texture)
	{
		return -1; // 返回-1表示纹理无效
	}

	// 获取全局纹理LOD设置，注意这里使用const指针
	const UTextureLODSettings* LODSettings = GetDefault<UTextureLODSettings>();
	if (!LODSettings)
	{
		return -1; // 返回-1表示无法获取LOD设置
	}

	// 确保纹理的LOD组索引是有效的
	if (Texture->LODGroup < 0 || Texture->LODGroup >= TEXTUREGROUP_MAX)
	{
		return -1; // 返回-1表示LOD组索引无效
	}

	// 获取纹理的LOD组
	//const FTextureLODGroup& LODGroup = LODSettings->GetTextureLODGroup(Texture->LODGroup);
	const FTextureLODGroup& LODGroup = LODSettings->TextureLODGroups[Texture->LODGroup];

	// 返回MaxLODSize
	return LODGroup.MaxLODSize;
}

int UAssetCheckToolBPLibrary::GetMaxInGameSizeWithSpecialGroup(UTexture* Texture, TEnumAsByte<TextureGroup> OptionalGroup)
{
	UTexture2D* Texture2D = Cast<UTexture2D>(Texture);
	const uint32 SurfaceWidth = (uint32)Texture->GetSurfaceWidth();
	const uint32 SurfaceHeight = (uint32)Texture->GetSurfaceHeight();

	const UTextureLODSettings* LODSettings = GetDefault<UTextureLODSettings>();
	if (!LODSettings)
	{
		return -1; // 无法获取LOD设置
	}

	// 使用TextureGroup枚举进行转换
	//const FTextureLODGroup& LODGroup = LODSettings->GetTextureLODGroup(OptionalGroup);
	const FTextureLODGroup& LODGroup = LODSettings->TextureLODGroups[OptionalGroup];

	int32 MaxResMipBias = Texture2D ? (Texture2D->GetNumMips() - Texture2D->GetNumMipsAllowed(true)) : Texture->GetCachedLODBias();
	MaxResMipBias = LODGroup.MaxLODSize; // 使用输入的TextureGroup的LOD设置

	const uint32 MaxInGameWidth = FMath::Max<uint32>(SurfaceWidth >> MaxResMipBias, 1);
	const uint32 MaxInGameHeight = FMath::Max<uint32>(SurfaceHeight >> MaxResMipBias, 1);
	int MaxInGameSize = FMath::Max<uint32>(MaxInGameWidth, MaxInGameHeight);
	return MaxInGameSize;
}

int UAssetCheckToolBPLibrary::GetMaxInGameSize(UTexture* Texture)
{
	if (!Texture)
	{
		return -1; // 返回-1表示纹理无效
	}
	// 确保纹理已加载
	Texture->ConditionalPostLoad();
	Texture->UpdateResource();


	UTexture2D* Texture2D = Cast<UTexture2D>(Texture);
	const uint32 SurfaceWidth = (uint32)Texture->GetSurfaceWidth();
	const uint32 SurfaceHeight = (uint32)Texture->GetSurfaceHeight();
	// In game max bias and dimensions
	const int32 MaxResMipBias = Texture2D ? (Texture2D->GetNumMips() - Texture2D->GetNumMipsAllowed(true)) : Texture->GetCachedLODBias();
	const uint32 MaxInGameWidth = FMath::Max<uint32>(SurfaceWidth >> MaxResMipBias, 1);
	const uint32 MaxInGameHeight = FMath::Max<uint32>(SurfaceHeight >> MaxResMipBias, 1);
	int MaxInGameSize = FMath::Max<uint32>(MaxInGameWidth, MaxInGameHeight);
	return MaxInGameSize;
}

bool UAssetCheckToolBPLibrary::GetMaxInGameSizev2(UTexture* Texture, int32& OutWidth, int32& OutHeight)
{
	if (!Texture)
	{
		OutWidth = -1;
		OutHeight = -1;
		return false; // 返回 false 表示纹理无效
	}

	// 确保纹理已加载
	Texture->ConditionalPostLoad();
	Texture->UpdateResource();

	UTexture2D* Texture2D = Cast<UTexture2D>(Texture);
	const uint32 SurfaceWidth = (uint32)Texture->GetSurfaceWidth();
	const uint32 SurfaceHeight = (uint32)Texture->GetSurfaceHeight();

	// 计算游戏中的最大 Mip Bias 和尺寸
	const int32 MaxResMipBias = Texture2D ? (Texture2D->GetNumMips() - Texture2D->GetNumMipsAllowed(true)) : Texture->GetCachedLODBias();
	const uint32 MaxInGameWidth = FMath::Max<uint32>(SurfaceWidth >> MaxResMipBias, 1);
	const uint32 MaxInGameHeight = FMath::Max<uint32>(SurfaceHeight >> MaxResMipBias, 1);

	// 设置输出参数
	OutWidth = static_cast<int32>(MaxInGameWidth);
	OutHeight = static_cast<int32>(MaxInGameHeight);

	return true; // 返回 true 表示成功
}

int32 UAssetCheckToolBPLibrary::GetMipMapNum(UTexture* Texture)
{
    if (!Texture)
    {
        UE_LOG(LogTemp, Error, TEXT("Texture is null."));
        return -1;
    }
	// 确保纹理已完全加载
	// 确保纹理已加载
	Texture->ConditionalPostLoad();
	Texture->UpdateResource();


    // 强制同步加载贴图
    Texture->WaitForStreaming();

    // 检查贴图类型并获取 Mipmap 数量
    if (UTexture2D* Texture2D = Cast<UTexture2D>(Texture))
    {
        return Texture2D->GetNumMips();
    }
    else if (UTextureCube* TextureCube = Cast<UTextureCube>(Texture))
    {
        return TextureCube->GetNumMips();
    }
    else if (UTexture2DArray* Texture2DArray = Cast<UTexture2DArray>(Texture))
    {
        return Texture2DArray->GetNumMips();
    }

    return -1;
}
void UAssetCheckToolBPLibrary::SetAutoComputeLODScreenSize(UStaticMesh* StaticMesh, bool Enable)
{
	StaticMesh->bAutoComputeLODScreenSize = Enable;
	// Now rebuild the mesh with it's LODs
	StaticMesh->CreateBodySetup();
	StaticMesh->Build();
	StaticMesh->PostEditChange();
	// 获取 StaticMesh 所在的包
	UPackage* Package = StaticMesh->GetOutermost();

	// 标记包已被修改
	Package->MarkPackageDirty();
}




void UAssetCheckToolBPLibrary::GetMaterialIndexWithScetion(UStaticMesh* StaticMesh, int32 InLODIndex, int32 SectionIndex, int32& MaterialIndex)
{
	FMeshSectionInfo Info = StaticMesh->GetSectionInfoMap().Get(InLODIndex, SectionIndex);

	MaterialIndex = Info.MaterialIndex;

}

class UMaterialInstance;
UMaterialInterface* UAssetCheckToolBPLibrary::GetMaterialParent(UMaterialInstance* MaterialInstance)
{
	if (MaterialInstance)
	{
		return MaterialInstance->Parent;
	}
	return nullptr;

}

int32 UAssetCheckToolBPLibrary::GetModelTriangleCount(UStaticMesh* StaticMesh, int32 LODIndex)
{
	if (StaticMesh && LODIndex >= 0 && LODIndex < StaticMesh->GetRenderData()->LODResources.Num())
	{
		FStaticMeshLODResources& LODModel = StaticMesh->GetRenderData()->LODResources[LODIndex];
		return LODModel.GetNumTriangles();
	}
	return 0;

}

int32 UAssetCheckToolBPLibrary::GetSkeletalMeshTriangleCount(USkeletalMesh* SkeletalMesh, int32 LODIndex)
{
	if (SkeletalMesh && LODIndex >= 0 && SkeletalMesh->GetResourceForRendering() && LODIndex < SkeletalMesh->GetResourceForRendering()->LODRenderData.Num())
	{
		FSkeletalMeshLODRenderData& LODModel = SkeletalMesh->GetResourceForRendering()->LODRenderData[LODIndex];
		return LODModel.GetTotalFaces();
	}
	return 0;
}

TArray<int32> UAssetCheckToolBPLibrary::GetAllLODTriangleCounts(UStaticMesh* StaticMesh)
{
	TArray<int32> TriangleCounts;

	if (StaticMesh)
	{	bool bShouldUnload = false;

		if (!StaticMesh->GetRenderData())
		{
			bShouldUnload = true;
		}

		int32 LODCount = StaticMesh->GetRenderData()->LODResources.Num();
		for (int32 LODIndex = 0; LODIndex < LODCount; LODIndex++)
		{
			FStaticMeshLODResources& LODModel = StaticMesh->GetRenderData()->LODResources[LODIndex];
			TriangleCounts.Add(LODModel.GetNumTriangles());
		}

		if (bShouldUnload)
		{
			StaticMesh->ConditionalBeginDestroy();
		}
	}
	return TriangleCounts;
}

TArray<int32> UAssetCheckToolBPLibrary::GetAllLODTriangleCountsSK(USkeletalMesh* SkeletalMesh)
{
	TArray<int32> TriangleCounts;

	if (SkeletalMesh)
	{
		// 获取骨骼网格渲染数据
		FSkeletalMeshRenderData* RenderData = SkeletalMesh->GetResourceForRendering();
		if (RenderData)
		{
			// 遍历所有LOD
			const int32 LODCount = RenderData->LODRenderData.Num();
			for (int32 LODIndex = 0; LODIndex < LODCount; ++LODIndex)
			{
				// 获取当前LOD的渲染数据
				const FSkeletalMeshLODRenderData& LODRenderData = RenderData->LODRenderData[LODIndex];
				int32 TotalTriangles = 0;

				// 累加所有渲染分段的三角形数量
				for (const FSkelMeshRenderSection& Section : LODRenderData.RenderSections)
				{
					TotalTriangles += Section.NumTriangles;
				}

				TriangleCounts.Add(TotalTriangles);
			}
		}
	}
	return TriangleCounts;
}

void  UAssetCheckToolBPLibrary::SetFoliageCullDistance(UFoliageType_InstancedStaticMesh* FoliageTypeInput, float MinCullDistance, float MaxCullDistance)
{
	if (FoliageTypeInput)
	{
		FoliageTypeInput->CullDistance.Min = MinCullDistance;
		FoliageTypeInput->CullDistance.Max = MaxCullDistance;


		// 获取 StaticMesh 所在的包
		//UPackage* Package = FoliageTypeInput->GetSource();
		UPackage* Package = FoliageTypeInput->GetOutermost();
		// 标记包已被修改
		Package->MarkPackageDirty();

	}
}


void  UAssetCheckToolBPLibrary::GetFoliageCullDistance(UFoliageType_InstancedStaticMesh* FoliageTypeInput, float& MinCullDistance, float& MaxCullDistance)
{
	if (FoliageTypeInput)
	{
		MinCullDistance = FoliageTypeInput->CullDistance.Min;
		MaxCullDistance = FoliageTypeInput->CullDistance.Max;
	}
}

void UAssetCheckToolBPLibrary::GetGenerateLightmapUVs(UStaticMesh* StaticMesh, bool& GenerateLightmapUVs)
{
	GenerateLightmapUVs = false;

	if (StaticMesh != nullptr)
	{
		FStaticMeshSourceModel* SrcModel = &StaticMesh->GetSourceModel(0);
		if (SrcModel->BuildSettings.bGenerateLightmapUVs)
		{
				GenerateLightmapUVs = true;


		}
	}
}

void UAssetCheckToolBPLibrary::GetSelectedFoliageTypes(TArray<UObject*>& FoliageTypeFiles, TArray<AInstancedFoliageActor*>& InstancedFoliageActors, TArray<UFoliageType*>& FoliageTypes)
#if ENGINE_MAJOR_VERSION >= 5
{
    FoliageTypeFiles.Empty();
    InstancedFoliageActors.Empty();
    FoliageTypes.Empty();

    FEditorModeTools& ModeTools = GLevelEditorModeTools();

    if (ModeTools.IsModeActive(FBuiltinEditorModes::EM_Foliage))
    {
        //FEdModeFoliage* FoliageEditMode = static_cast<FEdModeFoliage*>(ModeTools.GetActiveMode(FBuiltinEditorModes::EM_Foliage));
    	FEdModeFoliage* FoliageEditMode = (FEdModeFoliage*)ModeTools.GetActiveMode(FBuiltinEditorModes::EM_Foliage);


        if (FoliageEditMode != nullptr)
        {
            for (TActorIterator<AInstancedFoliageActor> ActorItr(GWorld); ActorItr; ++ActorItr)
            {
                AInstancedFoliageActor* IFA = *ActorItr;
                if (IFA != nullptr && IFA->HasSelectedInstances())
                {
                    InstancedFoliageActors.Add(IFA);

                    TMap<UFoliageType*, FFoliageInfo*> SelectedInstanceFoliageTypes = IFA->GetSelectedInstancesFoliageType();
                    for (auto& FoliageTypePair : SelectedInstanceFoliageTypes)
                    {
                        UFoliageType* FoliageType = FoliageTypePair.Key;
                        FFoliageInfo* FoliageInfo = FoliageTypePair.Value;

                        if (FoliageType != nullptr && FoliageInfo != nullptr)
                        {
                            UObject* SourceObject = FoliageType->GetSource();
                            if (SourceObject != nullptr)
                            {
                                UStaticMesh* StaticMesh = Cast<UStaticMesh>(SourceObject);
                                if (StaticMesh != nullptr)
                                {
                                    FoliageTypeFiles.Add(StaticMesh);
                                    FoliageTypes.Add(FoliageType);
                                }
                            	/*UCrossStaticMesh* CrossStaticMesh = Cast<UCrossStaticMesh >(SourceObject);
                            	if (CrossStaticMesh != nullptr)// Arashi Cross
                            	{
                            		FoliageTypeFiles.Add(CrossStaticMesh);
                            		FoliageTypes.Add(FoliageType);
                            	}*/
                            }
                        }
                    }
                }
            }
        }

    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Foliage edit mode is not active."));
    }

}
#else
{
		FoliageTypeFiles.Empty();
		InstancedFoliageActors.Empty();
		FoliageTypes.Empty();
		FEdModeFoliage* FoliageEditMode = nullptr;

		FoliageEditMode = static_cast<FEdModeFoliage*>(GLevelEditorModeTools().GetActiveMode(FBuiltinEditorModes::EM_Foliage));

		if (FoliageEditMode != nullptr)
		{
			for (TActorIterator<AInstancedFoliageActor> ActorItr(GWorld); ActorItr; ++ActorItr)
			{
				AInstancedFoliageActor* IFA = *ActorItr;
				if (IFA != nullptr && IFA->HasSelectedInstances())
				{
					// Add the AInstancedFoliageActor to the output array
					InstancedFoliageActors.Add(IFA);

					TMap<UFoliageType*, FFoliageInfo*> SelectedInstanceFoliageTypes = IFA->GetSelectedInstancesFoliageType();

					for (auto& FoliageTypePair : SelectedInstanceFoliageTypes)
					{
						UFoliageType* FoliageType = FoliageTypePair.Key;
						FFoliageInfo* FoliageInfo = FoliageTypePair.Value;

						if (FoliageType != nullptr && FoliageInfo != nullptr)
						{
							UObject* SourceObject = FoliageType->GetSource();
							if (SourceObject != nullptr)
							{
								UStaticMesh* StaticMesh = Cast<UStaticMesh>(SourceObject);
								if (StaticMesh != nullptr)
								{
									FoliageTypeFiles.Add(StaticMesh);
									FoliageTypes.Add(FoliageType);
								}
							}
						}
					}
				}
			}
		}
}
#endif




void UAssetCheckToolBPLibrary::AddTagToActors(const FName& Tag, bool& bSuccess, const TArray<AActor*>& Actors)
{
	bSuccess = false;

	// If no actors are provided, do nothing
	if (Actors.Num() == 0)
	{
		return;
	}

	// Iterate over provided actors
	for (AActor* Actor : Actors)
	{
		if (Actor)
		{
			// Add tag to each actor
			Actor->Tags.AddUnique(Tag);
			bSuccess = true;
		}
	}
}

void UAssetCheckToolBPLibrary::AddTagToActor(const FName& Tag, bool& bSuccess, AActor* Actor)
{
	bSuccess = false;

	// If no actor is provided, do nothing
	if (!Actor)
	{
		return;
	}

	// Add tag to the actor
	Actor->Tags.AddUnique(Tag);
	bSuccess = true;
}

void UAssetCheckToolBPLibrary::SetActorTag(const FName& Tag, int32 Index, AActor* Actor)
{
	// If no actor is provided, do nothing
	if (!Actor)
	{
		return;
	}

	// Resize the actor's tags array if necessary
	if (Index >= Actor->Tags.Num())
	{
		Actor->Tags.SetNum(Index + 1);
	}

	// Add tag to the actor
	Actor->Tags[Index] = Tag;
}

void UAssetCheckToolBPLibrary::GetLevel( AActor* Actor,FString& LevelName)
{
	if (Actor)
	{

		if (const ULevel* Level = Actor->GetLevel())
		{
			LevelName = Level->GetOutermost()->GetName();
			// Do something with LevelName
		}
	}
}

void UAssetCheckToolBPLibrary::RemoveTagFromActors(const FName& Tag, bool& bSuccess, const TArray<AActor*>& Actors)
{
	bSuccess = false;

	// If no actors are provided, do nothing
	if (Actors.Num() == 0)
	{
		return;
	}

	// Iterate over provided actors
	for (AActor* Actor : Actors)
	{
		if (Actor)
		{
			// Remove tag from each actor
			Actor->Tags.Remove(Tag);
			bSuccess = true;
		}
	}
}

FString UAssetCheckToolBPLibrary::GetStaticMeshSourceFilePath(UStaticMesh* StaticMesh)
{
	if (StaticMesh && StaticMesh->AssetImportData)
	{
		if (const UAssetImportData* ImportData = StaticMesh->AssetImportData)
		{
			FString SourceFilePath = ImportData->GetFirstFilename();
			return SourceFilePath;
		}
	}

	return FString();
}

FString UAssetCheckToolBPLibrary::GetSkeletalMeshSourceFilePath(USkeletalMesh* SkeletalMesh)
{
	if (SkeletalMesh)
	{
		// 使用 GetAssetImportData() 方法获取 AssetImportData
		if (const UAssetImportData* ImportData = SkeletalMesh->GetAssetImportData())
		{
			FString SourceFilePath = ImportData->GetFirstFilename();
			return SourceFilePath;
		}
	}

	return FString();
}

void UAssetCheckToolBPLibrary::FindStaticMeshWithSameSource(UStaticMesh* Mesh, const FString& AdditionalSearchPath, int32 mode, const FString& FilterString, TArray<UStaticMesh*>& OutMeshes)
{
	if (Mesh)
	{
		FString TargetSourcePath;
		if (mode == 0)
		{
			TargetSourcePath = FPaths::GetPath(Mesh->AssetImportData->GetFirstFilename());
		}
		if (mode == 1)
		{
			TargetSourcePath = Mesh->AssetImportData->GetFirstFilename();
		}

		// 获取 AssetRegistry 模块
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

		// 创建过滤器，只获取 StaticMesh 类型的资源
		FARFilter Filter;
#if ENGINE_MAJOR_VERSION >= 5
		Filter.ClassPaths.Add(FTopLevelAssetPath(UStaticMesh::StaticClass()->GetPathName()));
#else
		Filter.ClassNames.Add(UStaticMesh::StaticClass()->GetFName());
#endif
		// 获取所有的 StaticMesh 资源
		TArray<FAssetData> AssetDataArray;
		AssetRegistryModule.Get().GetAssets(Filter, AssetDataArray);

		// 获取输入StaticMesh所在文件夹路径
		FString InputMeshFolderPath = FPaths::GetPath(Mesh->GetPathName());

		// 过滤只保留指定文件夹及其子文件夹下的资源
		TArray<FAssetData> FilteredAssetDataArray;

		if (AdditionalSearchPath.IsEmpty())
		{
			for (const FAssetData& AssetData : AssetDataArray)
			{
#if ENGINE_MAJOR_VERSION >= 5
				FString AssetPath = AssetData.GetObjectPathString();
#else
				FString AssetPath = AssetData.ObjectPath.ToString();
#endif

				FString AssetFolderPath = FPaths::GetPath(AssetPath);

				if (AssetFolderPath == InputMeshFolderPath && !AssetPath.EndsWith(FilterString))
				{
					FilteredAssetDataArray.Add(AssetData);
				}
			}
		}
		else
		{
			FString AdditionalSearchPathWithSlash = AdditionalSearchPath + TEXT("/");

			for (const FAssetData& AssetData : AssetDataArray)
			{
#if ENGINE_MAJOR_VERSION >= 5
				FString AssetPath = AssetData.GetObjectPathString();
#else
				FString AssetPath = AssetData.ObjectPath.ToString();
#endif
				FString AssetFolderPath = FPaths::GetPath(AssetPath);

				if ((AssetFolderPath == InputMeshFolderPath || AssetFolderPath.StartsWith(AdditionalSearchPathWithSlash)) && !AssetPath.EndsWith(FilterString))
				{
					FilteredAssetDataArray.Add(AssetData);
				}
			}
		}

		/*
		// 遍历指定文件夹下的 StaticMesh
		if (mode == 2) //文件夹模式
		{
			for (const FAssetData& AssetData : FilteredAssetDataArray)
			{
				FString SourcePath = FPaths::GetPath(Cast<UStaticMesh>(AssetData.GetAsset())->AssetImportData->GetFirstFilename());

				// 比较源文件路径
				if (SourcePath == TargetSourcePath)
				{
					// 找到与目标 StaticMesh 相同文件夹的 StaticMesh
					UStaticMesh* StaticMesh = Cast<UStaticMesh>(AssetData.GetAsset());
					if (StaticMesh)
					{
						OutMeshes.Add(StaticMesh);
					}
				}
			}
		}
		if (mode == 3)//文件名模式
		{
			for (const FAssetData& AssetData : FilteredAssetDataArray)
			{
				FString SourcePath = Cast<UStaticMesh>(AssetData.GetAsset())->AssetImportData->GetFirstFilename();

				// 比较源文件路径
				if (SourcePath == TargetSourcePath)
				{
					// 找到与目标 StaticMesh 相同文件夹的 StaticMesh
					UStaticMesh* StaticMesh = Cast<UStaticMesh>(AssetData.GetAsset());
					if (StaticMesh)
					{
						OutMeshes.Add(StaticMesh);
					}
				}
			}
		}
		*/
		if (mode == 0)//boudingbox模式
		{
			for (const FAssetData& AssetData : FilteredAssetDataArray)
			{
				// 比较输入模型和目标模型的bounding box
				UStaticMesh* StaticMesh = Cast<UStaticMesh>(AssetData.GetAsset());
				FBox InputMeshBoundingBox = Mesh->GetBoundingBox();
				FBox TargetMeshBoundingBox = StaticMesh->GetBoundingBox();

				if (InputMeshBoundingBox.GetSize() == TargetMeshBoundingBox.GetSize())
				{
					OutMeshes.Add(StaticMesh);
				}
			}

		}
		if (mode == 1)//顶点数模式
		{
			int32 meshverticenum = Mesh->GetNumVertices(0);
			for (const FAssetData& AssetData : FilteredAssetDataArray)
			{
				// 比较输入模型和目标模型的bounding box
				UStaticMesh* StaticMesh = Cast<UStaticMesh>(AssetData.GetAsset());

				if (meshverticenum == StaticMesh->GetNumVertices(0))
				{
					OutMeshes.Add(StaticMesh);
				}
			}

		}
		if (mode == 2)//顶点数和boudingbox
		{
			int32 meshverticenum = Mesh->GetNumVertices(0);
			for (const FAssetData& AssetData : FilteredAssetDataArray)
			{
				// 比较输入模型和目标模型的bounding box
				UStaticMesh* StaticMesh = Cast<UStaticMesh>(AssetData.GetAsset());
				FBox InputMeshBoundingBox = Mesh->GetBoundingBox();
				FBox TargetMeshBoundingBox = StaticMesh->GetBoundingBox();

				if (meshverticenum == StaticMesh->GetNumVertices(0) && InputMeshBoundingBox.GetSize() == TargetMeshBoundingBox.GetSize())
				{
					OutMeshes.Add(StaticMesh);
				}
			}

		}


	}
}

void UAssetCheckToolBPLibrary::ReimportStaticMesh(UStaticMesh* StaticMesh)
{
	if (StaticMesh)
	{
		TArray<UObject*> AssetsToReimport;
		AssetsToReimport.Add(StaticMesh);

		FReimportManager::Instance()->ValidateAllSourceFileAndReimport(AssetsToReimport, true, false);
	}
}

void UAssetCheckToolBPLibrary::GetInViewportActor(TArray<AActor*>& OutActorList, float MaxDistance)
{
	// Get the current viewport client

	if (FEditorViewportClient* ViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient()))
	{
		// Get the current scene view
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
			ViewportClient->Viewport,
			ViewportClient->GetScene(),
			ViewportClient->EngineShowFlags)
			.SetRealtimeUpdate(ViewportClient->IsRealtime()));


		if (const FSceneView* SceneView = ViewportClient->CalcSceneView(&ViewFamily))
		{
			// Iterate over all actors in the world
			for (TActorIterator<AActor> ActorItr(ViewportClient->GetWorld()); ActorItr; ++ActorItr)
			{
				// Check if the actor is within the maximum distance and visible in the current scene view
				FVector Origin, BoxExtent;
				ActorItr->GetActorBounds(true, Origin, BoxExtent);
				if (FVector::Dist(Origin, SceneView->ViewLocation) <= MaxDistance &&
					SceneView->ViewFrustum.IntersectBox(Origin, BoxExtent))
				{
					// If the actor is within the maximum distance and visible, add it to the output list
					OutActorList.Add(*ActorItr);
				}
			}
		}
	}
}



template <typename T>
void UAssetCheckToolBPLibrary::SortNumListTemplate(const TArray<T>& NumList, TArray<T>& SortedList, TArray<int32>& IDList, bool bReverse)
{
	// Pair each element with its index
	TArray<TPair<int32, T>> Pairs;
	for (int32 i = 0; i < NumList.Num(); i++)
	{
		Pairs.Emplace(i, NumList[i]);
	}

	// Sort pairs by value
	Pairs.Sort([bReverse](const TPair<int32, T>& A, const TPair<int32, T>& B)
		{
			return bReverse ? A.Value > B.Value : A.Value < B.Value;
		});

	// Split pairs back into separate arrays
	SortedList.Empty();
	IDList.Empty();
	for (const TPair<int32, T>& Pair : Pairs)
	{
		SortedList.Add(Pair.Value);
		IDList.Add(Pair.Key);
	}
}

void UAssetCheckToolBPLibrary::SortNumListInt(const TArray<int32>& NumList, TArray<int32>& SortedList, TArray<int32>& IDList, bool bReverse)
{
	UAssetCheckToolBPLibrary::SortNumListTemplate(NumList, SortedList, IDList,bReverse);
}

void UAssetCheckToolBPLibrary::SortNumListFloat(const TArray<float>& NumList, TArray<float>& SortedList, TArray<int32>& IDList, bool bReverse)
{
	UAssetCheckToolBPLibrary::SortNumListTemplate(NumList, SortedList, IDList,bReverse);
}

void  UAssetCheckToolBPLibrary::FindInstancedStaticMeshComponents(UStaticMesh* StaticMesh, AInstancedFoliageActor* InstancedFoliageActor, TArray<UInstancedStaticMeshComponent*>& ResultComponents, bool& bFound)
{
	bFound = false;
	ResultComponents.Empty();

	if (StaticMesh && InstancedFoliageActor)
	{
		TArray<UActorComponent*> Components;
		InstancedFoliageActor->GetComponents(Components);

		for (UActorComponent* Component : Components)
		{
			UInstancedStaticMeshComponent* InstancedStaticMeshComponent = Cast<UInstancedStaticMeshComponent>(Component);
			if (InstancedStaticMeshComponent && InstancedStaticMeshComponent->GetStaticMesh() == StaticMesh)
			{
				ResultComponents.Add(InstancedStaticMeshComponent);
				bFound = true;
			}
		}
	}
}

bool UAssetCheckToolBPLibrary::SetCollisionProfile(UStaticMesh* StaticMesh, const FString& CollisionProfileName)
{
	if (StaticMesh)
	{
		// Check if the CollisionProfileName is already set to the desired value
		if (StaticMesh->GetBodySetup()->DefaultInstance.GetCollisionProfileName().ToString() == CollisionProfileName)
		{
			// Return false indicating that the profile was not set
			return false;
		}

		// Create BodySetup if it doesn't exist
		StaticMesh->CreateBodySetup();

		// Get the BodySetup of the StaticMesh

		if (UBodySetup* BodySetup = StaticMesh->GetBodySetup())
		{
			// Convert FString to FName
			FName ProfileName = FName(*CollisionProfileName);

			// Set Collision Profile
			BodySetup->DefaultInstance.SetCollisionProfileName(ProfileName);
			BodySetup->bSupportUVsAndFaceRemap = StaticMesh->bSupportPhysicalMaterialMasks;
			BodySetup->InvalidatePhysicsData();
			BodySetup->CreatePhysicsMeshes();

			// Get the package where the StaticMesh is located
			UPackage* Package = StaticMesh->GetOutermost();

			// Mark the package as modified
			Package->MarkPackageDirty();
		}

		// Return true indicating that the profile was set
		return true;
	}

	// Return false indicating that the profile was not set
	return false;
}

bool UAssetCheckToolBPLibrary::SaveArrayText(FString SaveDirector, FString FileName, TArray<FString> SaveText, bool AllowOverWriting )
{
	//Set complete file path.
	SaveDirector += "\\";
	SaveDirector += FileName;

	if (!AllowOverWriting)
	{
		if (FPlatformFileManager::Get().GetPlatformFile(*SaveDirector))
		{
			return false;
		}
	}

	FString FinalString = "";
	for (FString& Each : SaveText)
	{
		FinalString += Each;
		FinalString += LINE_TERMINATOR;
	}
	return FFileHelper::SaveStringToFile(FinalString, *SaveDirector);
}

bool UAssetCheckToolBPLibrary::SaveArrayTextPlus(FString SaveDirector, FString FileName, TArray<FString> SaveText, bool AllowOverWriting, FString Delimiter)
{
	// Set complete file path.
	SaveDirector += "\\";
	SaveDirector += FileName;

	if (!AllowOverWriting)
	{
		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*SaveDirector))
		{
			return false;
		}
	}
	FString FinalString = "";
	for (FString& Each : SaveText)
	{
		// Check if the string contains a comma or tab, and enclose in quotes if it does.
		//if (Each.Contains(TEXT(",")))
		//{
			// 转义逗号
		//	Each = Each.Replace(TEXT(","), TEXT(";"));
		//}

		FinalString += Each;
		FinalString += LINE_TERMINATOR;
	}

	// Save the file without BOM
	return FFileHelper::SaveStringToFile(FinalString, *SaveDirector, FFileHelper::EEncodingOptions::ForceUnicode);
}

TArray<int32> UAssetCheckToolBPLibrary::CheckUVSetEmpty(UStaticMesh* StaticMesh, int32 LODIndex)
{
	TArray<int32> ResultList;

	// 获取指定 LOD 级别的 Static Mesh
	const FStaticMeshLODResources& LODResource = StaticMesh->GetLODForExport(LODIndex);

	// 遍历每个 UVSet
	for (uint32 UVSetIndex = 0; UVSetIndex < static_cast<uint32>(LODResource.GetNumTexCoords()); UVSetIndex++)
	{
		// 获取 UVSet 的顶点数据
		const FStaticMeshVertexBuffer& VertexBuffer = LODResource.VertexBuffers.StaticMeshVertexBuffer;

		bool IsEmpty = true;
		#if ENGINE_MAJOR_VERSION >= 5
		for (uint32 VertIndex = 0; VertIndex < VertexBuffer.GetNumVertices(); ++VertIndex)
		{
			// 使用 FVector2f 替代 FVector2D
			FVector2f uv = VertexBuffer.GetVertexUV(VertIndex, UVSetIndex);
			FVector2f uvempty(0.f, 1.f);
			FVector2f uvempty2(0.f, 0.f);
			FVector2f uvempty3(1.f, 0.f);
			FVector2f uvempty4(1.f, 1.f);

			// 定义容差
			const float Tolerance = 0.01f;

			// 自定义比较函数，判断两个 UV 坐标是否在容差范围内相等
			auto IsNearlyEqual = [Tolerance](const FVector2f& A, const FVector2f& B) -> bool {
				return FMath::Abs(A.X - B.X) <= Tolerance && FMath::Abs(A.Y - B.Y) <= Tolerance;
			};

			// 检查 UV 坐标是否在容差范围内等于 uvempty 或 uvempty2
			if (!IsNearlyEqual(uv, uvempty) && !IsNearlyEqual(uv, uvempty2) && !IsNearlyEqual(uv, uvempty3) && !IsNearlyEqual(uv, uvempty4))
			{
				IsEmpty = false;
				break;
			}

		}
		#else
		for (uint32 VertIndex = 0; VertIndex < VertexBuffer.GetNumVertices(); ++VertIndex)
		{
			FVector2D uv = VertexBuffer.GetVertexUV(VertIndex, UVSetIndex);
			FVector2D uvempty(0.f, 1.f);
			FVector2D uvempty2(0.f, 0.f);
			if (uv != uvempty && uv != uvempty2)
			{
				IsEmpty = false;
				break;
			}
		}
		#endif

		// 如果UVSet中有任何非零的UV坐标，那么UVSet就不是空的
		ResultList.Add(IsEmpty ? 1 : 0);
	}

	return ResultList;
}


TArray<int32> UAssetCheckToolBPLibrary::CheckUVSetEmptySkeletal(USkeletalMesh* SkeletalMesh, int32 LODIndex)
{
    TArray<int32> ResultList;

    if (!SkeletalMesh || LODIndex < 0)
    {
        return ResultList;
    }

    // Get the skeletal mesh LOD data
#if ENGINE_MAJOR_VERSION >= 5
    FSkeletalMeshLODRenderData& LODData = SkeletalMesh->GetResourceForRendering()->LODRenderData[LODIndex];
#else
    FSkeletalMeshLODRenderData& LODData = SkeletalMesh->GetResourceForRendering()->LODRenderData[LODIndex];
#endif

    // Get the vertex buffer
    const FStaticMeshVertexBuffer& VertexBuffer = LODData.StaticVertexBuffers.StaticMeshVertexBuffer;

    // Iterate through each UV set
    for (uint32 UVSetIndex = 0; UVSetIndex < VertexBuffer.GetNumTexCoords(); UVSetIndex++)
    {
        bool IsEmpty = true;

#if ENGINE_MAJOR_VERSION >= 5
        for (uint32 VertIndex = 0; VertIndex < VertexBuffer.GetNumVertices(); ++VertIndex)
        {
            // For UE5, use FVector2f
            FVector2f uv = VertexBuffer.GetVertexUV(VertIndex, UVSetIndex);
            FVector2f uvempty(0.f, 1.f);
            FVector2f uvempty2(0.f, 0.f);
            FVector2f uvempty3(1.f, 0.f);
            FVector2f uvempty4(1.f, 1.f);

            // Define tolerance
            const float Tolerance = 0.01f;

            // Custom comparison function to check if two UV coordinates are nearly equal within tolerance
            auto IsNearlyEqual = [Tolerance](const FVector2f& A, const FVector2f& B) -> bool {
                return FMath::Abs(A.X - B.X) <= Tolerance && FMath::Abs(A.Y - B.Y) <= Tolerance;
            };

            // Check if UV coordinates are nearly equal to uvempty or uvempty2 or uvempty3 or uvempty4
            if (!IsNearlyEqual(uv, uvempty) &&
                !IsNearlyEqual(uv, uvempty2) &&
                !IsNearlyEqual(uv, uvempty3) &&
                !IsNearlyEqual(uv, uvempty4))
            {
                IsEmpty = false;
                break;
            }
        }
#else
        for (uint32 VertIndex = 0; VertIndex < VertexBuffer.GetNumVertices(); ++VertIndex)
        {
            // For UE4, use FVector2D
            FVector2D uv = VertexBuffer.GetVertexUV(VertIndex, UVSetIndex);
            FVector2D uvempty(0.f, 1.f);
            FVector2D uvempty2(0.f, 0.f);
            FVector2D uvempty3(1.f, 0.f);
            FVector2D uvempty4(1.f, 1.f);

            // Define tolerance
            const float Tolerance = 0.01f;

            // Custom comparison function for UE4 as well
            auto IsNearlyEqual = [Tolerance](const FVector2D& A, const FVector2D& B) -> bool {
                return FMath::Abs(A.X - B.X) <= Tolerance && FMath::Abs(A.Y - B.Y) <= Tolerance;
            };

            // Check with the same tolerance approach as UE5
            if (!IsNearlyEqual(uv, uvempty) &&
                !IsNearlyEqual(uv, uvempty2) &&
                !IsNearlyEqual(uv, uvempty3) &&
                !IsNearlyEqual(uv, uvempty4))
            {
                IsEmpty = false;
                break;
            }
        }
#endif

        // Add result to list (1 = empty, 0 = not empty)
        ResultList.Add(IsEmpty ? 1 : 0);
    }

    return ResultList;
}

void UAssetCheckToolBPLibrary::ExportStaticMeshesAsFBX(const TArray<UStaticMesh*>& StaticMeshes, const FString& OutputFolder)
{
	if (StaticMeshes.Num() == 0)
	{
		return;
	}

	// 获取AssetTools模块
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	// 创建导出参数
	TArray<UObject*> ObjectsToExport;

	for (UStaticMesh* StaticMesh : StaticMeshes)
	{
		if (StaticMesh)
		{
			ObjectsToExport.Add(StaticMesh);
		}
	}

	// 批量导出StaticMeshes
	AssetToolsModule.Get().ExportAssets(ObjectsToExport, OutputFolder);
}

TArray<UObject*> UAssetCheckToolBPLibrary::GetAllAssetsByClass(TSubclassOf<UObject> AssetClass)
{
	TArray<UObject*> Assets;

	// 获取 AssetRegistry 模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// 创建 AssetRegistry 查询
	FARFilter Filter;
#if ENGINE_MAJOR_VERSION >= 5
	Filter.ClassPaths.Add(AssetClass->GetClassPathName()); // UE5迁移: fix warning about DEPRECATED 'ClassNames'
#else
	Filter.ClassNames.Add(AssetClass->GetFName());
#endif
	Filter.bRecursiveClasses = true;

	// 运行查询
	TArray<FAssetData> AssetDataList;
	AssetRegistryModule.Get().GetAssets(Filter, AssetDataList);

	// 遍历查询结果，将资产添加到数组中
	for (const FAssetData& AssetData : AssetDataList)
	{

		if (UObject* Asset = AssetData.GetAsset())
		{
			Assets.Add(Asset);
		}
	}

	return Assets;
}

TArray<FString> UAssetCheckToolBPLibrary::GetAllAssetNamesByClass(TSubclassOf<UObject> AssetClass, bool bExcludeRedirectors)
{
	TArray<FString> PackageNames;

	// 获取 AssetRegistry 模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// 创建 AssetRegistry 查询
	FARFilter Filter;
#if ENGINE_MAJOR_VERSION >= 5
	Filter.ClassPaths.Add(AssetClass->GetClassPathName()); // UE5迁移: fix warning about DEPRECATED 'ClassNames'
#else
	Filter.ClassNames.Add(AssetClass->GetFName());
#endif
	Filter.bRecursiveClasses = true;

	// 运行查询
	TArray<FAssetData> AssetDataList;
	AssetRegistryModule.Get().GetAssets(Filter, AssetDataList);

	// 遍历查询结果，将资产包名添加到数组中
	for (const FAssetData& AssetData : AssetDataList)
	{
		// 如果需要排除 redirector 文件
		if (bExcludeRedirectors && AssetData.AssetName.ToString().EndsWith(TEXT("_C"), ESearchCase::IgnoreCase))
		{
			continue; // 跳过 redirector 文件
		}

		FString PackageName = AssetData.PackageName.ToString();
		PackageNames.Add(PackageName);
	}

	return PackageNames;
}

TArray<AActor*> UAssetCheckToolBPLibrary::GetAllActorsByClass(TSubclassOf<AActor> ActorClass)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();

	TArray<AActor*> FoundActors;

	// Ensure the world exists and the ActorClass is valid
	if (ActorClass && GEngine)
	{
		if (World)
		{
			UGameplayStatics::GetAllActorsOfClass(World, ActorClass, FoundActors);
		}
	}

	return FoundActors;
}

void UAssetCheckToolBPLibrary::CheckAndRemoveUVSets(UStaticMesh* Mesh, int32 MaxUVChannelCount, int32 LODIndex)
{
	if (Mesh && Mesh->GetRenderData() && LODIndex >= 0 && LODIndex < Mesh->GetRenderData()->LODResources.Num())
	{
		FStaticMeshLODResources& LODResource = Mesh->GetRenderData()->LODResources[LODIndex];
		//int32 NumUVChannels = Mesh->GetNumUVChannels(LODIndex);
		int32 NumUVChannels = GetNumTexCoords(Mesh, LODIndex);
		for (int32 UVChannelIndex = MaxUVChannelCount; UVChannelIndex < NumUVChannels; ++UVChannelIndex)
		{
			Mesh->RemoveUVChannel(LODIndex, UVChannelIndex);
		}

		// Get the package where the StaticMesh is located

		if (UPackage* Package = Mesh->GetOutermost())
		{
			// Mark the package as modified
			Package->MarkPackageDirty();
		}
	}
	else
	{
		// Handle invalid LOD index or missing render data here
	}
}

TArray<FString> UAssetCheckToolBPLibrary::FindSameSourceActorsWithDistance(TArray<AStaticMeshActor*> StaticMeshActors, float DistanceThreshold)
{
	TMap<FString, TArray<FString>> MatchingActorGroups;
	//UWorld* World = WorldContextObject->GetWorld();

	//Get Editor World
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		return TArray<FString>();
	}

	for (int32 i = 0; i < StaticMeshActors.Num(); i++)
	{
		AStaticMeshActor* CurrentActor = StaticMeshActors[i];
		FVector CurrentLocation = CurrentActor->GetActorLocation();

		TArray<FString> TempMatchingGroup;
		TempMatchingGroup.Add(CurrentActor->GetName());

		for (int32 j = i + 1; j < StaticMeshActors.Num(); j++)
		{
			AStaticMeshActor* OtherActor = StaticMeshActors[j];
			FVector OtherLocation = OtherActor->GetActorLocation();

			float Distance = FVector::Dist(CurrentLocation, OtherLocation);

			if (Distance < DistanceThreshold && CurrentActor->GetStaticMeshComponent()->GetStaticMesh() == OtherActor->GetStaticMeshComponent()->GetStaticMesh())
			{
				TempMatchingGroup.Add(OtherActor->GetName());
			}
		}

		if (TempMatchingGroup.Num() > 1)
		{
			TempMatchingGroup.Sort();
			FString MatchingGroup = FString::Join(TempMatchingGroup, TEXT(" "));
			FString Key = TempMatchingGroup[0]; // 使用第一个演员的名字作为键
			if (!MatchingActorGroups.Contains(Key) || MatchingActorGroups[Key].Num() < TempMatchingGroup.Num())
			{
				MatchingActorGroups.Add(Key, TempMatchingGroup);
			}
		}
	}

	TArray<FString> ResultArray;
	for (const auto& Pair : MatchingActorGroups)
	{
		ResultArray.Add(FString::Join(Pair.Value, TEXT(" ")));
	}

	return ResultArray;
}

TArray<AStaticMeshActor*> UAssetCheckToolBPLibrary::FindSameSourceActorsWithDistanceRatio(AStaticMeshActor* InputActor, float Ratio)
{
	TArray<AStaticMeshActor*> ResultActors;

	// 获取编辑器世界
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World || !InputActor)
	{
		return ResultActors;
	}

	// 获取输入actor的StaticMesh和BoundingBox的直径
	UStaticMesh* InputMesh = InputActor->GetStaticMeshComponent()->GetStaticMesh();
	float DistanceThreshold = InputActor->GetComponentsBoundingBox().GetSize().Size() * Ratio;

	// 遍历场景中的所有StaticMeshActor
	for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
	{
		AStaticMeshActor* CurrentActor = *It;

		// 如果当前actor与输入actor共享相同的StaticMesh，并且它们的距离小于DistanceThreshold
		if (CurrentActor->GetStaticMeshComponent()->GetStaticMesh() == InputMesh &&
			FVector::Dist(InputActor->GetActorLocation(), CurrentActor->GetActorLocation()) < DistanceThreshold &&
			InputActor->GetActorRotation() == CurrentActor->GetActorRotation() )
		{
			// 添加到结果数组
			ResultActors.Add(CurrentActor);
		}
	}

	return ResultActors;
}

TArray<AStaticMeshActor*> UAssetCheckToolBPLibrary::GetActorsFromName(const FString& ActorsString)
{
	TArray<AStaticMeshActor*> Actors;
	//Get Editor World
	UWorld* World = GEditor->GetEditorWorldContext().World();

	if (!World)
	{
		return Actors;
	}

	TArray<FString> ActorNames;
	ActorsString.ParseIntoArray(ActorNames, TEXT(" "), true);

	for (const FString& ActorName : ActorNames)
	{
		AStaticMeshActor* FoundActor = nullptr;

		// 根据 Actor 名称查找对应的 Actor
		for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
		{
			AStaticMeshActor* CurrentActor = *ActorItr;
			if (CurrentActor->GetName() == ActorName)
			{
				FoundActor = CurrentActor;
				break;
			}
		}

		if (FoundActor)
		{
			Actors.Add(FoundActor);
		}
	}

	return Actors;
}

FString UAssetCheckToolBPLibrary::GetStaticMeshLODSourceImportFilename(UStaticMesh* StaticMesh, int32 LODIndex)
{
	if (StaticMesh && StaticMesh->GetNumSourceModels() > LODIndex)
	{
		const FStaticMeshSourceModel& SrcModel = StaticMesh->GetSourceModel(LODIndex);
		return SrcModel.SourceImportFilename;
	}

	return FString();
}

void UAssetCheckToolBPLibrary::ImportStaticMeshLOD(UStaticMesh* StaticMesh, int32 LODIndex, const FString& FilePath)
{
	if (StaticMesh && LODIndex >= 0 && LODIndex < StaticMesh->GetNumLODs())
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		IAssetTools& AssetTools = AssetToolsModule.Get();
		TArray<FString> FilesToImport;
		FilesToImport.Add(FilePath);
		TArray<UObject*> ImportedAssets = AssetTools.ImportAssets(FilesToImport, StaticMesh->GetOutermost()->GetName());
		if (ImportedAssets.Num() > 0)
		{

			if (UStaticMesh* ImportedStaticMesh = Cast<UStaticMesh>(ImportedAssets[0]))
			{
				FStaticMeshSourceModel& SourceModel = StaticMesh->GetSourceModel(LODIndex);

				SourceModel.BuildSettings = ImportedStaticMesh->GetSourceModel(0).BuildSettings;
				StaticMesh->Modify();
			}
		}
	}
}

TArray<UObject*> UAssetCheckToolBPLibrary::GetAllAssetsByClassWithFilter(TSubclassOf<UObject> AssetClass, const TArray<FString>& SpecifiedPaths, const TArray<FString>& FilterEndWith, bool EndWithOut)
{
	// 获取 AssetRegistry 模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// 创建 AssetRegistry 查询
	FARFilter AssetFilter;
#if ENGINE_MAJOR_VERSION >= 5
	AssetFilter.ClassPaths.Add(AssetClass->GetClassPathName()); // UE5迁移: fix warning about DEPRECATED 'ClassNames'
#else
	AssetFilter.ClassNames.Add(AssetClass->GetFName());
#endif
	AssetFilter.bRecursiveClasses = true;

	// 预分配数组空间
	TArray<UObject*> Assets;
	Assets.Reserve(SpecifiedPaths.Num() * 5);

	// 遍历指定路径
	for ( FString Path : SpecifiedPaths)
	{

		if (Path.StartsWith("/All/"))
		{
			// 替换路径中的"/All/"为"/"
			Path=Path.Replace(TEXT("/All/"), TEXT("/"));
		}
		// 清理查询结果的内存
		TArray<FAssetData> AssetDataList;
		AssetDataList.Empty();

		// 添加当前路径
		AssetFilter.PackagePaths.Reset();
		AssetFilter.PackagePaths.Add(*Path.TrimStartAndEnd());		// 注意 清理路径字符串 删除多余的空格或换行符

		// 运行查询
		AssetRegistryModule.Get().GetAssets(AssetFilter, AssetDataList);

		// 遍历查询结果，将符合条件的资产添加到数组中
		for (const FAssetData& AssetData : AssetDataList)
		{

			if (UObject* Asset = AssetData.GetAsset())
			{
				// 获取资产的完整路径和名称
				FString AssetName = AssetData.AssetName.ToString();

				// 判断资产名称是否以指定字符结尾
				bool bEndsWithFilter = false;
				for (const FString& Filter : FilterEndWith)
				{
					if (AssetName.EndsWith(Filter))
					{
						bEndsWithFilter = true;
						break;
					}
				}

				// 如果资产名称不以指定字符结尾，则添加到数组中
				if (EndWithOut ? !bEndsWithFilter : bEndsWithFilter)
				{
					Assets.Add(Asset);
				}
			}
		}

		// 清理查询结果的内存
		AssetDataList.Empty();
	}

	// 释放未使用的内存
	Assets.Shrink();

	return Assets;
}


TArray<FString> UAssetCheckToolBPLibrary::GetAllAssetNamesByClassWithFilter(
	const TArray<TSubclassOf<UObject>>& AssetClasses, // 多个类的过滤
	const TArray<FString>& SpecifiedPaths,
	const TArray<FString>& FilterEndWith,
	bool EndWithOut,
	bool bUseClassFilter,
	bool bUseCast) // 新增的参数
{
	// 获取 AssetRegistry 模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// 创建 AssetRegistry 查询
	FARFilter AssetFilter;
	AssetFilter.bRecursiveClasses = true;

	// 根据 bInvertClassFilter 决定是否添加 AssetClasses
	if (!bUseClassFilter || bUseCast)
	{
		for (const TSubclassOf<UObject>& AssetClass : AssetClasses)
		{
#if ENGINE_MAJOR_VERSION >= 5
			AssetFilter.ClassPaths.Add(AssetClass->GetClassPathName()); // UE5迁移: fix warning about DEPRECATED 'ClassNames'
#else
			AssetFilter.ClassNames.Add(AssetClass->GetFName());
#endif
		}
	}

	// 预分配数组空间
	TArray<FString> PackageNames;
	PackageNames.Reserve(SpecifiedPaths.Num() * 5);

	// 遍历指定路径
	for ( FString Path : SpecifiedPaths)
	{
		if (Path.StartsWith("/All/"))
		{
			// 替换路径中的"/All/"为"/"
			Path=Path.Replace(TEXT("/All/"), TEXT("/"));
		}
		// 清理查询结果的内存
		TArray<FAssetData> AssetDataList;
		AssetDataList.Empty();

		// 添加当前路径
		AssetFilter.PackagePaths.Reset();
		AssetFilter.PackagePaths.Add(*Path.TrimStartAndEnd());		// 注意 清理路径字符串 删除多余的空格或换行符

		// 运行查询
		AssetRegistryModule.Get().GetAssets(AssetFilter, AssetDataList);

		// 遍历查询结果，将符合条件的资产包名添加到数组中
		for (const FAssetData& AssetData : AssetDataList)
		{
			// 获取资产的完整路径和名称
			FString AssetName = AssetData.AssetName.ToString();
			UObject* AssetObject = AssetData.GetAsset(); // 获取资产对象

			// 判断资产类是否匹配
			bool bClassMatches = false;

			// 检查 AssetClasses 是否为空
			if (AssetClasses.Num() == 0)
			{
				bClassMatches = true; // 如果为空，所有资产都匹配
			}
			else
			{
				for (const TSubclassOf<UObject>& AssetClass : AssetClasses)
				{
					// 检查 AssetClass 是否有效
					if (!AssetClass)
					{
						continue; // 如果 AssetClass 为 None，跳过
					}
#if ENGINE_MAJOR_VERSION >= 5
					if (AssetData.AssetClassPath == AssetClass->GetClassPathName())
#else
					if (AssetData.AssetClass == AssetClass->GetFName())
#endif

					{
						bClassMatches = true;
						break;
					}
					// 如果需要尝试转换
					if (bUseCast && AssetObject)
					{
						if (AssetClass->IsChildOf(AssetObject->GetClass()))
						{
							bClassMatches = true;
							break;
						}
					}
				}
			}

			// 如果反转类过滤，且类不匹配，则跳过
			if (bUseClassFilter && bClassMatches)
			{
				continue;
			}

			// 判断资产名称是否以指定字符结尾
			bool bEndsWithFilter = false;
			for (const FString& Filter : FilterEndWith)
			{
				if (AssetName.Contains(Filter))
				{
					bEndsWithFilter = true;
					break;
				}
			}

			// 如果资产名称不以指定字符结尾，则添加到数组中
			if (EndWithOut ? !bEndsWithFilter : bEndsWithFilter)
			{
				PackageNames.Add(AssetData.PackageName.ToString());
			}
		}

		// 清理查询结果的内存
		AssetDataList.Empty();
	}

	// 释放未使用的内存
	PackageNames.Shrink();

	return PackageNames;
}

TArray<FString> UAssetCheckToolBPLibrary::GetAllAssetNamesByPathWithFilter(
	const TArray<FString>& SpecifiedPaths,
	const TArray<FString>& FilterContains,
	bool bExcludeContains, // 更改参数名称
	bool bIncludeSubfolders,
	UClass* AssetClass)
{
	// 获取 AssetRegistry 模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// 创建 AssetRegistry 查询
	FARFilter AssetFilter;
	AssetFilter.bRecursiveClasses = true; // 保持递归查找，以便在子路径中查找
	AssetFilter.bRecursivePaths = bIncludeSubfolders; // 根据输入参数决定是否递归路径

	// 如果指定了类类型，则添加到过滤器中
	if (AssetClass)
	{
#if ENGINE_MAJOR_VERSION >= 5
		AssetFilter.ClassPaths.Add(AssetClass->GetClassPathName()); // UE5迁移: fix warning about DEPRECATED 'ClassNames'
#else
		AssetFilter.ClassNames.Add(AssetClass->GetFName());
#endif
	}

	// 预分配数组空间
	TArray<FString> PackageNames;
	PackageNames.Reserve(SpecifiedPaths.Num() * 5);

	// 遍历指定路径
	for (FString Path : SpecifiedPaths)
	{

		if (Path.StartsWith("/All/"))
		{
			// 替换路径中的"/All/"为"/"
			Path=Path.Replace(TEXT("/All/"), TEXT("/"));
		}
		// 去除路径末尾的斜杠
		FPaths::NormalizeDirectoryName(Path);

		// 清理查询结果的内存
		TArray<FAssetData> AssetDataList;
		AssetDataList.Empty();

		// 添加当前路径
		AssetFilter.PackagePaths.Reset();
		AssetFilter.PackagePaths.Add(*Path.TrimStartAndEnd());		// 注意 清理路径字符串 删除多余的空格或换行符

		// 运行查询
		AssetRegistryModule.Get().GetAssets(AssetFilter, AssetDataList);

		// 遍历查询结果，将符合条件的资产包名添加到数组中
		for (const FAssetData& AssetData : AssetDataList)
		{
			// 获取资产的完整路径和名称
			FString AssetName = AssetData.AssetName.ToString();

			// 判断资产名称是否包含指定字符
			bool bContainsFilter = false;
			for (const FString& Filter : FilterContains)
			{
				if (AssetName.Contains(Filter))
				{
					bContainsFilter = true;
					break;
				}
			}

			// 如果资产名称不包含指定字符，则添加到数组中
			if (bExcludeContains ? !bContainsFilter : bContainsFilter) // 使用新的参数名称
			{
				PackageNames.Add(AssetData.PackageName.ToString());
			}
		}

		// 清理查询结果的内存
		AssetDataList.Empty();
	}

	// 释放未使用的内存
	PackageNames.Shrink();

	return PackageNames;
}

TArray<FString> UAssetCheckToolBPLibrary::GetAllAssetNamesByPathWithFilterPlus(
    const TArray<FString>& SpecifiedPaths,
    const TArray<FString>& FilterContains,
    const TArray<FString>& ExcludeContains,
    bool bIncludeSubfolders,
    UClass* AssetClass,
    bool bIncludeRedirectors, // 新增参数：是否包含重定向文件
    bool bWithClass) // 新增参数：是否进行class过滤
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    FARFilter AssetFilter;
    AssetFilter.bRecursiveClasses = true;
    AssetFilter.bRecursivePaths = bIncludeSubfolders;

    if (bWithClass && AssetClass)
    {
#if ENGINE_MAJOR_VERSION >= 5
        AssetFilter.ClassPaths.Add(AssetClass->GetClassPathName()); // UE5迁移: fix warning about DEPRECATED 'ClassNames'
#else
        AssetFilter.ClassNames.Add(AssetClass->GetFName());
#endif
    }

    TArray<FString> PackageNames;
    PackageNames.Reserve(SpecifiedPaths.Num() * 5);

    for (FString Path : SpecifiedPaths)
    {
        FPaths::NormalizeDirectoryName(Path);
    	if (Path.StartsWith("/All/"))
    	{
    		// 替换路径中的"/All/"为"/"
    		Path=Path.Replace(TEXT("/All/"), TEXT("/"));
    	}


        TArray<FAssetData> AssetDataList;

        AssetFilter.PackagePaths.Reset();
        AssetFilter.PackagePaths.Add(*Path.TrimStartAndEnd()); // 注意 清理路径字符串 删除多余的空格或换行符

        AssetRegistryModule.Get().GetAssets(AssetFilter, AssetDataList);

        for (const FAssetData& AssetData : AssetDataList)
        {
            // 如果 bIncludeRedirectors 为 false，并且当前资产是重定向文件，则跳过
            if (!bIncludeRedirectors && AssetData.IsRedirector())
            {
                continue;
            }

            FString AssetName = AssetData.AssetName.ToString();
            bool bContainsFilter = true; // Assume inclusion unless FilterContains is specified and matches.

            if (FilterContains.Num() > 0)
            {
                // 检查FilterContains数组是否全为空字符串
                bool bAllEmpty = true;
                for (const FString& Filter : FilterContains)
                {
                    if (!Filter.IsEmpty())
                    {
                        bAllEmpty = false;
                        break;
                    }
                }
                // 如果FilterContains数组不全为空，才进行包含判断
                bContainsFilter = bAllEmpty ? true : FilterContains.ContainsByPredicate([&](const FString& Filter){ return AssetName.Contains(Filter); });
            }

            // 检查ExcludeContains数组是否全为空字符串
            bool bAllEmpty = true;
            for (const FString& Exclude : ExcludeContains)
            {
                if (!Exclude.IsEmpty())
                {
                    bAllEmpty = false;
                    break;
                }
            }
            // 如果ExcludeContains数组不全为空，才进行排除判断
            bool bContainsExclude = bAllEmpty ? false : ExcludeContains.ContainsByPredicate([&](const FString& Exclude){ return AssetName.Contains(Exclude); });

            if (bContainsFilter && !bContainsExclude)
            {
                PackageNames.Add(AssetData.PackageName.ToString());
            }
        }
    }

    PackageNames.Shrink();
    return PackageNames;
}



TArray<UObject*> UAssetCheckToolBPLibrary::GetAllAssetsByClassWithFilterPlus(
	TSubclassOf<UObject> AssetClass,
	const TArray<FString>& SpecifiedPaths,
	const TArray<FString>& WithFilters,
	const TArray<FString>& WithoutFilters)
{
	// 获取 AssetRegistry 模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// 创建 AssetRegistry 查询
	FARFilter AssetFilter;
#if ENGINE_MAJOR_VERSION >= 5
	AssetFilter.ClassPaths.Add(AssetClass->GetClassPathName()); // UE5迁移: fix warning about DEPRECATED 'ClassNames'
#else
	AssetFilter.ClassNames.Add(AssetClass->GetFName());
#endif
	AssetFilter.bRecursiveClasses = true;

	// 预分配数组空间
	TArray<UObject*> Assets;
	Assets.Reserve(SpecifiedPaths.Num() * 5);

	// 遍历指定路径
	for (const FString& Path : SpecifiedPaths)
	{
		// 清理查询结果的内存
		TArray<FAssetData> AssetDataList;
		AssetDataList.Empty();

		// 添加当前路径
		AssetFilter.PackagePaths.Reset();
		AssetFilter.PackagePaths.Add(*Path.TrimStartAndEnd());		// 注意 清理路径字符串 删除多余的空格或换行符

		// 运行查询
		AssetRegistryModule.Get().GetAssets(AssetFilter, AssetDataList);

		// 遍历查询结果，将符合条件的资产添加到数组中
		for (const FAssetData& AssetData : AssetDataList)
		{

			if (UObject* Asset = AssetData.GetAsset())
			{
				// 获取资产的完整路径和名称
				FString AssetName = AssetData.AssetName.ToString();

				// 判断资产名称是否包含 WithFilters 中的任意字符
				bool bContainsWithFilter = true;
				if (WithFilters.Num() > 0) // 只有在 WithFilters 不为空时才进行检查
				{
					bContainsWithFilter = false;
					for (const FString& Filter : WithFilters)
					{
						if (AssetName.Contains(Filter))
						{
							bContainsWithFilter = true;
							break;
						}
					}
				}

				// 判断资产名称是否包含 WithoutFilters 中的任意字符
				bool bContainsWithoutFilter = false;
				for (const FString& Filter : WithoutFilters)
				{
					if (AssetName.Contains(Filter))
					{
						bContainsWithoutFilter = true;
						break;
					}
				}

				// 如果符合条件，则添加到数组中
				if (bContainsWithFilter && !bContainsWithoutFilter)
				{
					Assets.Add(Asset);
				}
			}
		}

		// 清理查询结果的内存
		AssetDataList.Empty();
	}

	// 释放未使用的内存
	Assets.Shrink();

	return Assets;
}

TArray<FString> UAssetCheckToolBPLibrary::GetAllAssetNamesByPathWithFilters(
    const TArray<FString>& SpecifiedPaths,
    const TArray<FString>& FilterContains,
    const TArray<FString>& ExcludeContains,
    bool bIncludeSubfolders,
    const TArray<UClass*>& AssetClasses, // Changed to TArray<UClass*>
    bool bIncludeRedirectors)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FARFilter AssetFilter;
	AssetFilter.bRecursiveClasses = true;
	AssetFilter.bRecursivePaths = bIncludeSubfolders;

	// Handle multiple classes
	for (UClass* AssetClass : AssetClasses)
	{
		if (AssetClass)
		{
#if ENGINE_MAJOR_VERSION >= 5
			AssetFilter.ClassPaths.Add(AssetClass->GetClassPathName()); // UE5: Use ClassPathName
#else
			AssetFilter.ClassNames.Add(AssetClass->GetFName()); // Pre-UE5: Use ClassNames
#endif
		}
	}

	TArray<FString> PackageNames;
	PackageNames.Reserve(SpecifiedPaths.Num() * 5);

	for (const FString& Path : SpecifiedPaths)
	{
		FString NormalizedPath = Path;
		FPaths::NormalizeDirectoryName(NormalizedPath);
		TArray<FAssetData> AssetDataList;

		AssetFilter.PackagePaths.Reset();
		AssetFilter.PackagePaths.Add(*Path.TrimStartAndEnd());

		AssetRegistryModule.Get().GetAssets(AssetFilter, AssetDataList);

		for (const FAssetData& AssetData : AssetDataList)
		{
			if (!bIncludeRedirectors && AssetData.IsRedirector())
			{
				continue;
			}

			FString AssetName = AssetData.AssetName.ToString();
			bool bContainsFilter = true;

			if (FilterContains.Num() > 0)
			{
				bContainsFilter = FilterContains.ContainsByPredicate([&](const FString& Filter){ return AssetName.Contains(Filter); });
			}

			bool bContainsExclude = ExcludeContains.ContainsByPredicate([&](const FString& Exclude){ return AssetName.Contains(Exclude); });

			if (bContainsFilter && !bContainsExclude)
			{
				PackageNames.Add(AssetData.PackageName.ToString());
			}
		}
	}

	PackageNames.Shrink();
	return PackageNames;
}

bool UAssetCheckToolBPLibrary::IsAssetValidByFilter(
	UObject* Asset,
	const TArray<FDirectoryPath>& DirectoryPaths = TArray<FDirectoryPath>(),
	const TArray<FString>& IgnoreSubPaths = TArray<FString>(),
	const TArray<FString>& OnlyNames = TArray<FString>(),
	const TArray<FString>& IgnoreNames = TArray<FString>())
{
	if (!Asset)
	{
		return false; // 如果 Asset 为 nullptr，直接返回 false
	}

	// 获取资产的完整路径
	FString AssetPath = Asset->GetPathName();

	// 检查路径是否在指定的 DirectoryPaths 中
	bool bPathValid = false;
	if(DirectoryPaths.Num() == 0)
	{
		bPathValid = true;
	}
	else
	{
		for (const FDirectoryPath& DirectoryPath : DirectoryPaths)
		{
			if (AssetPath.StartsWith(DirectoryPath.Path))
			{
				bPathValid = true;
				break;
			}
		}
	}

	// 如果路径不在指定的 DirectoryPaths 中，返回 false
	if (!bPathValid)
	{
		return false;
	}

	// 检查是否在忽略的子路径中
	for (const FString& IgnoreSubPath : IgnoreSubPaths)
	{
		if (AssetPath.Contains(IgnoreSubPath))
		{
			return false; // 如果路径包含忽略的子路径，返回 false
		}
	}

	// 获取文件名
	FString AssetName = FPaths::GetBaseFilename(AssetPath);

	// 检查文件名是否在 OnlyNames 中
	bool bNameValid = OnlyNames.Num() == 0; // 如果 OnlyNames 为空，默认有效
	for (const FString& OnlyName : OnlyNames)
	{
		if (AssetName.Contains(OnlyName))
		{
			bNameValid = true;
			break;
		}
	}

	// 如果文件名不在 OnlyNames 中，返回 false
	if (!bNameValid)
	{
		return false;
	}

	// 检查是否在忽略的文件名中
	for (const FString& IgnoreName : IgnoreNames)
	{
		if (AssetName.Contains(IgnoreName))
		{
			return false; // 如果文件名包含忽略的文件名，返回 false
		}
	}

	// 如果所有条件都通过，返回 true
	return true;
}


bool UAssetCheckToolBPLibrary::IsAssetValidByIgnoreFilter(
	UObject* Asset,
	const TArray<FString>& IgnoreSubPaths,
	const TArray<FString>& IgnoreNames)
{
	if (!Asset)
	{
		return false; // 如果 Asset 为 nullptr，直接返回 false
	}

	// 获取资产的完整路径
	FString AssetPath = Asset->GetPathName();
	// 获取资产的文件名
	FString AssetName = FPaths::GetBaseFilename(AssetPath);

	// 检查是否在忽略的子路径中
	for (const FString& IgnoreSubPath : IgnoreSubPaths)
	{
		if (AssetPath.Contains(IgnoreSubPath))
		{
			return false; // 如果路径包含忽略的子路径，返回 false
		}
	}

	// 检查是否在忽略的文件名中
	for (const FString& IgnoreName : IgnoreNames)
	{
		if (AssetName.Contains(IgnoreName))
		{
			return false; // 如果文件名包含忽略的文件名，返回 false
		}
	}

	// 如果所有条件都通过，返回 true
	return true;
}

bool UAssetCheckToolBPLibrary::IsAssetValidByIgnoreFilterPlus(
	UObject* Asset,
	const TArray<FString>& IgnoreSubPaths,
	const TArray<FString>& IgnoreNames,
	const TArray<FString>& IncludeNames) // 新增的参数
{
	if (!Asset)
	{
		return false; // 如果 Asset 为 nullptr，直接返回 false
	}

	// 获取资产的完整路径
	FString AssetPath = Asset->GetPathName();
	// 获取资产的文件名
	FString AssetName = FPaths::GetBaseFilename(AssetPath);

	// 检查是否在忽略的子路径中
	for (const FString& IgnoreSubPath : IgnoreSubPaths)
	{
		if (AssetPath.Contains(IgnoreSubPath))
		{
			return false; // 如果路径包含忽略的子路径，返回 false
		}
	}

	// 检查是否在忽略的文件名中
	for (const FString& IgnoreName : IgnoreNames)
	{
		if (AssetName.Contains(IgnoreName))
		{
			return false; // 如果文件名包含忽略的文件名，返回 false
		}
	}

	// 检查是否在包含的文件名中
	if (IncludeNames.Num() > 0) // 如果 IncludeNames 不为空
	{
		bool bIncludeValid = false;
		for (const FString& IncludeName : IncludeNames)
		{
			if (AssetName.Contains(IncludeName))
			{
				bIncludeValid = true;
				break;
			}
		}

		// 如果文件名不包含任何 IncludeNames 中的字符，返回 false
		if (!bIncludeValid)
		{
			return false;
		}
	}

	// 如果所有条件都通过，返回 true
	return true;
}



bool UAssetCheckToolBPLibrary::IsAssetPathValidByIgnoreFilter(
	const FString& AssetPath,
	const TArray<FString>& IgnoreSubPaths,
	const TArray<FString>& IgnoreNames)
{
	if (AssetPath.IsEmpty())
	{
		return false; // 如果 AssetPath 为空，直接返回 false
	}

	// 获取资产的文件名
	FString AssetName = FPaths::GetBaseFilename(AssetPath);

	// 检查是否在忽略的子路径中
	for (const FString& IgnoreSubPath : IgnoreSubPaths)
	{
		if (AssetPath.Contains(IgnoreSubPath))
		{
			return false; // 如果路径包含忽略的子路径，返回 false
		}
	}

	// 检查是否在忽略的文件名中
	for (const FString& IgnoreName : IgnoreNames)
	{
		if (AssetName.Contains(IgnoreName))
		{
			return false; // 如果文件名包含忽略的文件名，返回 false
		}
	}

	// 如果所有条件都通过，返回 true
	return true;
}

bool UAssetCheckToolBPLibrary::IsAssetPathValidByIgnoreFilterPlus(
	const FString& AssetPath,
	const TArray<FString>& IgnoreSubPaths,
	const TArray<FString>& IgnoreNames,
	const TArray<FString>& IncludeNames) // 新增的参数
{
	if (AssetPath.IsEmpty())
	{
		return false; // 如果 AssetPath 为空，直接返回 false
	}

	// 获取资产的文件名
	FString AssetName = FPaths::GetBaseFilename(AssetPath);

	// 检查是否在忽略的子路径中
	for (const FString& IgnoreSubPath : IgnoreSubPaths)
	{
		if (AssetPath.Contains(IgnoreSubPath))
		{
			return false; // 如果路径包含忽略的子路径，返回 false
		}
	}

	// 检查是否在忽略的文件名中
	for (const FString& IgnoreName : IgnoreNames)
	{
		if (AssetName.Contains(IgnoreName))
		{
			return false; // 如果文件名包含忽略的文件名，返回 false
		}
	}

	// 检查是否在包含的文件名中
	if (IncludeNames.Num() > 0) // 如果 IncludeNames 不为空
	{
		bool bIncludeValid = false;
		for (const FString& IncludeName : IncludeNames)
		{
			if (AssetName.Contains(IncludeName))
			{
				bIncludeValid = true;
				break;
			}
		}

		// 如果文件名不包含任何 IncludeNames 中的字符，返回 false
		if (!bIncludeValid)
		{
			return false;
		}
	}

	// 如果所有条件都通过，返回 true
	return true;
}

bool UAssetCheckToolBPLibrary::IsAssetValidByFilters(
	const FString& AssetPath,
	const TArray<FString>& IgnoreSubPaths,
	const TArray<FString>& IgnoreNames,
	const TArray<FString>& IncludeNames, // 新增的参数
	const TArray<UClass*>& AllowedClasses) // 新增的允许类参数
{
    if (AssetPath.IsEmpty())
    {
        return false; // 如果 AssetPath 为空，直接返回 false
    }

	// 标准化路径

	FString StandardizedPath = ConvertResourcePath(AssetPath);

	// 加载资产注册表模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

#if ENGINE_MAJOR_VERSION >= 5
	// 创建一个 Soft Object Path
	FSoftObjectPath SoftObjectPath(StandardizedPath);
	// 查找资产 using FSoftObjectPath
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(SoftObjectPath);
#else
	// 查找资产 using FName
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(*StandardizedPath);
#endif

    if (!AssetData.IsValid())
    {
        return false; // 如果资产数据无效，返回 false
    }

    // 检查是否在允许的类中
    if (AllowedClasses.Num() > 0) // 如果 AllowedClasses 不为空
    {
        // 如果 AllowedClasses 中包含 nullptr，则忽略类的过滤
        bool bIgnoreClassFilter = AllowedClasses.ContainsByPredicate([](const UClass* Class) { return Class == nullptr; });
        if (!bIgnoreClassFilter)
        {
            // 检查资产是否属于 AllowedClasses 中的某个类
            bool bClassValid = false;
            for (const UClass* AllowedClass : AllowedClasses)
            {
                if (AllowedClass && AssetData.GetClass()->IsChildOf(AllowedClass))
                {
                    bClassValid = true;
                    break;
                }
            }

            // 如果资产不属于任何允许的类，返回 false
            if (!bClassValid)
            {
                return false;
            }
        }
        // 如果 bIgnoreClassFilter 为 true，则跳过类的过滤
    }

    // 获取资产的文件名
    FString AssetName = FPaths::GetBaseFilename(AssetPath);

    // 检查是否在忽略的子路径中
    for (const FString& IgnoreSubPath : IgnoreSubPaths)
    {
        if (AssetPath.Contains(IgnoreSubPath))
        {
            return false; // 如果路径包含忽略的子路径，返回 false
        }
    }

    // 检查是否在忽略的文件名中
    for (const FString& IgnoreName : IgnoreNames)
    {
        if (AssetName.Contains(IgnoreName))
        {
            return false; // 如果文件名包含忽略的文件名，返回 false
        }
    }

    // 检查是否在包含的文件名中
    if (IncludeNames.Num() > 0) // 如果 IncludeNames 不为空
    {
        bool bIncludeValid = false;
        for (const FString& IncludeName : IncludeNames)
        {
            if (AssetName.Contains(IncludeName))
            {
                bIncludeValid = true;
                break;
            }
        }

        // 如果文件名不包含任何 IncludeNames 中的字符，返回 false
        if (!bIncludeValid)
        {
            return false;
        }
    }

    // 如果所有条件都通过，返回 true
    return true;
}

void UAssetCheckToolBPLibrary::RecursiveAddSubFolders(const FString& Path, FARFilter& AssetFilter)
{
	// 添加当前路径
	AssetFilter.PackagePaths.Add(*Path.TrimStartAndEnd());		// 注意 清理路径字符串 删除多余的空格或换行符
	// 获取当前路径下的所有文件和文件夹
	TArray<FString> FilesAndFolders;
	//IFileManager::Get().FindFiles(FilesAndFolders, *Path, false, true);
	IFileManager::Get().FindFilesRecursive(FilesAndFolders, *Path, TEXT("*"), true, true,false);

	// 递归添加子文件夹路径
	for (const FString& FileOrFolder : FilesAndFolders)
	{
		FString FullPath = FPaths::Combine(Path, FileOrFolder);
		if (IFileManager::Get().DirectoryExists(*FullPath))
		{
			// 将父文件夹的路径与子文件夹的名称组合成完整的路径
			FString SubFolderPath = FullPath;
			RecursiveAddSubFolders(SubFolderPath, AssetFilter);
		}
	}
}

void UAssetCheckToolBPLibrary::MarkObjectDirty(UObject* object, bool Enable)
{
	// 获取 StaticMesh 所在的包
	UPackage* Package = object->GetOutermost();

	// 标记包已被修改
	Package->MarkPackageDirty();
}

void UAssetCheckToolBPLibrary::SaveAssets(const TArray<UObject*>& Assets)
{
	// 确保传入的资产数组不为空
	if (Assets.Num() == 0)
	{
		return;
	}

	// 获取第一个资产的包路径
	UObject* FirstAsset = Assets[0];
	FString PackagePath = FPackageName::GetLongPackagePath(FirstAsset->GetOutermost()->GetName());

	// 保存所有资产
	TArray<UPackage*> PackagesToSave;
	for (UObject* Asset : Assets)
	{
		// 确保传入的资产不为空
		if (Asset)
		{
			UPackage* Package = Asset->GetOutermost();
			PackagesToSave.AddUnique(Package);
		}
	}

	FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
	FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true, true);
}

void UAssetCheckToolBPLibrary::SaveAssetsSoft(const TArray<TSoftObjectPtr<UObject>>& Assets)
{
	// 确保传入的资产数组不为空
	if (Assets.Num() == 0)
	{
		return;
	}

	// 获取第一个资产的包路径
	UObject* FirstAsset = Assets[0].Get();
	if (!FirstAsset)
	{
		return; // 如果第一个资产无效，直接返回
	}

	FString PackagePath = FPackageName::GetLongPackagePath(FirstAsset->GetOutermost()->GetName());

	// 保存所有资产
	TArray<UPackage*> PackagesToSave;
	for (const TSoftObjectPtr<UObject>& SoftAsset : Assets)
	{
		// 确保传入的资产有效

		if (UObject* Asset = SoftAsset.Get())
		{
			UPackage* Package = Asset->GetOutermost();
			PackagesToSave.AddUnique(Package);
		}
	}

	// 弹出保存弹窗
	FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
	FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true, true);
}


bool  UAssetCheckToolBPLibrary::IsMeshClosed(UStaticMesh* Mesh)
{
	if (!Mesh || !Mesh->GetRenderData() || Mesh->GetRenderData()->LODResources.Num() == 0)
	{
		// 如果Mesh为空指针，或者RenderData为空，或者LODResources数量为0，则返回false
		return false;
	}

	// 获取StaticMesh的LOD数据
	FStaticMeshLODResources& LODModel = Mesh->GetRenderData()->LODResources[0];

	// 创建一个映射，用于存储每个边的平均位置和它出现的次数
#if ENGINE_MAJOR_VERSION >= 5
	TMap<FVector3f, int32> EdgeAveragePositionCountMap;
#else
	TMap<FVector, int32> EdgeAveragePositionCountMap;
#endif


	// 获取顶点缓冲区
	const FPositionVertexBuffer& VertexBuffer = LODModel.VertexBuffers.PositionVertexBuffer;

	// 获取索引缓冲区
	TArray<uint32> Indices;
	LODModel.IndexBuffer.GetCopy(Indices);

	// 遍历所有的三角形
	for (int32 i = 0; i < LODModel.GetNumTriangles(); ++i)
	{
#if ENGINE_MAJOR_VERSION >= 5
		// 获取三角形的顶点位置  //ue5
		FVector3f Vertex1 = VertexBuffer.VertexPosition(Indices[i * 3 + 0]);
		FVector3f Vertex2 = VertexBuffer.VertexPosition(Indices[i * 3 + 1]);
		FVector3f Vertex3 = VertexBuffer.VertexPosition(Indices[i * 3 + 2]);

		// 计算每条边的平均位置
		FVector3f Edge1AveragePosition = (Vertex1 + Vertex2) / 2.0f;
		FVector3f Edge2AveragePosition = (Vertex2 + Vertex3) / 2.0f;
		FVector3f Edge3AveragePosition = (Vertex3 + Vertex1) / 2.0f;
#else
		// 获取三角形的顶点位置   //ue4
		FVector Vertex1 = VertexBuffer.VertexPosition(Indices[i * 3 + 0]);
		FVector Vertex2 = VertexBuffer.VertexPosition(Indices[i * 3 + 1]);
		FVector Vertex3 = VertexBuffer.VertexPosition(Indices[i * 3 + 2]);

		// 计算每条边的平均位置
		FVector Edge1AveragePosition = (Vertex1 + Vertex2) / 2.0f;
		FVector Edge2AveragePosition = (Vertex2 + Vertex3) / 2.0f;
		FVector Edge3AveragePosition = (Vertex3 + Vertex1) / 2.0f;
#endif

		// 增加边的平均位置计数
		EdgeAveragePositionCountMap.FindOrAdd(Edge1AveragePosition)++;
		EdgeAveragePositionCountMap.FindOrAdd(Edge2AveragePosition)++;
		EdgeAveragePositionCountMap.FindOrAdd(Edge3AveragePosition)++;

	}

	// 检查所有的边的平均位置出现的次数是否为2
	for (auto& Elem : EdgeAveragePositionCountMap)
	{
		if (Elem.Value % 2 != 0)
		{
			// 如果有边的平均位置出现次数不为2，那么模型就不是封闭的
			return false;
		}
	}

	// 如果所有的边的平均位置出现次数都为2，那么模型就是封闭的
	return true;
}

bool UAssetCheckToolBPLibrary::checkUVOverlap(UStaticMesh* Mesh, int32 LODIndex, int32 UVIndex, float tolerance)
{
	if (Mesh && LODIndex >= 0 && UVIndex >= 0 && UVIndex < Mesh->GetRenderData()->LODResources[LODIndex].GetNumTexCoords())
	{
		// 获取StaticMesh的UV源
		const FStaticMeshLODResources& LODResource = Mesh->GetRenderData()->LODResources[LODIndex];
		const FStaticMeshVertexBuffer& VertexBuffer = LODResource.VertexBuffers.StaticMeshVertexBuffer;

		// 获取StaticMesh的索引缓冲区
		const FIndexArrayView& IndexBuffer = LODResource.IndexBuffer.GetArrayView();

		// 存储所有边的UV线段
#if ENGINE_MAJOR_VERSION >= 5
		TArray<FVector2f> UVSegments; //ue5
#else
		TArray<FVector2D> UVSegments; //ue4
#endif




		// 遍历每个三角形的边
		for (uint32 Index = 0; Index < static_cast<uint32>(IndexBuffer.Num()); Index += 3)
		{
			uint32 VertexIndex0 = IndexBuffer[Index];
			uint32 VertexIndex1 = IndexBuffer[Index + 1];
			uint32 VertexIndex2 = IndexBuffer[Index + 2];
#if ENGINE_MAJOR_VERSION >= 5
			FVector2f UV0 = VertexBuffer.GetVertexUV(VertexIndex0, UVIndex);
			FVector2f UV1 = VertexBuffer.GetVertexUV(VertexIndex1, UVIndex);
			FVector2f UV2 = VertexBuffer.GetVertexUV(VertexIndex2, UVIndex);
#else
			FVector2D UV0 = VertexBuffer.GetVertexUV(VertexIndex0, UVIndex);
			FVector2D UV1 = VertexBuffer.GetVertexUV(VertexIndex1, UVIndex);
			FVector2D UV2 = VertexBuffer.GetVertexUV(VertexIndex2, UVIndex);
#endif

			// 添加三角形的三条边到UV线段数组
			UVSegments.Add(UV0);
			UVSegments.Add(UV1);
			UVSegments.Add(UV1);
			UVSegments.Add(UV2);
			UVSegments.Add(UV2);
			UVSegments.Add(UV0);
		}

		// 遍历UV线段数组，逐个对比是否相交
		for (int32 i = 0; i < UVSegments.Num(); i += 2)
		{
			for (int32 j = i + 2; j < UVSegments.Num(); j += 2)
			{
#if ENGINE_MAJOR_VERSION >= 5
				if (AreSegmentsIntersecting(  //ue5
				FVector2D(UVSegments[i]),      // 显式转换为 FVector2D
				FVector2D(UVSegments[i + 1]),  // 显式转换为 FVector2D
				FVector2D(UVSegments[j]),      // 显式转换为 FVector2D
				FVector2D(UVSegments[j + 1]),  // 显式转换为 FVector2D
				tolerance))
#else
				if (AreSegmentsIntersecting(UVSegments[i], UVSegments[i + 1], UVSegments[j], UVSegments[j + 1], tolerance)) //ue4
#endif
				{
					return true; // 相交
				}
			}
		}
	}

	return false; // 不相交
}

bool UAssetCheckToolBPLibrary::CheckUVOverlapForSkeletalMesh(USkeletalMesh* Mesh, int32 LODIndex, int32 UVIndex, float tolerance)
{
    if (!Mesh || LODIndex < 0 || UVIndex < 0)
    {
        return false;
    }

    // Get the skeletal mesh LOD data
#if ENGINE_MAJOR_VERSION >= 5
    FSkeletalMeshLODRenderData& LODData = Mesh->GetResourceForRendering()->LODRenderData[LODIndex];
#else
    FSkeletalMeshLODRenderData& LODData = Mesh->GetResourceForRendering()->LODRenderData[LODIndex];
#endif

    // Check if the UV index is valid for this mesh
	if (static_cast<uint32>(UVIndex) >= LODData.StaticVertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords())
	{
		return false;
	}

    // Get the vertex and index buffers
    const FStaticMeshVertexBuffer& VertexBuffer = LODData.StaticVertexBuffers.StaticMeshVertexBuffer;
    const FRawStaticIndexBuffer16or32Interface& IndexBuffer = *LODData.MultiSizeIndexContainer.GetIndexBuffer();

    // Store all UV segments
#if ENGINE_MAJOR_VERSION >= 5
    TArray<FVector2f> UVSegments; // UE5
#else
    TArray<FVector2D> UVSegments; // UE4
#endif

    // Loop through all triangles
    uint32 NumIndices = IndexBuffer.Num();
    for (uint32 Index = 0; Index < NumIndices; Index += 3)
    {
        uint32 VertexIndex0 = IndexBuffer.Get(Index);
        uint32 VertexIndex1 = IndexBuffer.Get(Index + 1);
        uint32 VertexIndex2 = IndexBuffer.Get(Index + 2);

#if ENGINE_MAJOR_VERSION >= 5
        FVector2f UV0 = VertexBuffer.GetVertexUV(VertexIndex0, UVIndex);
        FVector2f UV1 = VertexBuffer.GetVertexUV(VertexIndex1, UVIndex);
        FVector2f UV2 = VertexBuffer.GetVertexUV(VertexIndex2, UVIndex);
#else
        FVector2D UV0 = VertexBuffer.GetVertexUV(VertexIndex0, UVIndex);
        FVector2D UV1 = VertexBuffer.GetVertexUV(VertexIndex1, UVIndex);
        FVector2D UV2 = VertexBuffer.GetVertexUV(VertexIndex2, UVIndex);
#endif

        // Add triangle edges to UV segments array
        UVSegments.Add(UV0);
        UVSegments.Add(UV1);
        UVSegments.Add(UV1);
        UVSegments.Add(UV2);
        UVSegments.Add(UV2);
        UVSegments.Add(UV0);
    }

    // Check for intersections between segments
    for (int32 i = 0; i < UVSegments.Num(); i += 2)
    {
        for (int32 j = i + 2; j < UVSegments.Num(); j += 2)
        {
#if ENGINE_MAJOR_VERSION >= 5
            if (AreSegmentsIntersecting(
                FVector2D(UVSegments[i]),      // Explicit conversion to FVector2D
                FVector2D(UVSegments[i + 1]),  // Explicit conversion to FVector2D
                FVector2D(UVSegments[j]),      // Explicit conversion to FVector2D
                FVector2D(UVSegments[j + 1]),  // Explicit conversion to FVector2D
                tolerance))
#else
            if (AreSegmentsIntersecting(UVSegments[i], UVSegments[i + 1], UVSegments[j], UVSegments[j + 1], tolerance)) // UE4
#endif
            {
                return true; // Intersection found
            }
        }
    }

    return false; // No intersection
}

bool UAssetCheckToolBPLibrary::AreSegmentsIntersecting(const FVector2D& A1, const FVector2D& A2, const FVector2D& B1, const FVector2D& B2, float tolerance)
{
	float crossA1 = FVector2D::CrossProduct(B1 - A1, A2 - A1);
	float crossA2 = FVector2D::CrossProduct(B2 - A1, A2 - A1);
	float crossB1 = FVector2D::CrossProduct(A1 - B1, B2 - B1);
	float crossB2 = FVector2D::CrossProduct(A2 - B1, B2 - B1);

	// 检查线段是否相交
	if (crossA1 * crossA2 < -tolerance && crossB1 * crossB2 < -tolerance)
	{
		return true;
	}

	return false;
}

bool UAssetCheckToolBPLibrary::CheckUVBounds(UStaticMesh* StaticMesh, int32 LODIndex, int32 UVIndex)
{
	if (StaticMesh && LODIndex >= 0 && UVIndex >= 0 && UVIndex < StaticMesh->GetRenderData()->LODResources[LODIndex].GetNumTexCoords())
	{
		const FStaticMeshLODResources& LODResource = StaticMesh->GetRenderData()->LODResources[LODIndex];
		const FStaticMeshVertexBuffer& VertexBuffer = LODResource.VertexBuffers.StaticMeshVertexBuffer;


		for (uint32 i = 0; i < static_cast<uint32>(LODResource.GetNumVertices()); i++)
		{
#if ENGINE_MAJOR_VERSION >= 5
			const FVector2f UV = VertexBuffer.GetVertexUV(i, UVIndex); //ue5
#else
			const FVector2D UV = VertexBuffer.GetVertexUV(i, UVIndex); //ue4
#endif

			if (UV.X < 0.0f || UV.X > 1.0f || UV.Y < 0.0f || UV.Y > 1.0f)
			{
				return true;
			}
		}

	}

	return false;
}

bool UAssetCheckToolBPLibrary::CheckUVBoundsSkeletal(USkeletalMesh* SkeletalMesh, int32 LODIndex, int32 UVIndex)
{
	if (!SkeletalMesh || LODIndex < 0 || UVIndex < 0)
	{
		return false;
	}

	// Get the skeletal mesh LOD data
#if ENGINE_MAJOR_VERSION >= 5
	FSkeletalMeshLODRenderData& LODData = SkeletalMesh->GetResourceForRendering()->LODRenderData[LODIndex];
#else
	FSkeletalMeshLODRenderData& LODData = SkeletalMesh->GetResourceForRendering()->LODRenderData[LODIndex];
#endif

	// Check if the UV index is valid
	if (static_cast<uint32>(UVIndex) >= LODData.StaticVertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords())
	{
		return false;
	}

	const FStaticMeshVertexBuffer& VertexBuffer = LODData.StaticVertexBuffers.StaticMeshVertexBuffer;

	// Check all vertices
	for (uint32 i = 0; i < VertexBuffer.GetNumVertices(); i++)
	{
#if ENGINE_MAJOR_VERSION >= 5
		const FVector2f UV = VertexBuffer.GetVertexUV(i, UVIndex); // UE5
#else
		const FVector2D UV = VertexBuffer.GetVertexUV(i, UVIndex); // UE4
#endif

		// Check if UV coordinates are outside the 0-1 range
		if (UV.X < 0.0f || UV.X > 1.0f || UV.Y < 0.0f || UV.Y > 1.0f)
		{
			return true; // Found UV outside the 0-1 range
		}
	}

	return false; // All UVs are within bounds
}

TArray<FString> UAssetCheckToolBPLibrary::SplitString(const FString& SourceString, const FString& Delimiter)
{
	TArray<FString> ResultArray;
	int32 StartIndex = 0;
	int32 FoundIndex = -1;

	while ((FoundIndex = SourceString.Find(Delimiter, ESearchCase::CaseSensitive, ESearchDir::FromStart, StartIndex)) != -1)
	{
		FString Substring = SourceString.Mid(StartIndex, FoundIndex - StartIndex);
		ResultArray.Add(Substring);
		StartIndex = FoundIndex + Delimiter.Len();
	}

	FString LastSubstring = SourceString.Mid(StartIndex);
	ResultArray.Add(LastSubstring);

	return ResultArray;
}

TArray<bool> UAssetCheckToolBPLibrary::AreMeshSectionsClosed(UStaticMesh* Mesh)
{
	TArray<bool> SectionClosedArray;

	if (!Mesh || !Mesh->GetRenderData() || Mesh->GetRenderData()->LODResources.Num() == 0)
	{
		// 如果Mesh为空指针，或者RenderData为空，或者LODResources数量为0，则返回空的布尔数组
		return SectionClosedArray;
	}

	// 获取StaticMesh的LOD数据
	FStaticMeshLODResources& LODModel = Mesh->GetRenderData()->LODResources[0];

	// 获取顶点缓冲区
	const FPositionVertexBuffer& VertexBuffer = LODModel.VertexBuffers.PositionVertexBuffer;

	// 获取索引缓冲区
	TArray<uint32> Indices;
	LODModel.IndexBuffer.GetCopy(Indices);

	// 遍历所有的网格部分（section）
	for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); ++SectionIndex)
	{

		// 获取当前网格部分的三角形索引范围
		const FStaticMeshSection& Section = LODModel.Sections[SectionIndex];
		int32 StartIndex = Section.FirstIndex;
		int32 EndIndex = StartIndex + Section.NumTriangles * 3;

		// 创建一个映射，用于存储每个边的平均位置和它出现的次数
#if ENGINE_MAJOR_VERSION >= 5
		TMap<FVector3f, int32> EdgeAveragePositionCountMap; //ue5
#else
		TMap<FVector, int32> EdgeAveragePositionCountMap; //ue4
#endif


		// 遍历当前网格部分的所有三角形
		for (int32 i = StartIndex; i < EndIndex; i += 3)
		{
			if (Indices.IsValidIndex(i + 2) && Indices.IsValidIndex(i + 1) && Indices.IsValidIndex(i))
			{
#if ENGINE_MAJOR_VERSION >= 5
				//ue5
				// 获取三角形的顶点位置
				FVector3f Vertex1 = VertexBuffer.VertexPosition(Indices[i]);
				FVector3f Vertex2 = VertexBuffer.VertexPosition(Indices[i + 1]);
				FVector3f Vertex3 = VertexBuffer.VertexPosition(Indices[i + 2]);

				// 计算每条边的平均位置
				FVector3f Edge1AveragePosition = (Vertex1 + Vertex2) / 2.0f;
				FVector3f Edge2AveragePosition = (Vertex2 + Vertex3) / 2.0f;
				FVector3f Edge3AveragePosition = (Vertex3 + Vertex1) / 2.0f;
#else
				//ue4
				// 获取三角形的顶点位置
				FVector Vertex1 = VertexBuffer.VertexPosition(Indices[i]);
				FVector Vertex2 = VertexBuffer.VertexPosition(Indices[i + 1]);
				FVector Vertex3 = VertexBuffer.VertexPosition(Indices[i + 2]);
				// 计算每条边的平均位置
				FVector Edge1AveragePosition = (Vertex1 + Vertex2) / 2.0f;
				FVector Edge2AveragePosition = (Vertex2 + Vertex3) / 2.0f;
				FVector Edge3AveragePosition = (Vertex3 + Vertex1) / 2.0f;
#endif

				// 增加边的平均位置计数
				EdgeAveragePositionCountMap.FindOrAdd(Edge1AveragePosition)++;
				EdgeAveragePositionCountMap.FindOrAdd(Edge2AveragePosition)++;
				EdgeAveragePositionCountMap.FindOrAdd(Edge3AveragePosition)++;
			}
			else
			{
				// 处理索引超出范围的情况
				// 可以输出错误信息或采取其他适当的操作
			}
		}

		// 检查所有的边的平均位置出现的次数是否为2
		bool IsSectionClosed = true;
		for (auto& Elem : EdgeAveragePositionCountMap)
		{
			if (Elem.Value % 2 != 0)
			{
				// 如果有边的平均位置出现次数不为2，那么当前网格部分不是封闭的
				IsSectionClosed = false;
				break;
			}
		}

		// 将当前网格部分的封闭状态添加到布尔数组中
		SectionClosedArray.Add(IsSectionClosed);
	}

	// 返回网格部分的封闭状态布尔数组
	return SectionClosedArray;
}

struct FEdgeStartEnd
{
	FVector Start;
	FVector End;

	// 重载相等运算符
	bool operator==(const FEdgeStartEnd& Other) const
	{
		return Start == Other.Start && End == Other.End;
	}

	// 重载哈希函数
	friend uint32 GetTypeHash(const FEdgeStartEnd& Edge)
	{
		return HashCombine(GetTypeHash(Edge.Start), GetTypeHash(Edge.End));
	}
};

TArray<bool> UAssetCheckToolBPLibrary::AreMeshSectionsClosedwithOutputEdge(UStaticMesh* Mesh, TArray<FVector>& EdgeStartArray, TArray<FVector>& EdgeEndArray)
{
	TArray<bool> SectionClosedArray;

	if (!Mesh || !Mesh->GetRenderData() || Mesh->GetRenderData()->LODResources.Num() == 0)
	{
		// 如果Mesh为空指针，或者RenderData为空，或者LODResources数量为0，则返回空的布尔数组
		return SectionClosedArray;
	}

	// 获取StaticMesh的LOD数据
	FStaticMeshLODResources& LODModel = Mesh->GetRenderData()->LODResources[0];

	// 获取顶点缓冲区
	const FPositionVertexBuffer& VertexBuffer = LODModel.VertexBuffers.PositionVertexBuffer;

	// 获取索引缓冲区
	TArray<uint32> Indices;
	LODModel.IndexBuffer.GetCopy(Indices);

	// 创建一个二维数组，用于存储每个网格部分的三角形索引
	TArray<TArray<uint32>> SectionTrianglesArray;

	// 遍历所有的网格部分（section）
	for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); ++SectionIndex)
	{
		// 获取当前网格部分的三角形索引范围
		const FStaticMeshSection& Section = LODModel.Sections[SectionIndex];
		int32 StartIndex = Section.FirstIndex;
		int32 EndIndex = StartIndex + Section.NumTriangles * 3;

		// 创建一个数组，用于存储当前网格部分的三角形索引
		TArray<uint32> SectionTriangles;

		// 遍历当前网格部分的所有三角形
		for (int32 i = StartIndex; i < EndIndex; i += 3)
		{
			if (Indices.IsValidIndex(i + 2) && Indices.IsValidIndex(i + 1) && Indices.IsValidIndex(i))
			{
				// 将三角形索引添加到当前网格部分的数组中
				SectionTriangles.Add(Indices[i]);
				SectionTriangles.Add(Indices[i + 1]);
				SectionTriangles.Add(Indices[i + 2]);
			}
			else
			{
				// 处理索引超出范围的情况
				// 可以输出错误信息或采取其他适当的操作
			}
		}

		// 将当前网格部分的三角形索引数组添加到二维数组中
		SectionTrianglesArray.Add(SectionTriangles);
	}

	// 遍历每个网格部分的三角形索引数组，判断封闭性
	for (const TArray<uint32>& SectionTriangles : SectionTrianglesArray)
	{
		// 创建一个映射，用于存储每个边的起始点和结束点的最小值和最大值
		TMap<FEdgeStartEnd, int32> EdgeStartEndMap;

		// 遍历当前网格部分的所有三角形
		for (int32 i = 0; i < SectionTriangles.Num(); i += 3)
		{
#if ENGINE_MAJOR_VERSION >= 5
			//ue5
			// 获取三角形的顶点位置
			FVector Vertex1 = FVector(VertexBuffer.VertexPosition(SectionTriangles[i]));
			FVector Vertex2 = FVector(VertexBuffer.VertexPosition(SectionTriangles[i + 1]));
			FVector Vertex3 = FVector(VertexBuffer.VertexPosition(SectionTriangles[i + 2]));

			// 存储边的起始点和结束点的最小值和最大值
			FEdgeStartEnd FEdgeStartEnd1 = { FVector(VectorMinOrMax(Vertex1, Vertex2, false)), FVector(VectorMinOrMax(Vertex1, Vertex2, true)) };
			FEdgeStartEnd FEdgeStartEnd2 = { FVector(VectorMinOrMax(Vertex2, Vertex3, false)), FVector(VectorMinOrMax(Vertex2, Vertex3, true)) };
			FEdgeStartEnd FEdgeStartEnd3 = { FVector(VectorMinOrMax(Vertex3, Vertex1, false)), FVector(VectorMinOrMax(Vertex3, Vertex1, true)) };
#else
			 //ue4
			// 获取三角形的顶点位置
			FVector Vertex1 = VertexBuffer.VertexPosition(SectionTriangles[i]);
			FVector Vertex2 = VertexBuffer.VertexPosition(SectionTriangles[i + 1]);
			FVector Vertex3 = VertexBuffer.VertexPosition(SectionTriangles[i + 2]);

			// 存储边的起始点和结束点的最小值和最大值
			FEdgeStartEnd FEdgeStartEnd1 = { VectorMinOrMax(Vertex1, Vertex2,false), VectorMinOrMax(Vertex1, Vertex2,true) };
			FEdgeStartEnd FEdgeStartEnd2 = { VectorMinOrMax(Vertex2, Vertex3,false), VectorMinOrMax(Vertex2, Vertex3,true) };
			FEdgeStartEnd FEdgeStartEnd3 = { VectorMinOrMax(Vertex3, Vertex1,false), VectorMinOrMax(Vertex3, Vertex1,true) };
#endif

			// 增加边的平均位置计数
			EdgeStartEndMap.FindOrAdd(FEdgeStartEnd1)++;
			EdgeStartEndMap.FindOrAdd(FEdgeStartEnd2)++;
			EdgeStartEndMap.FindOrAdd(FEdgeStartEnd3)++;
		}

		// 检查所有的边的平均位置出现的次数是否为2
		bool IsSectionClosed = true;
		for (auto& Elem : EdgeStartEndMap)
		{
			if (Elem.Value % 2 != 0)
			{
				// 如果有边的平均位置出现次数不为2，那么当前网格部分不是封闭的
				IsSectionClosed = false;
				EdgeStartArray.Add(Elem.Key.Start);
				EdgeEndArray.Add(Elem.Key.End);
				//break;
			}
		}

		// 将当前网格部分的封闭状态添加到布尔数组中
		SectionClosedArray.Add(IsSectionClosed);

	}

	// 返回网格部分的封闭状态布尔数组
	return SectionClosedArray;
}

FVector UAssetCheckToolBPLibrary::VectorMinOrMax(FVector Vector1, FVector Vector2, bool bMax)
{
	// 如果 bMax=1，返回每个分量的最大值，否则返回每个分量的最小值
	if (bMax)
	{
		return FVector(FMath::Max(Vector1.X, Vector2.X), FMath::Max(Vector1.Y, Vector2.Y), FMath::Max(Vector1.Z, Vector2.Z));
	}
	else
	{
		return FVector(FMath::Min(Vector1.X, Vector2.X), FMath::Min(Vector1.Y, Vector2.Y), FMath::Min(Vector1.Z, Vector2.Z));
	}
}

int32 UAssetCheckToolBPLibrary::GetNumTexCoords(UStaticMesh* StaticMesh, int32 LODIndex)
{
	if (StaticMesh)
	{
		// 获取静态网格的 LOD 资源
		FStaticMeshRenderData* RenderData = StaticMesh->GetRenderData();
		if (RenderData && RenderData->LODResources.IsValidIndex(LODIndex))
		{
			FStaticMeshLODResources& LODResource = RenderData->LODResources[LODIndex];
			// 返回指定 LOD 的 UV Set 数量
			return LODResource.GetNumTexCoords();
		}
	}

	// 如果无法获取到 LOD 资源或者静态网格为空，则返回默认值 -1
	return -1;
}

int32 UAssetCheckToolBPLibrary::GetNumTexCoordsForSkeletalMesh(USkeletalMesh* SkeletalMesh, int32 LODIndex)
{
	if (SkeletalMesh)
	{
		// 获取骨骼网格的渲染数据
		FSkeletalMeshRenderData* RenderData = SkeletalMesh->GetResourceForRendering();
		if (RenderData && RenderData->LODRenderData.IsValidIndex(LODIndex))
		{
			FSkeletalMeshLODRenderData& LODResource = RenderData->LODRenderData[LODIndex];
			// 返回指定 LOD 的 UV Set 数量
			return LODResource.GetNumTexCoords();
		}
	}

	// 如果无法获取到 LOD 资源或者骨骼网格为空，则返回默认值 -1
	return -1;
}

void UAssetCheckToolBPLibrary::SetMaterialInstanceSwitchParameterValue(UMaterialInstance* Instance, FName ParameterName, bool SwitchValue, bool bOverride)
{
	TArray<FGuid> Guids;
	TArray<FMaterialParameterInfo> OutParameterInfo;
	Instance->GetAllStaticSwitchParameterInfo(OutParameterInfo, Guids);
	FStaticParameterSet StaticParameters = Instance->GetStaticParameters();

	for (int32 ParameterIdx = 0; ParameterIdx < OutParameterInfo.Num(); ParameterIdx++)
	{
		const FMaterialParameterInfo& ParameterInfo = OutParameterInfo[ParameterIdx];
		const FGuid ExpressionId = Guids[ParameterIdx];
		if (ParameterInfo.Name == ParameterName)
		{
			FStaticSwitchParameter* NewParameter =
				new (StaticParameters.StaticSwitchParameters) FStaticSwitchParameter(ParameterInfo, SwitchValue, bOverride, ExpressionId);
			break;
		}
	}
	Instance->UpdateStaticPermutation(StaticParameters);
}

void UAssetCheckToolBPLibrary::GetMaterialInstanceSwitchParameterValue(UMaterialInstance* Instance, FName ParameterName, bool& SwitchValue, bool& IsOverride, bool bOveriddenOnly, bool bCheckParent)
{
#if ENGINE_MAJOR_VERSION >= 5
	//ue5
	// 检查 Instance 是否为空
	if (Instance == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Material Instance is null."));
		return;
	}
	// 使用 FHashedMaterialParameterInfo 包装参数名
	FHashedMaterialParameterInfo ParameterInfo(ParameterName);

	// 用于存储表达式 GUID
	FGuid ExpressionGUID;

	// 调用 GetStaticSwitchParameterValue 获取参数值
	IsOverride = Instance->GetStaticSwitchParameterValue(ParameterInfo, SwitchValue, ExpressionGUID, bOveriddenOnly);
#else
	//ue4
		if (Instance == nullptr)
		{
			return;
		}

		FGuid ExpressionGUID;
		IsOverride = Instance->GetStaticSwitchParameterValue(ParameterName, SwitchValue, ExpressionGUID, bOveriddenOnly, bCheckParent);
#endif


	// 如果未找到参数，记录警告
	if (!IsOverride)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find static switch parameter: %s"), *ParameterName.ToString());
	}

}

bool UAssetCheckToolBPLibrary::IsBoundingBoxVisibleInViewport(const FVector& Position, const FVector& BoxExtent, float MaxDistance)
{
	// 获取当前视口客户端

	if (FEditorViewportClient* ViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient()))
	{
		// 获取当前场景视图
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
			ViewportClient->Viewport,
			ViewportClient->GetScene(),
			ViewportClient->EngineShowFlags)
			.SetRealtimeUpdate(ViewportClient->IsRealtime()));


		if (const FSceneView* SceneView = ViewportClient->CalcSceneView(&ViewFamily))
		{
			// 计算边界框的八个角点
			TArray<FVector> BoxCorners;
			BoxCorners.Add(Position + FVector(BoxExtent.X, BoxExtent.Y, BoxExtent.Z));
			BoxCorners.Add(Position + FVector(BoxExtent.X, BoxExtent.Y, -BoxExtent.Z));
			BoxCorners.Add(Position + FVector(BoxExtent.X, -BoxExtent.Y, BoxExtent.Z));
			BoxCorners.Add(Position + FVector(BoxExtent.X, -BoxExtent.Y, -BoxExtent.Z));
			BoxCorners.Add(Position + FVector(-BoxExtent.X, BoxExtent.Y, BoxExtent.Z));
			BoxCorners.Add(Position + FVector(-BoxExtent.X, BoxExtent.Y, -BoxExtent.Z));
			BoxCorners.Add(Position + FVector(-BoxExtent.X, -BoxExtent.Y, BoxExtent.Z));
			BoxCorners.Add(Position + FVector(-BoxExtent.X, -BoxExtent.Y, -BoxExtent.Z));

			// 检查边界框的任意一个角点是否在当前场景视图中可见，并且距离不超过最大距离
			for (const FVector& Corner : BoxCorners)
			{
				float Distance = FVector::Dist(Corner, SceneView->ViewLocation);
				if (SceneView->ViewFrustum.IntersectBox(Corner, BoxExtent) && Distance <= MaxDistance)
				{
					// 边界框的至少一个角点在视口中可见且距离不超过最大距离
					return true;
				}
			}
		}
	}

	// 边界框在视口中不可见或距离超过最大距离
	return false;
}


TArray<AInstancedFoliageActor*> UAssetCheckToolBPLibrary::GetVisibleFoliageActorsInViewport()
{
	TArray<AInstancedFoliageActor*> VisibleFoliageActors;

	// 获取当前视口客户端
	FEditorViewportClient* ViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
	if (!ViewportClient)
	{
		return VisibleFoliageActors;
	}

	// 获取当前场景视图
	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
		ViewportClient->Viewport,
		ViewportClient->GetScene(),
		ViewportClient->EngineShowFlags)
		.SetRealtimeUpdate(ViewportClient->IsRealtime()));

	FSceneView* SceneView = ViewportClient->CalcSceneView(&ViewFamily);
	if (!SceneView)
	{
		return VisibleFoliageActors;
	}

	// 遍历场景中的所有 FoliageActor
	for (TActorIterator<AInstancedFoliageActor> It(ViewportClient->GetWorld()); It; ++It)
	{
		AInstancedFoliageActor* FoliageActor = *It;
		bool bIsVisible = false;

		// 遍历 FoliageActor 的所有组件
		for (UActorComponent* Component : FoliageActor->GetComponents())
		{

			if (UFoliageInstancedStaticMeshComponent* MeshComponent = Cast<UFoliageInstancedStaticMeshComponent>(Component))
			{
				// 遍历所有实例
				for (int32 InstanceIndex = 0; InstanceIndex < MeshComponent->GetInstanceCount(); ++InstanceIndex)
				{
					FTransform InstanceTransform;
					MeshComponent->GetInstanceTransform(InstanceIndex, InstanceTransform, true);

					FVector InstanceLocation = InstanceTransform.GetLocation();
					if (SceneView->ViewFrustum.IntersectSphere(InstanceLocation, 0.0f))
					{
						bIsVisible = true;
						break;
					}
				}
			}

			if (bIsVisible)
			{
				break;
			}
		}

		if (bIsVisible)
		{
			VisibleFoliageActors.Add(FoliageActor);
		}
	}

	return VisibleFoliageActors;
}









TArray<UFoliageType*> UAssetCheckToolBPLibrary::FindFoliageTypesForStaticMesh(UStaticMesh* TargetStaticMesh)
{
	TArray<UFoliageType*> FoliageTypes;

	if (!TargetStaticMesh)
	{
		return FoliageTypes;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

#if ENGINE_MAJOR_VERSION >= 5
	//ue5
	// 获取所有 UFoliageType_InstancedStaticMesh 类型的资产
	TArray<FAssetData> AssetDataArray;
	FTopLevelAssetPath ClassPath = UFoliageType_InstancedStaticMesh::StaticClass()->GetClassPathName();
	AssetRegistry.GetAssetsByClass(ClassPath, AssetDataArray);
#else
	 //ue4
		TArray<FAssetData> AssetDataArray;
		AssetRegistry.GetAssetsByClass(UFoliageType_InstancedStaticMesh::StaticClass()->GetFName(), AssetDataArray);
#endif

	AssetDataArray.FilterByPredicate([&](const FAssetData& AssetData) {
		UFoliageType_InstancedStaticMesh* FoliageType = Cast<UFoliageType_InstancedStaticMesh>(AssetData.GetAsset());
		return FoliageType && FoliageType->GetStaticMesh() && FoliageType->GetStaticMesh() == TargetStaticMesh;
		});

	for (const FAssetData& AssetData : AssetDataArray)
	{

		if (UFoliageType_InstancedStaticMesh* FoliageType = Cast<UFoliageType_InstancedStaticMesh>(AssetData.GetAsset()))
		{
			FoliageTypes.Add(FoliageType);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to cast FAssetData to UFoliageType_InstancedStaticMesh."));
		}
	}

	return FoliageTypes;
}






void UAssetCheckToolBPLibrary::SetFoliageTypeCullingDistance(UFoliageType* FoliageType, float distance)
{
	if (!FoliageType)
	{
		return;
	}

	UFoliageType_InstancedStaticMesh* FoliageType_ISM = Cast<UFoliageType_InstancedStaticMesh>(FoliageType);

	if (!FoliageType_ISM)
	{
		return;
	}


}













bool UAssetCheckToolBPLibrary::SaveObjectArrayToFile(const TArray<UObject*>& ObjectArray, const FString& FilePath, UClass* ObjectType)
{
	FString SaveData;
	for (UObject* Object : ObjectArray)
	{
		if (Object && Object->IsA(ObjectType))
		{
			SaveData += Object->GetPathName() + TEXT("\n");
		}
	}

	return FFileHelper::SaveStringToFile(SaveData, *FilePath);
}

TArray<UObject*> UAssetCheckToolBPLibrary::LoadObjectArrayFromFile(const FString& FilePath, TSubclassOf<UObject> ObjectType)
{
	FString LoadData;
	FFileHelper::LoadFileToString(LoadData, *FilePath);

	TArray<UObject*> ObjectArray;

	TArray<FString> Array;
	LoadData.ParseIntoArray(Array, TEXT("\n"), true);
	for (const FString& ObjectPath : Array)
	{

		if (UObject* Object = StaticLoadObject(ObjectType, nullptr, *ObjectPath, nullptr, LOAD_None, nullptr))
		{
			ObjectArray.Add(Object);
		}
	}

	return ObjectArray;
}


void UAssetCheckToolBPLibrary::ParseAndPrintKeyValuePairs(const FString& InputString)
{
	TArray<FString> KeyValuePairs;
	InputString.ParseIntoArray(KeyValuePairs, TEXT("),("), true);

	for (const FString& KeyValuePair : KeyValuePairs)
	{
		FString Key;
		FString Value;

		if (KeyValuePair.Split(TEXT("="), &Key, &Value))
		{
			// 去除引号和括号
			Key.RemoveFromStart(TEXT("name_"));
			Key.RemoveFromEnd(TEXT("_877874594E53D63BE3E4F49CDBB98EBC"));
			Value.RemoveFromStart(TEXT("\""));
			Value.RemoveFromEnd(TEXT(")"));

		}
	}
}

TArray<FString> UAssetCheckToolBPLibrary::GetDataTablePropertyNames(UDataTable* DataTable)
{
	TArray<FString> PropertyNames;

	if (DataTable)
	{

		if (const UScriptStruct* RowStruct = DataTable->GetRowStruct())
		{
			for (TFieldIterator<FProperty> PropertyIt(RowStruct); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				FString FullPropertyName = Property->GetNameCPP();
				int32 LastUnderscore;
				if (FullPropertyName.FindLastChar('_', LastUnderscore))
				{
					int32 SecondLastUnderscore;
					if (FullPropertyName.Left(LastUnderscore).FindLastChar('_', SecondLastUnderscore))
					{
						// 保留最后两个下划线之前的部分
						FullPropertyName = FullPropertyName.Left(SecondLastUnderscore);
					}
				}
				PropertyNames.Add(FullPropertyName);
			}
		}
	}

	return PropertyNames;
}

bool UAssetCheckToolBPLibrary::SetMICScalarParam_withoutUpdata(UMaterialInstanceConstant* Material, FString ParamName, float Value)
{
#if WITH_EDITOR
	FName Name = FName(*ParamName);

	if (Material != nullptr)
	{
		Material->SetScalarParameterValueEditorOnly(Name, Value);
		//UpdateMIC(Material);

		return true;
	}
#endif
	return false;
}

bool UAssetCheckToolBPLibrary::SetMICVectorParam_withoutUpdata(UMaterialInstanceConstant* Material, FString ParamName, FLinearColor Value)
{
#if WITH_EDITOR
	FName Name = FName(*ParamName);

	if (Material != nullptr)
	{
		Material->SetVectorParameterValueEditorOnly(Name, Value);
		//UpdateMIC(Material);

		return true;
	}

#endif
	return false;
}

bool UAssetCheckToolBPLibrary::SetMICTextureParam_withoutUpdata(UMaterialInstanceConstant* Material, FString ParamName, UTexture2D* Texture)
{
#if WITH_EDITOR
	FName Name = FName(*ParamName);

	if (Material != nullptr)
	{
		Material->SetTextureParameterValueEditorOnly(Name, Texture);
		//UpdateMIC(Material);

		return true;
	}

#endif
	return false;
}

/*
void UAssetCheckToolBPLibrary::SetAllFoliageCullingMaskBR(bool DC0, bool DC1, bool DC2, bool SC0, bool SC1, bool SC2, bool SC3)
{
	uint64 FoliageCullingMask = 0ull;

	if (DC0 == 1)
	{
		FoliageCullingMask = FoliageCullingMask ^ (1ull << 0 << 32);
	}
	else
	{
		FoliageCullingMask = FoliageCullingMask ^ (0ull << 0 << 32);
	}

	if (DC1 == 1)
	{
		FoliageCullingMask = FoliageCullingMask ^ (1ull << 1 << 32);
	}
	else
	{
		FoliageCullingMask = FoliageCullingMask ^ (0ull << 1 << 32);
	}


	if (DC2 == 1)
	{
		FoliageCullingMask = FoliageCullingMask ^ (1ull << 2 << 32);
	}
	else
	{
		FoliageCullingMask = FoliageCullingMask ^ (0ull << 2 << 32);
	}

	if (SC0 == 1)
	{
		FoliageCullingMask = FoliageCullingMask ^ (1ull << 0 );
	}
	else
	{
		FoliageCullingMask = FoliageCullingMask ^ (0ull << 0 );
	}

	if (SC1 == 1)
	{
		FoliageCullingMask = FoliageCullingMask ^ (1ull << 1);
	}
	else
	{
		FoliageCullingMask = FoliageCullingMask ^ (0ull << 1);
	}

	if (SC2 == 1)
	{
		FoliageCullingMask = FoliageCullingMask ^ (1ull << 2);
	}
	else
	{
		FoliageCullingMask = FoliageCullingMask ^ (0ull << 2);
	}

	if (SC3 == 1)
	{
		FoliageCullingMask = FoliageCullingMask ^ (1ull << 3);
	}
	else
	{
		FoliageCullingMask = FoliageCullingMask ^ (0ull << 3);
	}
	if (GCurrentLevelEditingViewportClient)
	{
		GCurrentLevelEditingViewportClient->FoliageCullingMask = FoliageCullingMask;
	}


}

*/

void UAssetCheckToolBPLibrary::FindReferencesInScene(UStaticMesh* TargetMesh, TArray<AActor*>& OutActorArray, TArray<AActor*>& OutFoliageActorArray)
{
	// 创建一个数组来存储找到的Actor
	TArray<AActor*> ReferencingActors;

	for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;

		// 遍历Actor的所有组件
		for (UActorComponent* Component : Actor->GetComponents())
		{
			// 检查组件是否是UStaticMeshComponent
			if (UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(Component))
			{
				// 检查MeshComponent是否引用了TargetMesh
				if (MeshComponent->GetStaticMesh() == TargetMesh)
				{
					// 将Actor添加到数组中
					if (Cast<AInstancedFoliageActor>(Actor))
					{
						OutFoliageActorArray.Add(Actor);
					}
					else
					{
						OutActorArray.Add(Actor);
					}

				}
			}
		}
	}
}

void UAssetCheckToolBPLibrary::FindReferencesInViewport(UStaticMesh* TargetMesh, TArray<AActor*>& OutActorArray, TArray<AActor*>& OutFoliageActorArray, float MaxDistance)
{
    // Get the current viewport client

	if (FEditorViewportClient* ViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient()))
    {
        // Get the current scene view
        FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
            ViewportClient->Viewport,
            ViewportClient->GetScene(),
            ViewportClient->EngineShowFlags)
            .SetRealtimeUpdate(ViewportClient->IsRealtime()));


        if (const FSceneView* SceneView = ViewportClient->CalcSceneView(&ViewFamily))
        {
            // Iterate over all actors in the world
            for (TActorIterator<AActor> ActorItr(ViewportClient->GetWorld()); ActorItr; ++ActorItr)
            {
                // Check if the actor is within the maximum distance and visible in the current scene view
                FVector Origin, BoxExtent;
                ActorItr->GetActorBounds(true, Origin, BoxExtent);
                if (FVector::Dist(Origin, SceneView->ViewLocation) <= MaxDistance &&
                    SceneView->ViewFrustum.IntersectBox(Origin, BoxExtent))
                {
                    // If the actor is within the maximum distance and visible, check if it references the target mesh
                    AActor* Actor = *ActorItr;
                    for (UActorComponent* Component : Actor->GetComponents())
                    {
                        // Check if the component is a UStaticMeshComponent
                        if (UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(Component))
                        {
                            // Check if the MeshComponent references the TargetMesh
                            if (MeshComponent->GetStaticMesh() == TargetMesh)
                            {
                                // Add the Actor to the appropriate output array
                                if (Cast<AInstancedFoliageActor>(Actor))
                                {
                                    OutFoliageActorArray.Add(Actor);
                                }
                                else
                                {
                                    OutActorArray.Add(Actor);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


void UAssetCheckToolBPLibrary::FindReferencesNumInScene(UStaticMesh* TargetMesh, int32& OutActorCount, int32& OutFoliageActorCount, int32& OutInstanceCount)
{
	OutActorCount = 0;
	OutFoliageActorCount = 0;
	OutInstanceCount = 0;

	// Iterate over all AActor objects
	for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{

		if (AActor* CurrentActor = *ActorItr)
		{
			if (AInstancedFoliageActor* InstancedFoliageActor = Cast<AInstancedFoliageActor>(CurrentActor))
			{
				bool bMatchFound = false;
				TArray<UInstancedStaticMeshComponent*> InstancedComponents;
				InstancedFoliageActor->GetComponents<UInstancedStaticMeshComponent>(InstancedComponents);
				for (UInstancedStaticMeshComponent* Component : InstancedComponents)
				{
					if (Component->GetStaticMesh() == TargetMesh)
					{
						OutInstanceCount += Component->GetInstanceCount();
						bMatchFound = true;
					}
				}
				if (bMatchFound)
				{
					OutFoliageActorCount++;
				}
			}
			else
			{

				for (UActorComponent* Component : CurrentActor->GetComponents())
				{
					// Check if the component is a UStaticMeshComponent
					if (UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(Component))
					{
						// Check if MeshComponent references the TargetMesh
						if (MeshComponent->GetStaticMesh() == TargetMesh)
						{
							// Add the actor to the count
							OutActorCount++;

						}
					}
				}

			}
		}
	}
}

void UAssetCheckToolBPLibrary::SetViewMode(EViewModeIndex viewmode)
{
	if (GCurrentLevelEditingViewportClient)
	{
	GCurrentLevelEditingViewportClient->SetViewMode(viewmode);
	}
}

void UAssetCheckToolBPLibrary::GetViewportInfos(FVector& OutLocation, FRotator& OutRotation, float& OutFOV)
{
	// 获取当前视口客户端

	if (FEditorViewportClient* ViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient()))
	{
		// 获取当前场景视图
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
			ViewportClient->Viewport,
			ViewportClient->GetScene(),
			ViewportClient->EngineShowFlags)
			.SetRealtimeUpdate(ViewportClient->IsRealtime()));



		if (const FSceneView* SceneView = ViewportClient->CalcSceneView(&ViewFamily))
		{
			// 获取视口的位置
			OutLocation = SceneView->ViewLocation;

			// 获取视口的旋转
			OutRotation = ViewportClient->GetViewRotation();

			// 获取视口的视野（FOV）
			OutFOV = ViewportClient->ViewFOV; // Use ViewFOV from FEditorViewportClient
		}
	}
}

void UAssetCheckToolBPLibrary::SetViewportFOV(float FOV)
{
	// 获取当前视口客户端

	if (FEditorViewportClient* ViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient()))
	{
		// 获取当前场景视图
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
			ViewportClient->Viewport,
			ViewportClient->GetScene(),
			ViewportClient->EngineShowFlags)
			.SetRealtimeUpdate(ViewportClient->IsRealtime()));


		// 获取视口的视野（FOV）
		ViewportClient->ViewFOV = FOV; // Use ViewFOV from FEditorViewportClient
	}
}



void UAssetCheckToolBPLibrary::GetFoliageType(UStaticMesh* TargetMesh, TArray<UFoliageType*>& OutFoliageTypes, bool FindSameNameFoliage)
{
	if (!TargetMesh)
	{
		return;
	}

#if ENGINE_MAJOR_VERSION >= 5
	//ue5
	// 获取 AssetRegistry 模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	// 搜索所有资产
	AssetRegistry.SearchAllAssets(true);
	// 获取所有 UFoliageType 类型的资产
	TArray<FAssetData> AssetDataArray;
	FTopLevelAssetPath ClassPath = UFoliageType::StaticClass()->GetClassPathName();
	AssetRegistry.GetAssetsByClass(ClassPath, AssetDataArray, true);
#else
	 //ue4
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().SearchAllAssets(true);

		TArray<FAssetData> AssetDataArray;
		AssetRegistryModule.Get().GetAssetsByClass(UFoliageType::StaticClass()->GetFName(), AssetDataArray, true);
#endif


	// 根据模型名加上"_FoliageType"来查找UFoliageType资源
	FString FoliageTypeName = TargetMesh->GetName() + "_FoliageType";

	// 遍历所有找到的FoliageType资源
	for (const FAssetData& AssetData : AssetDataArray)
	{
		// 获取FoliageType资源的StaticMesh属性
		UStaticMesh* StaticMesh = nullptr;
		if (UObject* FoliageTypeObject = AssetData.GetAsset())
		{
			if (UFoliageType* FoliageType = Cast<UFoliageType>(FoliageTypeObject))
			{
				StaticMesh = Cast<UStaticMesh>(FoliageType->GetSource());
			}
		}
		// 如果找到了StaticMesh属性，并且它的值和我们要找的StaticMesh相同
		if (StaticMesh && StaticMesh->GetPathName() == TargetMesh->GetPathName())
		{
			// 找到了对应的FoliageType，可以在这里进行你想要的操作
			// 注意，这里只是找到了FoliageType的元数据，如果你想操作FoliageType，可能需要加载它到内存中

			if (UFoliageType* FoliageType = Cast<UFoliageType>(AssetData.GetAsset()))
			{
				if (FindSameNameFoliage)
				{
					if (FoliageType->GetName() == FoliageTypeName)
					{
						OutFoliageTypes.Add(FoliageType);
						return;
					}
					else
					{
						OutFoliageTypes.Add(FoliageType);
					}
				}
				else
				{
					OutFoliageTypes.Add(FoliageType);
				}
			}
		}
	}
}

void UAssetCheckToolBPLibrary::CalculateAverageVertexColorAndMaterialPerSection(UStaticMesh* Mesh, int32 LODIndex, TArray<FColor>& AverageColors, TArray<UMaterialInterface*>& Materials)
{
	AverageColors.Empty();
	Materials.Empty();

	if (!Mesh || !Mesh->GetRenderData() || Mesh->GetRenderData()->LODResources.Num() <= LODIndex)
	{
		return;
	}


	FStaticMeshLODResources& LODModel = Mesh->GetRenderData()->LODResources[LODIndex];
	if (!LODModel.Sections.Num())
	{
		return;
	}

	const FColorVertexBuffer& ColorBuffer = LODModel.VertexBuffers.ColorVertexBuffer;
	if (!ColorBuffer.GetNumVertices())
	{
		return;
	}

	TArray<uint32> Indices;
	LODModel.IndexBuffer.GetCopy(Indices);
	if (!Indices.Num())
	{
		return;
	}

	for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); ++SectionIndex)
	{
		const FStaticMeshSection& Section = LODModel.Sections[SectionIndex];
		int32 StartIndex = Section.FirstIndex;
		int32 EndIndex = StartIndex + Section.NumTriangles * 3;
		UMaterialInterface* Material = Mesh->GetMaterial(Section.MaterialIndex);
		Materials.Add(Material);

		FLinearColor ColorAccumulator = FLinearColor::Black;
		int32 VertexCount = 0;

		for (int32 i = StartIndex; i < EndIndex; i += 3)
		{
			if (Indices.IsValidIndex(i + 2) && Indices.IsValidIndex(i + 1) && Indices.IsValidIndex(i))
			{
				ColorAccumulator += ColorBuffer.VertexColor(Indices[i]).ReinterpretAsLinear();
				ColorAccumulator += ColorBuffer.VertexColor(Indices[i + 1]).ReinterpretAsLinear();
				ColorAccumulator += ColorBuffer.VertexColor(Indices[i + 2]).ReinterpretAsLinear();
				VertexCount += 3;
			}
			else
			{
				// Handle out of range indices
			}
		}

		FLinearColor AverageColor = ColorAccumulator / VertexCount;
		AverageColors.Add(AverageColor.ToFColor(false));
		UE_LOG(LogTemp, Log, TEXT("Section %s AverageColor is %s"), *Material->GetName(), *AverageColor.ToFColor(false).ToString());
	}
}





void UAssetCheckToolBPLibrary::CalculateAverageVertexColorAndMaterialPerSectionWithArea(UStaticMesh* Mesh, int32 LODIndex, TArray<FColor>& AverageColors, TArray<UMaterialInterface*>& Materials)
{
	AverageColors.Empty();
	Materials.Empty();

	if (!Mesh || !Mesh->GetRenderData() || Mesh->GetRenderData()->LODResources.Num() <= LODIndex)
	{
		return;
	}

	// Get all materials from the mesh and add them to the Materials array


	FStaticMeshLODResources& LODModel = Mesh->GetRenderData()->LODResources[LODIndex];
	if (!LODModel.Sections.Num())
	{
		return;
	}

	const FColorVertexBuffer& ColorBuffer = LODModel.VertexBuffers.ColorVertexBuffer;
	if (!ColorBuffer.GetNumVertices())
	{
		return;
	}

	TArray<uint32> Indices;
	LODModel.IndexBuffer.GetCopy(Indices);
	if (!Indices.Num())
	{
		return;
	}

	for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); ++SectionIndex)
	{
		const FStaticMeshSection& Section = LODModel.Sections[SectionIndex];
		int32 StartIndex = Section.FirstIndex;
		int32 EndIndex = StartIndex + Section.NumTriangles * 3;
		UMaterialInterface* Material = Mesh->GetMaterial(Section.MaterialIndex);
		Materials.Add(Material);


		FLinearColor ColorAccumulator = FLinearColor::Black;
		float TotalArea = 0.0f;

		for (int32 i = StartIndex; i < EndIndex; i += 3)
		{
			if (Indices.IsValidIndex(i + 2) && Indices.IsValidIndex(i + 1) && Indices.IsValidIndex(i))
			{
#if ENGINE_MAJOR_VERSION >= 5
				//ue5
				// 获取三角形的顶点位置（FVector3f）
				FVector3f P0 = LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(Indices[i]);
				FVector3f P1 = LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(Indices[i + 1]);
				FVector3f P2 = LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(Indices[i + 2]);
				// 计算三角形的面积
				float Area = FVector3f::CrossProduct(P1 - P0, P2 - P0).Size() * 0.5f;
#else
				//ue4
				FVector P0 = LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(Indices[i]);
				FVector P1 = LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(Indices[i + 1]);
				FVector P2 = LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(Indices[i + 2]);
				// Calculate the area of the triangle
				float Area = FVector::CrossProduct(P1 - P0, P2 - P0).Size() * 0.5f;
#endif

				TotalArea += Area;
				// Accumulate the color weighted by the area
				FLinearColor TriangleColor = (ColorBuffer.VertexColor(Indices[i]).ReinterpretAsLinear() +
					ColorBuffer.VertexColor(Indices[i + 1]).ReinterpretAsLinear() +
					ColorBuffer.VertexColor(Indices[i + 2]).ReinterpretAsLinear()) / 3.0f;

				ColorAccumulator += TriangleColor * Area;
			}
			else
			{
				// Handle out of range indices
			}

		}

		FLinearColor AverageColor = ColorAccumulator / TotalArea;
		AverageColors.Add(AverageColor.ToFColor(false));

		UE_LOG(LogTemp, Log, TEXT("Section %s AverageColor is %s"), *Material->GetName(), *AverageColor.ToFColor(false).ToString());
	}
}
/*
void UAssetCheckToolBPLibrary::SelectAssetInContentFolder(UStaticMesh* Mesh)
{


}*/

void UAssetCheckToolBPLibrary::ConvertFoliageToActor(AInstancedFoliageActor* InstancedFoliageActor)
{
    if (!InstancedFoliageActor) return;

    UWorld* World = InstancedFoliageActor->GetWorld();
    if (!World) return;

    //TArray<UActorComponent*> Components = InstancedFoliageActor->GetComponentsByClass(UInstancedStaticMeshComponent::StaticClass()); //废弃
	// 使用 TArray 来存储组件
	TArray<UActorComponent*> Components;
	// 获取 UInstancedStaticMeshComponent 类型的组件
	InstancedFoliageActor->GetComponents(UInstancedStaticMeshComponent::StaticClass(), Components);



    // Get the name of the InstancedFoliageActor for the folder
    FName FoliageActorName = InstancedFoliageActor->GetFName();

    // 遍历所有的UInstancedStaticMeshComponent
    for (UActorComponent* ActorComponent : Components)
    {
        UInstancedStaticMeshComponent* Component = Cast<UInstancedStaticMeshComponent>(ActorComponent);
        if (!Component) continue;

        // Get the name of the StaticMesh for the folder
        FName StaticMeshName = Component->GetStaticMesh()->GetFName();

        // Create the folder name
        FName FolderName = FName(*(StaticMeshName.ToString() + "_" + FoliageActorName.ToString()));

        // 遍历每个实例
        for (int32 i = 0; i < Component->GetInstanceCount(); i++)
        {
            FTransform InstanceTransform;
            Component->GetInstanceTransform(i, InstanceTransform, true);

            // 创建新的Actor
            AActor* NewActor = World->SpawnActor<AActor>(InstanceTransform.GetLocation(), InstanceTransform.GetRotation().Rotator());
            if (!NewActor) continue;

            // Add the name of the source FoliageActor as a tag
            NewActor->Tags.Add(FoliageActorName);

            // 创建新的StaticMeshComponent并将其附加到新的Actor
            UStaticMeshComponent* NewComponent = NewObject<UStaticMeshComponent>(NewActor);
            NewComponent->SetStaticMesh(Component->GetStaticMesh());
            NewComponent->SetWorldTransform(InstanceTransform);
            NewComponent->AttachToComponent(NewActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
            NewComponent->RegisterComponent();

            // Set the folder path for the new actor
            NewActor->SetFolderPath(FolderName);
        }
    }
}

void UAssetCheckToolBPLibrary::ConvertActorsToFoliageByFolder( FName FolderName)
{
	//Get Editor World
	UWorld* World = GEditor->GetEditorWorldContext().World();

	if (!World) return;

	// Create a new InstancedFoliageActor
	AInstancedFoliageActor* NewInstancedFoliageActor = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(World, true);

	// Iterate over all actors in the world
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;

		// Check if the actor is in the specified folder
		if (Actor->GetFolderPath() == FolderName)
		{
			// Get the StaticMeshComponent
			UStaticMeshComponent* Component = Actor->FindComponentByClass<UStaticMeshComponent>();
			if (!Component) continue;

			// Create a new UInstancedStaticMeshComponent
			UInstancedStaticMeshComponent* InstancedMeshComponent = NewObject<UInstancedStaticMeshComponent>(NewInstancedFoliageActor);
			InstancedMeshComponent->SetStaticMesh(Component->GetStaticMesh());

#if ENGINE_MAJOR_VERSION >= 5
			FTransform ComponentTransform = Component->GetComponentTransform();
			InstancedMeshComponent->AddInstance(ComponentTransform, true);
#else
			InstancedMeshComponent->AddInstanceWorldSpace(Component->GetComponentTransform());
#endif


			// Add the UInstancedStaticMeshComponent to the InstancedFoliageActor
			NewInstancedFoliageActor->AddInstanceComponent(InstancedMeshComponent);
			InstancedMeshComponent->AttachToComponent(NewInstancedFoliageActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);

			// Register the component and apply the changes
			InstancedMeshComponent->RegisterComponentWithWorld(World);
			NewInstancedFoliageActor->MarkComponentsRenderStateDirty();

			// Destroy the actor
			Actor->Destroy();
		}
	}
}


void UAssetCheckToolBPLibrary::ConvertFoliageToActorWithArray(AInstancedFoliageActor* InstancedFoliageActor, TArray<UObject*> MeshesToConvert, TArray<UFoliageType*> FoliageTypes, bool bDestroy,TArray<AActor*>& NewActors)
#if ENGINE_MAJOR_VERSION >= 5
//ue5
{
    if (!InstancedFoliageActor) return;

    UWorld* World = InstancedFoliageActor->GetWorld();
    if (!World) return;

    // 获取 InstancedFoliageActor 的 Level
        ULevel* Level = InstancedFoliageActor->GetLevel();
    if (!Level) return;

    // 获取所有 UInstancedStaticMeshComponent 组件
    //TArray<UActorComponent*> Components;
    //InstancedFoliageActor->GetComponents(UFoliageInstancedStaticMeshComponent::StaticClass(), Components);

        TArray<UActorComponent*> Components = InstancedFoliageActor->GetInstanceComponents();
    // 获取 InstancedFoliageActor 的名称用于文件夹路径
    FName FolderName = InstancedFoliageActor->GetFName();

    // 获取 Level 的名称
    FString LevelName = Level->GetOutermost()->GetName();

    // 遍历所有的 UInstancedStaticMeshComponent
    for (UActorComponent* ActorComponent : Components)
    {
	    UStaticMeshComponent* Component = Cast<UStaticMeshComponent>(ActorComponent);
    	if (!Component) continue;


    	// 检查组件的 StaticMesh 是否在 MeshesToConvert 数组中


    	UFoliageInstancedStaticMeshComponent* InstanceComponent = Cast<UFoliageInstancedStaticMeshComponent>(ActorComponent);
    	//UFoliageInstancedCrossStaticMeshPlaceholder* CrossInstanceComponent = Cast<UFoliageInstancedCrossStaticMeshPlaceholder>(ActorComponent);


    	/*if(CrossInstanceComponent)
    	{
    		// 检查组件的 StaticMesh 是否在 MeshesToConvert 数组中
    		int32 MeshIndex = MeshesToConvert.IndexOfByKey(CrossInstanceComponent->GetCrossStaticMesh());
    		if (MeshIndex == INDEX_NONE) continue;
    		UFoliageType* FoliageType = FoliageTypes.IsValidIndex(MeshIndex) ? FoliageTypes[MeshIndex] : nullptr;
    		// 遍历每个实例
    		for (int32 i = 0; i < CrossInstanceComponent->GetInstanceCount(); i++)
    		{
    			FTransform InstanceTransform;
    			CrossInstanceComponent->GetInstanceTransform(i, InstanceTransform, true);

    			// 创建新的 StaticMeshActor
    			ACrossStaticMeshActor* NewActor = World->SpawnActor<ACrossStaticMeshActor>(InstanceTransform.GetLocation(), InstanceTransform.GetRotation().Rotator());
    			if (!NewActor) continue;

    			// 设置新的 StaticMeshActor 的 StaticMesh
    			NewActor->SetCrossStaticMesh(CrossInstanceComponent->GetCrossStaticMesh());

    			NewActor->SetActorTransform(InstanceTransform);

    			// 设置新 Actor 的文件夹路径
    			FString NewFolderPath = LevelName + "/" + FolderName.ToString() + "_" + CrossInstanceComponent->GetStaticMesh()->GetFName().ToString();
    			NewActor->SetFolderPath(FName(*NewFolderPath));

    			// 添加标签到新 Actor
    			NewActor->Tags.Add(FName(*LevelName));
    			NewActor->Tags.Add(FName(*InstancedFoliageActor->GetName()));

    			// 如果存在 FoliageType，添加对应的标签
    			if (FoliageType)
    			{
    				//NewActor->Tags.Add(FName(*FoliageType->GetName()));
    				NewActor->Tags.Add(FName(*FoliageType->GetPathName()));
    				NewActor->SetActorLabel(*FoliageType->GetName());
    			}

    			NewActors.Add(NewActor);
    			UE_LOG(LogTemp, Log, TEXT("Convert %s From %s"), *NewActor->GetName(), *InstancedFoliageActor->GetName());
    		}

    		// 如果 bDestroy 为 true，移除组件
    		if (bDestroy && InstanceComponent && FoliageType)
    		{
    			FFoliageInfo* FoliageInfo = InstancedFoliageActor->FindInfo(FoliageType);
    			if (FoliageInfo)
    			{
    				// 创建实例索引数组
    				TArray<int32> InstancesIndices;
    				for (int32 i = 0; i < FoliageInfo->Instances.Num(); i++)
    				{
    					InstancesIndices.Add(i);
    				}

    				// 将 TArray<int32> 转换为 TArrayView<const int32>
    				TArrayView<const int32> InstancesIndicesView(InstancesIndices);

    				// 移除实例并重建植被树
    				FoliageInfo->RemoveInstances(InstancesIndicesView, true);
    				InstanceComponent->DestroyComponent();
    			}
    		}
    	}*/

    	if(InstanceComponent)
    	{
    		// 检查组件的 StaticMesh 是否在 MeshesToConvert 数组中
    		int32 MeshIndex = MeshesToConvert.IndexOfByKey(Component->GetStaticMesh());
    		if (MeshIndex == INDEX_NONE) continue;
    		UFoliageType* FoliageType = FoliageTypes.IsValidIndex(MeshIndex) ? FoliageTypes[MeshIndex] : nullptr;
    		// 遍历每个实例
    		for (int32 i = 0; i < InstanceComponent->GetInstanceCount(); i++)
    		{
    			FTransform InstanceTransform;
    			InstanceComponent->GetInstanceTransform(i, InstanceTransform, true);

    			// 创建新的 StaticMeshActor
    			AStaticMeshActor* NewActor = World->SpawnActor<AStaticMeshActor>(InstanceTransform.GetLocation(), InstanceTransform.GetRotation().Rotator());
    			if (!NewActor) continue;

    			// 设置新的 StaticMeshActor 的 StaticMesh
    			NewActor->GetStaticMeshComponent()->SetStaticMesh(InstanceComponent->GetStaticMesh());
    			NewActor->GetStaticMeshComponent()->SetWorldTransform(InstanceTransform);

    			// 设置新 Actor 的文件夹路径
    			FString NewFolderPath = LevelName + "/" + FolderName.ToString() + "_" + InstanceComponent->GetStaticMesh()->GetFName().ToString();
    			NewActor->SetFolderPath(FName(*NewFolderPath));

    			// 添加标签到新 Actor
    			NewActor->Tags.Add(FName(*LevelName));
    			NewActor->Tags.Add(FName(*InstancedFoliageActor->GetName()));

    			// 如果存在 FoliageType，添加对应的标签
    			if (FoliageType)
    			{
    				//NewActor->Tags.Add(FName(*FoliageType->GetName()));
    				NewActor->Tags.Add(FName(*FoliageType->GetPathName()));
    				NewActor->SetActorLabel(*FoliageType->GetName());
    			}

    			NewActors.Add(NewActor);
    			UE_LOG(LogTemp, Log, TEXT("Convert %s From %s"), *NewActor->GetName(), *InstancedFoliageActor->GetName());
    		}

    		// 如果 bDestroy 为 true，移除组件
    		if (bDestroy && InstanceComponent && FoliageType)
    		{
    			FFoliageInfo* FoliageInfo = InstancedFoliageActor->FindInfo(FoliageType);
    			if (FoliageInfo)
    			{
    				// 创建实例索引数组
    				TArray<int32> InstancesIndices;
    				for (int32 i = 0; i < FoliageInfo->Instances.Num(); i++)
    				{
    					InstancesIndices.Add(i);
    				}

    				// 将 TArray<int32> 转换为 TArrayView<const int32>
    				TArrayView<const int32> InstancesIndicesView(InstancesIndices);

    				// 移除实例并重建植被树
    				FoliageInfo->RemoveInstances(InstancesIndicesView, true);
    				InstanceComponent->DestroyComponent();
    			}
    		}

    		//Component->DestroyComponent();
    	}
    }
    // 将新 Actor 移动到 InstancedFoliageActor 的 Level
    UEditorLevelUtils::MoveActorsToLevel(NewActors, Level, false);

    // 设置新 Actor 的标志
    for (AActor* Actor : NewActors)
    {
        // 清除所有内部标志
        Actor->ClearInternalFlags(EInternalObjectFlags::None);

        // 设置 RF_Transactional 标志
        Actor->SetFlags(RF_Transactional);
    }
}
#else
{ //ue4
    if (!InstancedFoliageActor) return;

    UWorld* World = InstancedFoliageActor->GetWorld();
    if (!World) return;

    // Get the level of the InstancedFoliageActor
    ULevel* Level = InstancedFoliageActor->GetLevel();
    if (!Level) return;

    //TArray<UActorComponent*> Components = InstancedFoliageActor->GetComponentsByClass(UInstancedStaticMeshComponent::StaticClass()); //废弃

	// 使用 TArray 来存储组件
	TArray<UActorComponent*> Components;
	// 获取 UInstancedStaticMeshComponent 类型的组件
	InstancedFoliageActor->GetComponents(UInstancedStaticMeshComponent::StaticClass(), Components);


    // Get the name of the InstancedFoliageActor for the folder
    FName FolderName = InstancedFoliageActor->GetFName();

    // Get the name of the level
    FString LevelName = Level->GetOutermost()->GetName();

    // 遍历所有的UInstancedStaticMeshComponent
    for (UActorComponent* ActorComponent : Components)
    {
        UInstancedStaticMeshComponent* Component = Cast<UInstancedStaticMeshComponent>(ActorComponent);
        if (!Component) continue;

        // Check if the StaticMesh of the component is in the MeshesToConvert array
        int32 MeshIndex = MeshesToConvert.IndexOfByKey(Component->GetStaticMesh());
        if (MeshIndex == INDEX_NONE) continue;

        UFoliageType* FoliageType = FoliageTypes.IsValidIndex(MeshIndex) ? FoliageTypes[MeshIndex] : nullptr;



        // 遍历每个实例
        for (int32 i = 0; i < Component->GetInstanceCount(); i++)
        {
            FTransform InstanceTransform;
            Component->GetInstanceTransform(i, InstanceTransform, true);

            // 创建新的StaticMeshActor
            AStaticMeshActor* NewActor = World->SpawnActor<AStaticMeshActor>(InstanceTransform.GetLocation(), InstanceTransform.GetRotation().Rotator());
            if (!NewActor) continue;

            // 设置新的StaticMeshActor的StaticMesh
            NewActor->GetStaticMeshComponent()->SetStaticMesh(Component->GetStaticMesh());
            NewActor->GetStaticMeshComponent()->SetWorldTransform(InstanceTransform);

            // Set the folder path for the new actor
            FString NewFolderPath = LevelName + "/" + FolderName.ToString() + "_" + Component->GetStaticMesh()->GetFName().ToString();
            NewActor->SetFolderPath(FName(*NewFolderPath));


            // Add a tag to the new actor with the value equal to the name of the original InstancedFoliageActor
            NewActor->Tags.Add(FName(*LevelName));
            NewActor->Tags.Add(FName(*InstancedFoliageActor->GetName()));

            // Add a tag to the new actor with the corresponding foliage type
            if (FoliageType)
            {
                NewActor->Tags.Add(FName(*FoliageType->GetName()));
            	NewActor->SetActorLabel(*FoliageType->GetName());
            }

            NewActors.Add(NewActor);
        	UE_LOG(LogTemp, Log, TEXT("Convert %s From %s"), *NewActor->GetName(), *InstancedFoliageActor->GetName());
        }
       // If bDestroy is true, remove the component
    	if (bDestroy)
    	{

    		if (Component)
    		{
    			FFoliageInfo* FoliageInfo = InstancedFoliageActor->FindInfo(FoliageType);
    			FoliageInfo->RemoveBaseComponentOnInstances();
    			TArray<int32> InstancesIndices;
    			for( int i = 0; i < FoliageInfo->Instances.Num(); i++ )
    			{
    				InstancesIndices.Add( i );
    			}
    			FoliageInfo->RemoveInstances(InstancedFoliageActor,InstancesIndices,true);
    		}
    	}
    }
	// Move the new actors to the level of the InstancedFoliageActor
	UEditorLevelUtils::MoveActorsToLevel(NewActors, Level, false);

	for(AActor* actor : NewActors)
	{
		actor->ClearInternalFlags(EInternalObjectFlags::AllFlags);
		actor->SetFlags(RF_Transactional);
	}
}
#endif

void UAssetCheckToolBPLibrary::ConvertActorsToFoliage(TArray<AActor*> ActorsToConvert, TArray<UFoliageType*> FoliageTypes, bool bConvertAll)
#if ENGINE_MAJOR_VERSION >= 5
{//ue5
    // 获取编辑器世界
    UWorld* World = GEditor->GetEditorWorldContext().World();

    if (!IsValid(World) || ActorsToConvert.Num() != FoliageTypes.Num())
    {
        return;
    }

    TArray<AActor*> ActorsToProcess;
    TArray<UFoliageType*> FoliageTypesToProcess;

    // 如果需要转换所有匹配的 StaticMeshActor
    if (bConvertAll)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), AllActors);

        for (AActor* Actor : AllActors)
        {
            AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor);
            if (!StaticMeshActor) continue;

            UStaticMesh* Mesh = StaticMeshActor->GetStaticMeshComponent()->GetStaticMesh();
            if (!Mesh) continue;

            for (AActor* ActorToConvert : ActorsToConvert)
            {
                AStaticMeshActor* StaticMeshActorToConvert = Cast<AStaticMeshActor>(ActorToConvert);
                if (!StaticMeshActorToConvert) continue;

                UStaticMesh* MeshToConvert = StaticMeshActorToConvert->GetStaticMeshComponent()->GetStaticMesh();
                if (!MeshToConvert) continue;

                if (Mesh == MeshToConvert)
                {
                    ActorsToProcess.Add(Actor);
                    int32 Index = ActorsToConvert.IndexOfByKey(ActorToConvert);
                    UFoliageType* FoliageType = FoliageTypes[Index];
                    FoliageTypesToProcess.Add(FoliageType);
                    break;
                }
            }
        }
    }
    else
    {
        ActorsToProcess = ActorsToConvert;
        FoliageTypesToProcess = FoliageTypes;
    }

    // 获取世界中的所有 InstancedFoliageActor
    TArray<AInstancedFoliageActor*> AllFoliageActors;
    for (TActorIterator<AInstancedFoliageActor> It(World); It; ++It)
    {
        AllFoliageActors.Add(*It);
    }

    // 遍历所有需要处理的 Actor
    for (int32 i = 0; i < ActorsToProcess.Num(); i++)
    {
        AActor* Actor = ActorsToProcess[i];
        UFoliageType* FoliageType = FoliageTypesToProcess[i];

        AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor);
        if (!StaticMeshActor) continue;

        UStaticMesh* Mesh = StaticMeshActor->GetStaticMeshComponent()->GetStaticMesh();
        if (!Mesh) continue;

        // 找到包含 Actor 的 InstancedFoliageActor
        AInstancedFoliageActor* FoliageActor = nullptr;
        for (AInstancedFoliageActor* IFA : AllFoliageActors)
        {
            // 替换为你的逻辑以确定 Actor 是否在子关卡中
            if (IFA->GetActorLocation().Equals(Actor->GetActorLocation()))
            {
                FoliageActor = IFA;
                break;
            }
        }
        if (!FoliageActor) continue;

        // 获取或创建 FoliageInfo
        FFoliageInfo* FoliageInfo = FoliageActor->FindOrAddMesh(FoliageType);
        if (!FoliageInfo) continue;

        // 创建新的 FoliageInstance
        FFoliageInstance NewInstance;
        NewInstance.Location = FVector(Actor->GetActorLocation()); // 将 FVector 转换为 FVector3f
        NewInstance.Rotation = Actor->GetActorRotation();
        NewInstance.DrawScale3D = FVector3f(Actor->GetActorScale3D()); // 将 FVector 转换为 FVector3f

        // 添加新实例到 FoliageInfo
        FoliageInfo->AddInstance(FoliageType, NewInstance, StaticMeshActor->GetStaticMeshComponent());

        // 销毁原始 Actor
        Actor->Destroy();
    }
}
#else
{//ue4
	//Get Editor World
	UWorld* World = GEditor->GetEditorWorldContext().World();

	if (!IsValid(World) || ActorsToConvert.Num() != FoliageTypes.Num())
	{
		return;
	}

	TArray<AActor*> ActorsToProcess;
	TArray<UFoliageType*> FoliageTypesToProcess;
	if (bConvertAll)
	{
		TArray<AActor*> AllActors;
		UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), AllActors);

		for (AActor* Actor : AllActors)
		{

			AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor);
			if (!StaticMeshActor) continue;

			UStaticMesh* Mesh = StaticMeshActor->GetStaticMeshComponent()->GetStaticMesh();
			if (!Mesh) continue;

			for (AActor* ActorToConvert : ActorsToConvert)
			{
				AStaticMeshActor* StaticMeshActorToConvert = Cast<AStaticMeshActor>(ActorToConvert);
				if (!StaticMeshActorToConvert) continue;

				UStaticMesh* MeshToConvert = StaticMeshActorToConvert->GetStaticMeshComponent()->GetStaticMesh();
				if (!MeshToConvert) continue;

				if (Mesh == MeshToConvert)
				{
					ActorsToProcess.Add(Actor);
					int32 Index = ActorsToConvert.IndexOfByKey(ActorToConvert);
					UFoliageType* FoliageType = FoliageTypes[Index];
					FoliageTypesToProcess.Add(FoliageType);
					break;
				}
			}
		}
	}
	else
	{
		ActorsToProcess = ActorsToConvert;
	}

	// Get all InstancedFoliageActors in the world
	TArray<AInstancedFoliageActor*> AllFoliageActors;
	for (TActorIterator<AInstancedFoliageActor> It(World); It; ++It)
	{
		AllFoliageActors.Add(*It);
	}

	for (int32 i = 0; i < ActorsToProcess.Num(); i++)
	{
		AActor* Actor = ActorsToProcess[i];
		UFoliageType* FoliageType = FoliageTypesToProcess[i];

		AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor);
		if (!StaticMeshActor) continue;

		UStaticMesh* Mesh = StaticMeshActor->GetStaticMeshComponent()->GetStaticMesh();
		if (!Mesh) continue;

		// Find the InstancedFoliageActor for the sublevel that contains the actor
		AInstancedFoliageActor* FoliageActor = nullptr;
		for (AInstancedFoliageActor* IFA : AllFoliageActors)
		{
			// Replace this with your own logic to determine if the actor is in the sublevel
			if (IFA->GetActorLocation().Equals(Actor->GetActorLocation()))
			{
				FoliageActor = IFA;
				break;
			}
		}
		if (!FoliageActor) continue;

		// Get the FoliageInfo for this mesh
		FFoliageInfo* FoliageInfo = FoliageActor->FindOrAddMesh(FoliageType);
		if (!FoliageInfo) continue;

		// Create a new FoliageInstance
		FFoliageInstance NewInstance;
		NewInstance.Location = Actor->GetActorLocation();
		NewInstance.Rotation = Actor->GetActorRotation();
		NewInstance.DrawScale3D = Actor->GetActorScale3D();

		// Add the new instance to the FoliageInfo
		FoliageInfo->AddInstance(FoliageActor, FoliageType, NewInstance);

		// Destroy the original actor
		Actor->Destroy();
	}
}
#endif

void UAssetCheckToolBPLibrary::ConvertActorsToFoliageWithTag(TArray<AActor*> ActorsToConvert, TArray<FString>& OutLevelPaths)
#if ENGINE_MAJOR_VERSION >= 5
{//ue5
    // 获取编辑器世界
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!IsValid(World))
    {
        return;
    }

    TArray<FString> FoldersToDel;
    TArray<AActor*> ActorsToProcess;
	ActorsToProcess = ActorsToConvert;

    /*for (AActor* Actor : ActorsToProcess)
    {
        AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor);
    	ACrossStaticMeshActor* CrossStaticMeshActor = Cast<ACrossStaticMeshActor>(Actor);  //Arashi Cross资产支持
        if (!StaticMeshActor && !CrossStaticMeshActor) continue;  //Arashi Cross资产支持
    	UStaticMesh* Mesh = nullptr;
		if(StaticMeshActor)
			 Mesh = StaticMeshActor->GetStaticMeshComponent()->GetStaticMesh();
    	if(CrossStaticMeshActor)  //Arashi Cross资产支持
			 Mesh = CrossStaticMeshActor->GetCrossStaticMesh()->GetPlatformInfo(UCrossPlatformSettings::GetCrossPlatform()).GroupMeshes[0]; //Arashi Cross资产支持
        if (!Mesh) continue;

        ULevel* ActorLevel = Actor->GetLevel();
        FName LevelName = Actor->Tags.Num() > 0 ? FName(Actor->Tags[0].ToString()) : FName();

        ULevelStreaming* LevelStreaming = World->GetLevelStreamingForPackageName(LevelName);
        if (LevelStreaming && LevelStreaming->IsLevelLoaded())
        {
            ULevel* Sublevel = LevelStreaming->GetLoadedLevel();
            ActorLevel = Sublevel;
        }

        // 将关卡路径添加到输出数组
        if (ActorLevel)
        {
            OutLevelPaths.AddUnique(LevelName.ToString());
        }
    	AInstancedFoliageActor* FoliageActor= nullptr;
    	//AInstancedFoliageActor* FoliageActor = AInstancedFoliageActor::GetInstancedFoliageActorForLevel(ActorLevel);
    	//if(!FoliageActor)
    	//
    	if(StaticMeshActor)
    		FoliageActor = GetPartitionFoliageActor( StaticMeshActor);
    	if(CrossStaticMeshActor)  //Arashi Cross资产支持
    		FoliageActor = GetPartitionFoliageActor( CrossStaticMeshActor);


    	//}
    	/*
    	// 如果未找到指定的 FoliageActor，使用原有逻辑
    	if (!FoliageActor)
    	{
    		FoliageActor = AInstancedFoliageActor::GetInstancedFoliageActorForLevel(ActorLevel);
    		if (!FoliageActor)
    		{
    			FoliageActor = World->SpawnActor<AInstancedFoliageActor>();
    			FoliageActor->Tags = Actor->Tags;
    		}
    	}#1#

        if (FoliageActor)
        {
        	FoliageActor->Tags = Actor->Tags;
            FString FoliageTypeName = Actor->Tags.Num() > 2 ? Actor->Tags[2].ToString() : TEXT("");

            //if (UFoliageType* FoliageType = FindObject<UFoliageType>(ANY_PACKAGE, *FoliageTypeName))
            if (UFoliageType* FoliageType = FindObject<UFoliageType>(nullptr, *FoliageTypeName))
            {

                FFoliageInfo* Info = FoliageActor->FindOrAddMesh(FoliageType);

                UFoliageInstancedStaticMeshComponent* Component = NewObject<UFoliageInstancedStaticMeshComponent>(FoliageActor, NAME_None, RF_Transactional);
                //Component->SetStaticMesh(Mesh);
                //Component->RegisterComponent();
                //Component->AttachToComponent(FoliageActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);


                // 创建新的 FoliageInstance
                FFoliageInstance NewInstance;
                NewInstance.Location = FVector(Actor->GetActorLocation()); // 将 FVector 转换为 FVector3f
                NewInstance.Rotation = Actor->GetActorRotation();
                NewInstance.DrawScale3D = FVector3f(Actor->GetActorScale()); // 将 FVector 转换为 FVector3f
                //NewInstance.BaseComponent = Component;


            	// 记录需要删除的文件夹路径
            	FString FolderPath = Actor->GetFolderPath().ToString();
            	FoldersToDel.AddUnique(FolderPath);

            	// 从关卡中移除 Actor
            	ActorLevel->Actors.Remove(Actor);
            	ActorLevel->ActorsForGC.Remove(Actor);
            	Actor->Destroy();

            	// 将新实例的位置从本地坐标转换为世界坐标
            	//FVector WorldLocation = FoliageActor->GetTransform().TransformPosition(NewInstance.Location);
            	//NewInstance.Location = FVector(WorldLocation); // 将 FVector 转换为 FVector3f

            	float ZStartOffset = Mesh->GetBoundingBox().Max.Z * Actor->GetActorScale().Z;

            	FVector Start = NewInstance.Location + FVector(0.f, 0.f, ZStartOffset);
            	FVector End = NewInstance.Location - FVector(0.f, 0.f, 10000.f);


            	FHitResult Hit;
            	static FName NAME_FoliageSnap = FName("FoliageSnap");
            	if (AInstancedFoliageActor::FoliageTrace(FoliageActor->GetWorld(), Hit, FDesiredFoliageInstance(Start, End, FoliageType), NAME_FoliageSnap,  false, FFoliageTraceFilterFunc(), (NewInstance.Flags & FOLIAGE_AlignToNormal)))
            	{
            		UPrimitiveComponent* HitComponent = Hit.Component.Get();

            		//if (HitComponent->GetComponentLevel() != FoliageActor->GetLevel())return ;


            		//if (HitComponent->IsCreatedByConstructionScript())return;

            		// Find BSP brush
            		UModelComponent* ModelComponent = Cast<UModelComponent>(HitComponent);
            		if (ModelComponent)
            		{
            			ABrush* BrushActor = ModelComponent->GetModel()->FindBrush((FVector3f)Hit.Location);
            			if (BrushActor)
            			{
            				HitComponent = BrushActor->GetBrushComponent();
            			}
            		}
            		NewInstance.BaseComponent = HitComponent;
            	}
            	Info->AddInstance(FoliageType, NewInstance);
                // 刷新 FoliageInfo
                Info->Refresh( true, true);

                // 标记关卡为已修改
                ActorLevel->Modify(true);
            }
        }
    }*/

    // 删除文件夹
    if (FoldersToDel.Num() > 0)
    {
        AssetViewUtils::DeleteFolders(FoldersToDel);
    }
}
#else
{//ue4
    // Get Editor World
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!IsValid(World))
    {
        return;
    }
    TArray<FString> FoldersToDel;
    TArray<AActor*> ActorsToProcess;

    if (bConvertAll && ActorsToConvert.Num() > 0)
    {
        TArray<AActor*> AllLevelsActors;
        // Check if the world exists
        if (World)
        {
            // Iterate through all levels
            for (ULevel* Level : World->GetLevels())
            {
                // Check if the level exists
                if (Level)
                {
                    // Iterate through all actors in the level
                    for (AActor* Actor : Level->Actors)
                    {
                        // Check if the actor exists and is AStaticMeshActor
                        if (Actor && Actor->IsA(AStaticMeshActor::StaticClass()) && Actor->Tags.Num() > 2)
                        {
                            AllLevelsActors.AddUnique(Actor);
                        }
                    }
                }
            }
        }
        for (AActor* inputActor : ActorsToConvert)
        {
            if (inputActor->GetFolderPath().IsValid())
            {
                // Get the Outliner path of the actor
                FName OutlinerPath = inputActor->GetFolderPath();

                for (AActor* Actor : AllLevelsActors)
                {
                    // Check if the Outliner path of the actor matches the first actor
                    if (Actor->GetFolderPath() == OutlinerPath)
                    {
                        ActorsToProcess.AddUnique(Actor);
                    }
                }
            }
        }
    }
    else
    {
        ActorsToProcess = ActorsToConvert;
    }

    for (AActor* Actor : ActorsToProcess)
    {
        AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor);
        if (!StaticMeshActor) continue;

        UStaticMesh* Mesh = StaticMeshActor->GetStaticMeshComponent()->GetStaticMesh();
        if (!Mesh) continue;

        ULevel* ActorLevel = Actor->GetLevel();
        FName LevelName = FName(Actor->Tags[0].ToString());

        ULevelStreaming* LevelStreaming = World->GetLevelStreamingForPackageName(LevelName);
        if (LevelStreaming && LevelStreaming->IsLevelLoaded())
        {
            ULevel* Sublevel = LevelStreaming->GetLoadedLevel();
            ActorLevel = Sublevel;
        }

        // Add the level path to the output array
        if (ActorLevel)
        {
        	OutLevelPaths.AddUnique(LevelName.ToString());
        }

        // Find or create a matching AInstancedFoliageActor
        AInstancedFoliageActor* FoliageActor = AInstancedFoliageActor::GetInstancedFoliageActorForLevel(ActorLevel);

        if (!FoliageActor)
        {
            FoliageActor = World->SpawnActor<AInstancedFoliageActor>();
            FoliageActor->Tags = Actor->Tags;
        }
        if (FoliageActor)
        {
            FString FoliageTypeName = Actor->Tags[2].ToString();

            if (UFoliageType* FoliageType = FindObject<UFoliageType>(ANY_PACKAGE, *FoliageTypeName))
            {
                FFoliageInfo* Info = FoliageActor->FindOrAddMesh(FoliageType);
                UFoliageInstancedStaticMeshComponent* Component = NewObject<UFoliageInstancedStaticMeshComponent>(FoliageActor, NAME_None, RF_Transactional);
                Component->SetStaticMesh(Mesh);
                Component->RegisterComponent();
                Component->AttachToComponent(FoliageActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);

                FFoliageInstance NewInstance;
                NewInstance.Location = Actor->GetActorLocation();
                NewInstance.Rotation = Actor->GetActorRotation();
                NewInstance.DrawScale3D = Actor->GetActorScale();
                NewInstance.BaseComponent = Component;

                Info->AddInstance(FoliageActor, FoliageType, NewInstance);
                // Convert the local location of the new instance to world location
                FVector WorldLocation = FoliageActor->GetTransform().TransformPosition(NewInstance.Location);
                // Update the location of the new instance
                NewInstance.Location = WorldLocation;
                Info->Refresh(FoliageActor, true, true);

                FString FolderPath = Actor->GetFolderPath().ToString();
                FoldersToDel.AddUnique(FolderPath);
                ActorLevel->Actors.Remove(Actor);
                ActorLevel->ActorsForGC.Remove(Actor);
                Actor->Destroy();

                ActorLevel->Modify(true);
            }
        }
    }

    if (FoldersToDel.Num() > 0)
    {
        AssetViewUtils::DeleteFolders(FoldersToDel);
    }
}
#endif





void UAssetCheckToolBPLibrary::GetStaticMeshCollisionPreset(UStaticMesh* TargetStaticMesh, FString& CollisionPreset)
{
	if (TargetStaticMesh && TargetStaticMesh->GetBodySetup())
	{
		const UBodySetup* BodySetup = TargetStaticMesh->GetBodySetup();
		const FName& ProfileName = BodySetup->DefaultInstance.GetCollisionProfileName();
		CollisionPreset = ProfileName.ToString();
	}
	else
	{
		CollisionPreset = FString(); // 设置为空字符串，表示无效的碰撞预设
	}
}

void UAssetCheckToolBPLibrary::GetSkeletalMeshCollisionPreset(USkeletalMesh* TargetSkeletalMesh, FString& CollisionPreset)
{
	if (TargetSkeletalMesh && TargetSkeletalMesh->GetBodySetup())
	{
		const UBodySetup* BodySetup = TargetSkeletalMesh->GetBodySetup();
		const FName& ProfileName = BodySetup->DefaultInstance.GetCollisionProfileName();
		CollisionPreset = ProfileName.ToString();
	}
	else
	{
		CollisionPreset = FString(); // 设置为空字符串，表示无效的碰撞预设
	}
}


void UAssetCheckToolBPLibrary::GetPaperSpriteCollisionPreset(UPaperSprite* TargetPaperSprite, FString& CollisionPreset)
{
	if (TargetPaperSprite && TargetPaperSprite->BodySetup)
	{
		const UBodySetup* BodySetup = TargetPaperSprite->BodySetup;
		const FName& ProfileName = BodySetup->DefaultInstance.GetCollisionProfileName();
		CollisionPreset = ProfileName.ToString();
	}
	else
	{
		CollisionPreset = FString(); // 设置为空字符串，表示无效的碰撞预设
	}
}





TArray<FString> UAssetCheckToolBPLibrary::GetTagFromStaticMesh(UStaticMesh* StaticMesh) {
	if (StaticMesh) {

		if (const UBRAssetToolTagAssetUserData* AssetUserData = StaticMesh->GetAssetUserData<UBRAssetToolTagAssetUserData>())
		{
			return AssetUserData->GetAllTags();
		}
	}
	return TArray<FString>();
}

void UAssetCheckToolBPLibrary::AddTagToObjects(TArray<UObject*> Objects, const FString& Tag) {
	for (UObject* Object : Objects) {

		if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(Object)) {
			UBRAssetToolTagAssetUserData* AssetUserData = StaticMesh->GetAssetUserData<UBRAssetToolTagAssetUserData>();
			if (!AssetUserData) {
				AssetUserData = NewObject<UBRAssetToolTagAssetUserData>(StaticMesh);
				StaticMesh->AddAssetUserData(AssetUserData);
			}
			if (!AssetUserData->HasTag(Tag)) {
				AssetUserData->AddTag(Tag);
				StaticMesh->MarkPackageDirty();
			}
		}
	}
}

void UAssetCheckToolBPLibrary::RemoveTagFromObjects(TArray<UObject*> Objects, const FString& Tag) {
	for (UObject* Object : Objects) {

		if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(Object)) {
			UBRAssetToolTagAssetUserData* AssetUserData = StaticMesh->GetAssetUserData<UBRAssetToolTagAssetUserData>();
			if (AssetUserData && AssetUserData->HasTag(Tag)) {
				AssetUserData->RemoveTag(Tag);
				StaticMesh->MarkPackageDirty();
			}
		}
	}
}

bool UAssetCheckToolBPLibrary::HasTagInStaticMesh(UStaticMesh* StaticMesh, const FString& Tag)
{
	if (StaticMesh) {
		UBRAssetToolTagAssetUserData* AssetUserData = StaticMesh->GetAssetUserData<UBRAssetToolTagAssetUserData>();
		if (AssetUserData && AssetUserData->HasTag(Tag)) {
			return true;
		}
	}
	return false;
}

void UAssetCheckToolBPLibrary::GetLODStatusOfActorsInCurrentView(const TArray<AActor*>& Actors, TArray<UStaticMesh*>& StaticMeshArray, TArray<int32>& LODStatusArray)
{

	if (FEditorViewportClient* ViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient()))
    {
        FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
            ViewportClient->Viewport,
            ViewportClient->GetScene(),
            ViewportClient->EngineShowFlags)
            .SetRealtimeUpdate(ViewportClient->IsRealtime()));

        ViewportClient->CalcSceneView(&ViewFamily);

        StaticMeshArray.Empty();
        LODStatusArray.Empty();

        if (ViewFamily.Views.Num() > 0)
        {
            const FSceneView* SceneView = ViewFamily.Views[0];

            for (AActor* Actor : Actors)
            {
                if (Actor)
                {

                    if (UStaticMeshComponent* StaticMeshComponent = Actor->FindComponentByClass<UStaticMeshComponent>()) // 检查StaticMeshComponent是否为空
                    {

                        if (UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh()) // 检查StaticMesh是否为空
                        {
                            const FStaticMeshRenderData* RenderData = StaticMesh->GetRenderData();
                            if (RenderData != nullptr)
                            {
                                int32 LOD;
                                if (StaticMeshComponent->ForcedLodModel != 0)
                                    LOD = StaticMeshComponent->ForcedLodModel - 1;
                                else
                                    LOD = ComputeStaticMeshLOD(RenderData, StaticMeshComponent->Bounds.Origin, StaticMeshComponent->Bounds.SphereRadius, *SceneView, StaticMesh->GetMinLOD().Default);

                                // 将StaticMesh和LOD级别添加到对应的数组中
                                StaticMeshArray.Add(StaticMesh);
                                LODStatusArray.Add(LOD);
                            }
                        }
                    }
                }
                else
                {
                    // 处理Actor为空的情况
                    // 这只是一个示例，你可能希望以不同的方式处理这种情况
                    StaticMeshArray.Add(nullptr);
                    LODStatusArray.Add(0);
                }
            }
        }
    }
}

void UAssetCheckToolBPLibrary::UpdateMaterialBR(UMaterialInstanceConstant* Material)
{
#if WITH_EDITOR
	if (Material)
	{
		UMaterialEditorInstanceConstant* EditorMIC = NewObject<UMaterialEditorInstanceConstant>();
		EditorMIC->SetSourceInstance(Material);
		Material->MarkPackageDirty();
	}
#endif
}


void UAssetCheckToolBPLibrary::SwitchFoliageMode()
{
	// 获取编辑器模式工具
	FEditorModeTools& ModeTools = GLevelEditorModeTools();

	// 检查当前是否处于指定的模式

		if (ModeTools.IsModeActive(FBuiltinEditorModes::EM_Foliage))
		{
			// 如果处于指定的模式，则切换到默认模式
			FEdModeFoliage* FoliageEditMode = nullptr;
			FoliageEditMode = static_cast<FEdModeFoliage*>(ModeTools.GetActiveMode(FBuiltinEditorModes::EM_Foliage));
			if(FoliageEditMode)
			{
					FoliageEditMode->UISettings.SetEraseToolSelected(false);
					FoliageEditMode->UISettings.SetLassoSelectToolSelected(false);
					FoliageEditMode->UISettings.SetPaintToolSelected(false);
					FoliageEditMode->UISettings.SetReapplyToolSelected(false);
					FoliageEditMode->UISettings.SetPaintBucketToolSelected(false);
					FoliageEditMode->UISettings.SetIsInQuickEraseMode(false);
					FoliageEditMode->UISettings.SetIsInQuickSingleInstantiationMode(false);
					FoliageEditMode->UISettings.SetIsInSingleInstantiationMode(false);
					FoliageEditMode->UISettings.SetSelectToolSelected(false) ;
					FoliageEditMode->OnToolChanged.Broadcast();

			}


			ModeTools.DeactivateMode(FBuiltinEditorModes::EM_Foliage);
		}
		else
		{
			// 如果不处于指定的模式，则切换到指定的模式
			ModeTools.ActivateMode(FBuiltinEditorModes::EM_Foliage);
			FEdModeFoliage* FoliageEditMode = nullptr;
			FoliageEditMode = static_cast<FEdModeFoliage*>(ModeTools.GetActiveMode(FBuiltinEditorModes::EM_Foliage));
			if(FoliageEditMode)
			{
				if(FoliageEditMode->UISettings.GetSelectToolSelected() == false)
				{
					FoliageEditMode->UISettings.SetEraseToolSelected(false);
					FoliageEditMode->UISettings.SetLassoSelectToolSelected(false);
					FoliageEditMode->UISettings.SetPaintToolSelected(false);
					FoliageEditMode->UISettings.SetReapplyToolSelected(false);
					FoliageEditMode->UISettings.SetPaintBucketToolSelected(false);
					FoliageEditMode->UISettings.SetIsInQuickEraseMode(false);
					FoliageEditMode->UISettings.SetIsInQuickSingleInstantiationMode(false);
					FoliageEditMode->UISettings.SetIsInSingleInstantiationMode(false);
					FoliageEditMode->UISettings.SetSelectToolSelected(true) ;
					FoliageEditMode->OnToolChanged.Broadcast();


				}
			}
		}
}

void UAssetCheckToolBPLibrary::SplitStringToStrings(FString InputString, FString SplitString,  TArray<FString>& OutStrings, bool InCullEmpty )
{
		TArray<FString> CSVLineArray;
		InputString.ParseIntoArray(CSVLineArray, *SplitString,InCullEmpty);
	    OutStrings=CSVLineArray;
}


void UAssetCheckToolBPLibrary::SplitTextToStrings(FText InputString, FString SplitString, TArray<FString>& OutStrings, bool InCullEmpty )
{
	TArray<FString> CSVLineArray;
	InputString.ToString().ParseIntoArray(CSVLineArray, *SplitString,InCullEmpty);
	OutStrings = CSVLineArray;
}

void UAssetCheckToolBPLibrary::ReadCSVToString(FString CSVPath, int32 SpacesNume,TArray<FString>& CSVLines_String,  TArray<FText>& CSVLines_Text,TArray<FString>& CSVLinesSplitBySpaces)
{
	TArray<FString> LinesStrings;
	FFileHelper::LoadFileToStringArray(LinesStrings, *CSVPath);
	CSVLines_String=LinesStrings;
	for (const FString& CSVLine : LinesStrings)
	{
		CSVLines_Text.Add(FText::FromString(CSVLine));
		CSVLinesSplitBySpaces.Add(CSVLine.ConvertTabsToSpaces(SpacesNume));
	}
}

void UAssetCheckToolBPLibrary::InBound(AActor* Actor, AActor* BoundActor, bool& Inbound, bool OnlyXY)
{
	if (Actor && BoundActor)
	{
		FVector ActorLocation = Actor->GetActorLocation();
		FVector BoundActorLocation = BoundActor->GetActorLocation();
		FVector BoundActorExtent = BoundActor->GetComponentsBoundingBox().GetExtent();

		FVector ActorLocationXY = FVector(ActorLocation.X, ActorLocation.Y, BoundActorLocation.Z);

		if (OnlyXY)
		{
			Inbound = BoundActor->GetComponentsBoundingBox().IsInsideXY(ActorLocationXY);
		}
		else
		{
			Inbound = BoundActor->GetComponentsBoundingBox().IsInside(ActorLocation);
		}
	}
	else
	{
		Inbound = false;
	}
}


void UAssetCheckToolBPLibrary::RunEditorUtilityWidgetByPath( FString path)
{


	if (UEditorUtilityWidgetBlueprint* Widget = LoadObject<UEditorUtilityWidgetBlueprint>(nullptr,*path ))
	{
		UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
		EditorUtilitySubsystem->SpawnAndRegisterTab(Widget);
	}
}


bool UAssetCheckToolBPLibrary::ConfirmDialog(FString message)
{
    FText DialogTitle = FText::FromString("Confirm");
    FText DialogContent = FText::FromString(message);
#if ENGINE_MAJOR_VERSION >= 5
	EAppReturnType::Type DialogResult = FMessageDialog::Open(EAppMsgType::YesNo, DialogContent, DialogTitle);
#else
	EAppReturnType::Type DialogResult = FMessageDialog::Open(EAppMsgType::YesNo, DialogContent, &DialogTitle);
#endif



    if (DialogResult == EAppReturnType::Yes)
    {
        return true;
    }
    else
    {
        return false;
    }
}



void UAssetCheckToolBPLibrary::MoveViewportCamerasToActor(AActor* Actor)
{
	if (IsValid(Actor))
	{
		if (GEditor)
		{
			GEditor->MoveViewportCamerasToActor(*Actor, false);
			GEditor->SelectActor(Actor, true, true);
		}
	}
}

void UAssetCheckToolBPLibrary::BrowserToObjects( TArray<UObject*> InObjectsToSync, bool bFocusContentBrowser  )
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.Get().SyncBrowserToAssets( InObjectsToSync, false, bFocusContentBrowser );
}


TArray<int32> UAssetCheckToolBPLibrary::SortToIntArray(const TArray<FString>& strs)
{
	// 创建一个排序后的字符串数组
	TArray<FString> sortedStrs = strs;
	Algo::Sort(sortedStrs);

	// 创建一个新的整数数组，其中的元素是原始字符串在排序后数组中的位置
	TArray<int32> sortedPositions;
	for (const FString& str : strs) {
		sortedPositions.Add(sortedStrs.IndexOfByKey(str));
	}

	return sortedPositions;
}

TArray<AActor*> UAssetCheckToolBPLibrary::GetIntersectingBBoxActors(AActor* InputActor, TArray<AActor*> OtherActors)
{
	TArray<AActor*> IntersectingActors;

	if (!InputActor || OtherActors.Num() == 0)
	{
		return IntersectingActors;
	}

	// 计算输入Actor的AABB
	FBox AABB1 = InputActor->GetComponentsBoundingBox();

	// 遍历输入的其他Actor数组
	for (AActor* OtherActor : OtherActors)
	{
		if (OtherActor == InputActor)
		{
			continue;
		}

		// 计算其他Actor的AABB
		FBox AABB2 = OtherActor->GetComponentsBoundingBox();

		// 检查两个AABB是否相交或者一个包含另一个
		if (AABB1.Intersect(AABB2) || AABB1.IsInside(AABB2) || AABB2.IsInside(AABB1))
		{
			IntersectingActors.Add(OtherActor);
		}
	}

	return IntersectingActors;
}

TArray<AActor*> UAssetCheckToolBPLibrary::GetFilteredActorsFromScene(TArray<FString> ExcludeStrings, TArray<FString> IncludeStrings)
{
	TArray<AActor*> FilteredActors;
	//Get Editor World
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!IsValid(World))
	{
		return FilteredActors;
	}

	// 获取场景中的所有Actor
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;

		// 检查Actor是否包含UStaticMeshComponent
		UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Actor->GetComponentByClass(UStaticMeshComponent::StaticClass()));
		if (!MeshComp)
		{
			continue;
		}

		// 获取Actor的名字
		FString ActorName = Actor->GetName();

		// 检查Actor的名字是否包含"排除"字符数组中的任何一个字符
		bool bExclude = false;
		for (FString ExcludeString : ExcludeStrings)
		{
			if (ActorName.Contains(ExcludeString))
			{
				bExclude = true;
				break;
			}
		}

		if (bExclude)
		{
			continue;
		}

		// 检查Actor的名字是否包含"只保留"字符数组中的任何一个字符
		bool bInclude = false;
		for (FString IncludeString : IncludeStrings)
		{
			if (ActorName.Contains(IncludeString))
			{
				bInclude = true;
				break;
			}
		}

		if (!bInclude && IncludeStrings.Num() > 0)
		{
			continue;
		}

		// 如果Actor通过了所有的检查，那么将它添加到结果数组中
		FilteredActors.Add(Actor);
	}

	return FilteredActors;
}
bool UAssetCheckToolBPLibrary::TrianglesIntersect(const FVector& V1_1, const FVector& V1_2, const FVector& V1_3, const FVector& V2_1, const FVector& V2_2, const FVector& V2_3)
{
	constexpr float Tolerance = 1e-6f; // 浮点数容差

	// 分离轴定理实现
	auto TestAxis = [Tolerance](const FVector& Axis, const FVector& V1_1, const FVector& V1_2, const FVector& V1_3, const FVector& V2_1, const FVector& V2_2, const FVector& V2_3) -> bool
	{
		float  Max1, Max2;

		// 投影三角形1的顶点到轴上
		float Min1 = Max1 = FVector::DotProduct(Axis, V1_1);
		float Proj1 = FVector::DotProduct(Axis, V1_2);
		Min1 = FMath::Min(Min1, Proj1);
		Max1 = FMath::Max(Max1, Proj1);
		Proj1 = FVector::DotProduct(Axis, V1_3);
		Min1 = FMath::Min(Min1, Proj1);
		Max1 = FMath::Max(Max1, Proj1);

		// 投影三角形2的顶点到轴上
		float Min2 = Max2 = FVector::DotProduct(Axis, V2_1);
		float Proj2 = FVector::DotProduct(Axis, V2_2);
		Min2 = FMath::Min(Min2, Proj2);
		Max2 = FMath::Max(Max2, Proj2);
		Proj2 = FVector::DotProduct(Axis, V2_3);
		Min2 = FMath::Min(Min2, Proj2);
		Max2 = FMath::Max(Max2, Proj2);

		// 检查投影是否重叠
		return !(Max1 < Min2 - Tolerance || Max2 < Min1 - Tolerance);
	};

	// 三角形1的法向量
	FVector Normal1 = FVector::CrossProduct(V1_2 - V1_1, V1_3 - V1_1).GetSafeNormal();
	// 三角形2的法向量
	FVector Normal2 = FVector::CrossProduct(V2_2 - V2_1, V2_3 - V2_1).GetSafeNormal();

	// 检查法向量轴
	if (!TestAxis(Normal1, V1_1, V1_2, V1_3, V2_1, V2_2, V2_3) || !TestAxis(Normal2, V1_1, V1_2, V1_3, V2_1, V2_2, V2_3))
	{
		return false;
	}

	// 检查边的叉积轴
	FVector Edges1[3] = { V1_2 - V1_1, V1_3 - V1_2, V1_1 - V1_3 };
	FVector Edges2[3] = { V2_2 - V2_1, V2_3 - V2_2, V2_1 - V2_3 };

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			FVector Axis = FVector::CrossProduct(Edges1[i], Edges2[j]).GetSafeNormal();
			if (!TestAxis(Axis, V1_1, V1_2, V1_3, V2_1, V2_2, V2_3))
			{
				return false;
			}
		}
	}

	return true;
}

bool UAssetCheckToolBPLibrary::AreActorsColliding(UObject* Object1, AActor* Actor2)
#if ENGINE_MAJOR_VERSION >= 5
{//ue5
    if (!Object1 || !Actor2)
    {
        return false;
    }

    TArray<UStaticMeshComponent*> MeshComponents1;

    // 判断输入1的类型
    if (AActor* InputActor1 = Cast<AActor>(Object1))
    {
        // 如果输入1是Actor，则获取其所有的UStaticMeshComponent
        if (InputActor1 == Actor2)
        {
            return false;
        }
        InputActor1->GetComponents<UStaticMeshComponent>(MeshComponents1);
    }
    else if (UStaticMeshComponent* MeshComp1 = Cast<UStaticMeshComponent>(Object1))
    {
        // 如果输入1是UStaticMeshComponent，则将其加入数组
        AActor* OwnerActor1 = MeshComp1->GetOwner();
        if (OwnerActor1 == Actor2)
        {
            return false;
        }
        MeshComponents1.Add(MeshComp1);
    }

    // 获取第二个Actor的所有UStaticMeshComponent
    TArray<UStaticMeshComponent*> MeshComponents2;
    Actor2->GetComponents<UStaticMeshComponent>(MeshComponents2);

    // 遍历第一个数组中的所有UStaticMeshComponent
    for (UStaticMeshComponent* MeshComp1 : MeshComponents1)
    {
        if (!MeshComp1 || !MeshComp1->GetStaticMesh())
        {
            continue;
        }

        // 获取第一个UStaticMeshComponent的LOD0几何体
        FStaticMeshLODResources& LODModel1 = MeshComp1->GetStaticMesh()->GetRenderData()->LODResources[0];
        FPositionVertexBuffer& VertexBuffer1 = LODModel1.VertexBuffers.PositionVertexBuffer;
        FIndexArrayView Indices1 = LODModel1.IndexBuffer.GetArrayView();

        // 计算第一个UStaticMeshComponent的AABB
        FBox AABB1 = MeshComp1->Bounds.GetBox();

        // 遍历第二个数组中的所有UStaticMeshComponent
        for (UStaticMeshComponent* MeshComp2 : MeshComponents2)
        {
            if (!MeshComp2 || !MeshComp2->GetStaticMesh())
            {
                continue;
            }

            // 获取第二个UStaticMeshComponent的LOD0几何体
            FStaticMeshLODResources& LODModel2 = MeshComp2->GetStaticMesh()->GetRenderData()->LODResources[0];
            FPositionVertexBuffer& VertexBuffer2 = LODModel2.VertexBuffers.PositionVertexBuffer;
            FIndexArrayView Indices2 = LODModel2.IndexBuffer.GetArrayView();

            // 计算第二个UStaticMeshComponent的AABB
            FBox AABB2 = MeshComp2->Bounds.GetBox();

            // AABB预筛选
            if (!AABB1.Intersect(AABB2))
            {
                continue;
            }

            // 遍历第一个Actor的所有三角形
            for (int32 i = 0; i < Indices1.Num(); i += 3)
            {
                FVector Vertex1_1 = FVector(VertexBuffer1.VertexPosition(Indices1[i]));
                FVector Vertex1_2 = FVector(VertexBuffer1.VertexPosition(Indices1[i + 1]));
                FVector Vertex1_3 = FVector(VertexBuffer1.VertexPosition(Indices1[i + 2]));

                // 将顶点转换到世界空间
                Vertex1_1 = MeshComp1->GetComponentTransform().TransformPosition(Vertex1_1);
                Vertex1_2 = MeshComp1->GetComponentTransform().TransformPosition(Vertex1_2);
                Vertex1_3 = MeshComp1->GetComponentTransform().TransformPosition(Vertex1_3);

                // 遍历第二个Actor的所有三角形
                for (int32 j = 0; j < Indices2.Num(); j += 3)
                {
                    FVector Vertex2_1 = FVector(VertexBuffer2.VertexPosition(Indices2[j]));
                    FVector Vertex2_2 = FVector(VertexBuffer2.VertexPosition(Indices2[j + 1]));
                    FVector Vertex2_3 = FVector(VertexBuffer2.VertexPosition(Indices2[j + 2]));

                    // 将顶点转换到世界空间
                    Vertex2_1 = MeshComp2->GetComponentTransform().TransformPosition(Vertex2_1);
                    Vertex2_2 = MeshComp2->GetComponentTransform().TransformPosition(Vertex2_2);
                    Vertex2_3 = MeshComp2->GetComponentTransform().TransformPosition(Vertex2_3);

                    // 检查两个三角形是否相交
                    if (TrianglesIntersect(Vertex1_1, Vertex1_2, Vertex1_3, Vertex2_1, Vertex2_2, Vertex2_3))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}
#else
{//ue4
    if (!Object1 || !Actor2)
    {
        return false;
    }

    TArray<UStaticMeshComponent*> MeshComponents1;

    // 判断输入1的类型
    if (AActor* InputActor1 = Cast<AActor>(Object1))
    {
        // 如果输入1是Actor，则获取其所有的UStaticMeshComponent
        if (InputActor1 == Actor2)
        {
            return false;
        }
        InputActor1->GetComponents<UStaticMeshComponent>(MeshComponents1);
    }
    else if (UStaticMeshComponent* MeshComp1 = Cast<UStaticMeshComponent>(Object1))
    {
        // 如果输入1是UStaticMeshComponent，则将其加入数组
        AActor* OwnerActor1 = MeshComp1->GetOwner();
        if (OwnerActor1 == Actor2)
        {
            return false;
        }
        MeshComponents1.Add(MeshComp1);
    }

    // 获取第二个Actor的所有UStaticMeshComponent
    TArray<UStaticMeshComponent*> MeshComponents2;
    Actor2->GetComponents<UStaticMeshComponent>(MeshComponents2);

    // 遍历第一个数组中的所有UStaticMeshComponent
    for (UStaticMeshComponent* MeshComp1 : MeshComponents1)
    {
        if (!MeshComp1 || !MeshComp1->GetStaticMesh())
        {
            continue;
        }

        // 获取第一个UStaticMeshComponent的LOD0几何体
        FStaticMeshLODResources& LODModel1 = MeshComp1->GetStaticMesh()->GetRenderData()->LODResources[0];
        FPositionVertexBuffer& VertexBuffer1 = LODModel1.VertexBuffers.PositionVertexBuffer;
        FIndexArrayView Indices1 = LODModel1.IndexBuffer.GetArrayView();

        // 计算第一个UStaticMeshComponent的AABB
        FBox AABB1 = MeshComp1->Bounds.GetBox();

        // 遍历第二个数组中的所有UStaticMeshComponent
        for (UStaticMeshComponent* MeshComp2 : MeshComponents2)
        {
            if (!MeshComp2 || !MeshComp2->GetStaticMesh())
            {
                continue;
            }

            // 获取第二个UStaticMeshComponent的LOD0几何体
            FStaticMeshLODResources& LODModel2 = MeshComp2->GetStaticMesh()->GetRenderData()->LODResources[0];
            FPositionVertexBuffer& VertexBuffer2 = LODModel2.VertexBuffers.PositionVertexBuffer;
            FIndexArrayView Indices2 = LODModel2.IndexBuffer.GetArrayView();

            // 计算第二个UStaticMeshComponent的AABB
            FBox AABB2 = MeshComp2->Bounds.GetBox();

            // AABB预筛选
            if (!AABB1.Intersect(AABB2))
            {
                continue;
            }

            // 遍历第一个Actor的所有三角形
            for (int32 i = 0; i < Indices1.Num(); i += 3)
            {
                FVector Vertex1_1 = VertexBuffer1.VertexPosition(Indices1[i]);
                FVector Vertex1_2 = VertexBuffer1.VertexPosition(Indices1[i + 1]);
                FVector Vertex1_3 = VertexBuffer1.VertexPosition(Indices1[i + 2]);

                // 将顶点转换到世界空间
                Vertex1_1 = MeshComp1->GetComponentTransform().TransformPosition(Vertex1_1);
                Vertex1_2 = MeshComp1->GetComponentTransform().TransformPosition(Vertex1_2);
                Vertex1_3 = MeshComp1->GetComponentTransform().TransformPosition(Vertex1_3);

                // 遍历第二个Actor的所有三角形
                for (int32 j = 0; j < Indices2.Num(); j += 3)
                {
                    FVector Vertex2_1 = VertexBuffer2.VertexPosition(Indices2[j]);
                    FVector Vertex2_2 = VertexBuffer2.VertexPosition(Indices2[j + 1]);
                    FVector Vertex2_3 = VertexBuffer2.VertexPosition(Indices2[j + 2]);

                    // 将顶点转换到世界空间
                    Vertex2_1 = MeshComp2->GetComponentTransform().TransformPosition(Vertex2_1);
                    Vertex2_2 = MeshComp2->GetComponentTransform().TransformPosition(Vertex2_2);
                    Vertex2_3 = MeshComp2->GetComponentTransform().TransformPosition(Vertex2_3);

                    // 检查两个三角形是否相交
                    if (TrianglesIntersect(Vertex1_1, Vertex1_2, Vertex1_3, Vertex2_1, Vertex2_2, Vertex2_3))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}
#endif



bool UAssetCheckToolBPLibrary::CheckCollisionWithActors(UObject* Object, const TArray<AActor*>& Actors)
{
    for (AActor* Actor2 : Actors)
    {
        if (AreActorsColliding(Object, Actor2))
        {
            return true;
        }
    }
    return false;
}


bool UAssetCheckToolBPLibrary::RayIntersectsTriangle(const FVector& RayOrigin, const FVector& RayDirection, const FVector& V0, const FVector& V1, const FVector& V2, FVector& OutIntersectionPoint)
{
    // Möller–Trumbore intersection algorithm
    constexpr float EPSILON = 0.0000001f;
    FVector Edge1 = V1 - V0;
    FVector Edge2 = V2 - V0;
    FVector H = FVector::CrossProduct(RayDirection, Edge2);
    float A = FVector::DotProduct(Edge1, H);
    if (A > -EPSILON && A < EPSILON)
    {
        return false; // This ray is parallel to this triangle.
    }
    float F = 1.0f / A;
    FVector S = RayOrigin - V0;
    float U = F * FVector::DotProduct(S, H);
    if (U < 0.0f || U > 1.0f)
    {
        return false;
    }
    FVector Q = FVector::CrossProduct(S, Edge1);
    float V = F * FVector::DotProduct(RayDirection, Q);
    if (V < 0.0f || U + V > 1.0f)
    {
        return false;
    }
    // At this stage we can compute t to find out where the intersection point is on the line.
    float T = F * FVector::DotProduct(Edge2, Q);
    if (T > EPSILON) // ray intersection
    {
        OutIntersectionPoint = RayOrigin + RayDirection * T;
        return true;
    }
    else // This means that there is a line intersection but not a ray intersection.
    {
        return false;
    }
}

bool ManualLineBoxIntersection(const FBox& Box, const FVector& Start, const FVector& End)
{
	FVector Direction = (End - Start).GetSafeNormal();
	FVector InvDirection = FVector(1.0f / Direction.X, 1.0f / Direction.Y, 1.0f / Direction.Z);

	FVector t1 = (Box.Min - Start) * InvDirection;
	FVector t2 = (Box.Max - Start) * InvDirection;

	FVector tMin(FMath::Min(t1.X, t2.X), FMath::Min(t1.Y, t2.Y), FMath::Min(t1.Z, t2.Z));
	FVector tMax(FMath::Max(t1.X, t2.X), FMath::Max(t1.Y, t2.Y), FMath::Max(t1.Z, t2.Z));

	float tEnter = FMath::Max(FMath::Max(tMin.X, tMin.Y), tMin.Z);
	float tExit = FMath::Min(FMath::Min(tMax.X, tMax.Y), tMax.Z);

	return tEnter <= tExit && tExit >= 0.0f;
}

bool UAssetCheckToolBPLibrary::GetRaycastPointAndDistance(const FVector& Start, const FVector& End, float& OutHitDistance, FString& OutHitActorName, FVector& OutHitLocation, FVector& OutHitNormal, const TArray<AActor*>& Actors, const TArray<AActor*>& IgnoreActors)
{
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!IsValid(World))
    {
        return false;
    }

    FVector RayDirection = (End - Start).GetSafeNormal();
    float ClosestHitDistance = FLT_MAX;
    bool bHit = false;

    TArray<AActor*> ActorsToCheck;

    if (Actors.Num() > 0)
    {
        ActorsToCheck = Actors;
    }
    else
    {
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            AActor* Actor = *It;
            ActorsToCheck.Add(Actor);
        }
    }

    for (AActor* Actor : ActorsToCheck)
    {
        if (IgnoreActors.Contains(Actor))
        {
            continue;
        }

        UStaticMeshComponent* StaticMeshComponent = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (Actor->IsA<ALandscape>() || Actor->IsA<ALandscapeStreamingProxy>())
        {
        	// 处理地形的碰撞检测
        	FHitResult HitResult;
        	FCollisionQueryParams Params;
        	Params.AddIgnoredActors(IgnoreActors);

        	if (World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
        	{
#if ENGINE_MAJOR_VERSION >= 5
        		if (HitResult.GetActor() == Actor) //ue5
#else
        		if (HitResult.Actor == Actor) //ue4
#endif
        		{
        			float HitDistance = FVector::Dist(Start, HitResult.Location);
        			if (HitDistance < ClosestHitDistance)
        			{
        				ClosestHitDistance = HitDistance;
        				OutHitDistance = HitDistance;
        				OutHitActorName = Actor->GetName();
        				OutHitLocation = HitResult.Location; // 存储碰撞点位置
        				OutHitNormal = HitResult.Normal; // 存储法线

        				bHit = true;
        			}
        		}
        	}
        }
        else if (StaticMeshComponent && StaticMeshComponent->GetStaticMesh())
        {
            // 获取包围盒
            FBoxSphereBounds Bounds = StaticMeshComponent->Bounds;
            FBox BBox = Bounds.GetBox();


            // 检查射线是否与包围盒相交
            if (!FMath::LineBoxIntersection(BBox, Start, End, RayDirection))
            {

                // 手动计算相交
                if (!ManualLineBoxIntersection(BBox, Start, End))
                {

                    continue;
                }

            }

            // 获取LOD0几何体
            FStaticMeshLODResources& LODModel = StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources[0];
            FPositionVertexBuffer& VertexBuffer = LODModel.VertexBuffers.PositionVertexBuffer;
            FIndexArrayView Indices = LODModel.IndexBuffer.GetArrayView();

            // 遍历所有三角形
            for (int32 i = 0; i < Indices.Num(); i += 3)
            {
#if ENGINE_MAJOR_VERSION >= 5
            	//ue5
            	FVector Vertex1 = FVector(VertexBuffer.VertexPosition(Indices[i]));
            	FVector Vertex2 = FVector(VertexBuffer.VertexPosition(Indices[i + 1]));
            	FVector Vertex3 = FVector(VertexBuffer.VertexPosition(Indices[i + 2]));

            	// 将顶点转换到世界空间
            	Vertex1 = StaticMeshComponent->GetComponentTransform().TransformPosition(Vertex1);
            	Vertex2 = StaticMeshComponent->GetComponentTransform().TransformPosition(Vertex2);
            	Vertex3 = StaticMeshComponent->GetComponentTransform().TransformPosition(Vertex3);
#else
            	//ue4
				FVector Vertex1 = VertexBuffer.VertexPosition(Indices[i]);
				FVector Vertex2 = VertexBuffer.VertexPosition(Indices[i + 1]);
				FVector Vertex3 = VertexBuffer.VertexPosition(Indices[i + 2]);

				// 将顶点转换到世界空间
				Vertex1 = StaticMeshComponent->GetComponentTransform().TransformPosition(Vertex1);
				Vertex2 = StaticMeshComponent->GetComponentTransform().TransformPosition(Vertex2);
				Vertex3 = StaticMeshComponent->GetComponentTransform().TransformPosition(Vertex3);
#endif

                FVector IntersectionPoint;
                if (RayIntersectsTriangle(Start, RayDirection, Vertex1, Vertex2, Vertex3, IntersectionPoint))
                {
                    float HitDistance = FVector::Dist(Start, IntersectionPoint);
                    if (HitDistance < ClosestHitDistance)
                    {
                        ClosestHitDistance = HitDistance;
                        OutHitDistance = HitDistance;
                        OutHitActorName = StaticMeshComponent->GetOwner()->GetName();
                        OutHitLocation = IntersectionPoint; // 存储碰撞点位置

                        // 计算法线
                        FVector Edge1 = Vertex2 - Vertex1;
                        FVector Edge2 = Vertex3 - Vertex1;
                        OutHitNormal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal(); // 存储法线

                        bHit = true;
                    }
                }
            }
        }

    }

    return bHit;
}

TArray<FVector> UAssetCheckToolBPLibrary::ScatterPointsOnActorBottom(UObject* WorldContextObject, AActor* TargetActor, float PointSpacing, float RayLength, bool bDebugDraw)
{
    TArray<FVector> HitPoints;

    if (!WorldContextObject || !TargetActor)
    {
        return HitPoints;
    }

    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    if (!World)
    {
        return HitPoints;
    }

    // 获取Actor的包围盒
    FBox ActorBBox = TargetActor->GetComponentsBoundingBox();

    // 获取包围盒的底面
    FVector Min = ActorBBox.Min;
    FVector Max = ActorBBox.Max;

    // 获取StaticMeshComponent
    UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(TargetActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));
    if (!StaticMeshComponent || !StaticMeshComponent->GetStaticMesh())
    {
        return HitPoints;
    }

    // 获取StaticMesh的顶点和索引
    const FStaticMeshLODResources& LODModel = StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources[0];
    const FPositionVertexBuffer& VertexBuffer = LODModel.VertexBuffers.PositionVertexBuffer;
    const FRawStaticIndexBuffer& IndexBuffer = LODModel.IndexBuffer;

    // 创建一个三角形索引数组
    struct FTriangle
    {
        uint32 Index0, Index1, Index2;
        FVector Center;
    };
    TArray<FTriangle> Triangles;

    // 填充三角形索引数组并计算中心点
    for (int32 i = 0; i < IndexBuffer.GetNumIndices(); i += 3)
    {
        uint32 Index0 = IndexBuffer.GetIndex(i);
        uint32 Index1 = IndexBuffer.GetIndex(i + 1);
        uint32 Index2 = IndexBuffer.GetIndex(i + 2);
/* ue4
        FVector V0 = StaticMeshComponent->GetComponentTransform().TransformPosition(VertexBuffer.VertexPosition(Index0));
        FVector V1 = StaticMeshComponent->GetComponentTransform().TransformPosition(VertexBuffer.VertexPosition(Index1));
        FVector V2 = StaticMeshComponent->GetComponentTransform().TransformPosition(VertexBuffer.VertexPosition(Index2));
*/
//ue5
    	FVector V0 = StaticMeshComponent->GetComponentTransform().TransformPosition(FVector(VertexBuffer.VertexPosition(Index0)));
    	FVector V1 = StaticMeshComponent->GetComponentTransform().TransformPosition(FVector(VertexBuffer.VertexPosition(Index1)));
    	FVector V2 = StaticMeshComponent->GetComponentTransform().TransformPosition(FVector(VertexBuffer.VertexPosition(Index2)));

        FVector Center = (V0 + V1 + V2) / 3.0f;

        Triangles.Add({ Index0, Index1, Index2, Center });
    }

    // 按照中心点的Z值从小到大排序
    Triangles.Sort([](const FTriangle& A, const FTriangle& B) {
        return A.Center.Z < B.Center.Z;
    });

    // 在底面上散布指定间距的点
    for (float X = Min.X; X <= Max.X; X += PointSpacing)
    {
        for (float Y = Min.Y; Y <= Max.Y; Y += PointSpacing)
        {
            FVector StartPoint = FVector(X, Y, Min.Z);
            FVector EndPoint = StartPoint + FVector(0, 0, RayLength); // 向上投射
            FVector RayDirection = (EndPoint - StartPoint).GetSafeNormal();

            // 遍历所有三角形
            for (const FTriangle& Triangle : Triangles)
            {
#if ENGINE_MAJOR_VERSION >= 5
            	//ue5
            	FVector V0 = StaticMeshComponent->GetComponentTransform().TransformPosition(FVector(VertexBuffer.VertexPosition(Triangle.Index0)));
            	FVector V1 = StaticMeshComponent->GetComponentTransform().TransformPosition(FVector(VertexBuffer.VertexPosition(Triangle.Index1)));
            	FVector V2 = StaticMeshComponent->GetComponentTransform().TransformPosition(FVector(VertexBuffer.VertexPosition(Triangle.Index2)));
#else
            	//ue4
				FVector V0 = StaticMeshComponent->GetComponentTransform().TransformPosition(VertexBuffer.VertexPosition(Triangle.Index0));
				FVector V1 = StaticMeshComponent->GetComponentTransform().TransformPosition(VertexBuffer.VertexPosition(Triangle.Index1));
				FVector V2 = StaticMeshComponent->GetComponentTransform().TransformPosition(VertexBuffer.VertexPosition(Triangle.Index2));
#endif

                FVector IntersectionPoint;
                if (RayIntersectsTriangle(StartPoint, RayDirection, V0, V1, V2, IntersectionPoint))
                {
                    // 确保交点在模型底部
                    if (IntersectionPoint.Z <= Min.Z + RayLength)
                    {
                        HitPoints.Add(IntersectionPoint);
                        break; // 只需要第一个交点
                    }
                }
            }

            // 绘制调试射线
            if (bDebugDraw)
            {
                FColor LineColor = HitPoints.Num() > 0 ? FColor::Green : FColor::Red;
                DrawDebugLine(World, StartPoint, EndPoint, LineColor, false, 1, 0, 1);
            }
        }
    }

    return HitPoints;
}

TArray<FVector> UAssetCheckToolBPLibrary::GetLowestPointsInActorSections(AActor* TargetActor)
{
	TArray<FVector> LowestPoints;

	if (!TargetActor)
	{
		return LowestPoints;
	}

	// 获取Actor中的所有StaticMeshComponent
	TArray<UStaticMeshComponent*> StaticMeshComponents;
	TargetActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);

	for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
	{
		if (!StaticMeshComponent || !StaticMeshComponent->GetStaticMesh())
		{
			continue;
		}

		// 获取StaticMesh的LOD资源
		const FStaticMeshLODResources& LODModel = StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources[0];
		const FPositionVertexBuffer& VertexBuffer = LODModel.VertexBuffers.PositionVertexBuffer;

		// 遍历每个Section
		for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); ++SectionIndex)
		{
			const FStaticMeshSection& Section = LODModel.Sections[SectionIndex];

			// 初始化最低点为一个非常高的值
			FVector LowestPoint(FLT_MAX, FLT_MAX, FLT_MAX);

			// 遍历Section中的所有顶点
			for (uint32 VertexIndex = Section.FirstIndex; VertexIndex < Section.FirstIndex + Section.NumTriangles * 3; ++VertexIndex)
			{
				uint32 Index = LODModel.IndexBuffer.GetIndex(VertexIndex);
#if ENGINE_MAJOR_VERSION >= 5
				FVector VertexPosition = StaticMeshComponent->GetComponentTransform().TransformPosition(FVector(VertexBuffer.VertexPosition(Index))); //ue5
#else
				FVector VertexPosition = StaticMeshComponent->GetComponentTransform().TransformPosition(VertexBuffer.VertexPosition(Index));//ue4
#endif

				// 更新最低点
				if (VertexPosition.Z < LowestPoint.Z)
				{
					LowestPoint = VertexPosition;
				}
			}

			// 将最低点添加到数组中
			LowestPoints.Add(LowestPoint);
		}
	}

	return LowestPoints;
}

bool UAssetCheckToolBPLibrary::GetShortestRaycastDistance(const TArray<FVector>& Points, const FVector& Direction, float Distance,const TArray<AActor*>& Actors, const TArray<AActor*>& IgnoreActors, FVector& OutOriginalPoint, FVector& OutShortestPoint, float& OutShortestDistance, FString& OutHitActorName, FVector& OutHitLocation, FVector& OutHitNormal)
{
	bool bFoundShortest = false;
	OutShortestDistance = FLT_MAX;

	for (const FVector& Point : Points)
	{
		float HitDistance;
		FString HitActorName;
		FVector HitLocation;
		FVector HitNormal;

		// 计算终点
		FVector End = Point + Direction * Distance;

		if (GetRaycastPointAndDistance(Point, End, HitDistance, HitActorName, HitLocation, HitNormal,Actors,IgnoreActors))
		{
			if (HitDistance < OutShortestDistance)
			{
				OutShortestDistance = HitDistance;
				OutOriginalPoint = Point; // 存储原始点位置
				OutShortestPoint = HitLocation; // 存储碰撞点位置
				OutHitActorName = HitActorName;
				OutHitLocation = HitLocation;
				OutHitNormal = HitNormal;
				bFoundShortest = true;
			}
		}
	}

	return bFoundShortest;
}

FVector UAssetCheckToolBPLibrary::GenerateRandomPointInTriangle(const FVector& A, const FVector& B, const FVector& C)
{
	float r1 = FMath::FRand();
	float r2 = FMath::FRand();

	if (r1 + r2 >= 1.0f)
	{
		r1 = 1.0f - r1;
		r2 = 1.0f - r2;
	}

	float r3 = 1.0f - r1 - r2;

	return r1 * A + r2 * B + r3 * C;
}

void UAssetCheckToolBPLibrary::ScatterPointsOnActorSurface(AActor* Actor, float MinDistance, int32 MaxAttempts, TArray<FVector>& OutPoints, TArray<FVector>& OutNormals)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor is null"));
        return;
    }

    TArray<UStaticMeshComponent*> StaticMeshComponents;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);

    for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
    {
        if (!StaticMeshComponent || !StaticMeshComponent->GetStaticMesh())
        {
            continue;
        }

        UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
        FTransform ComponentTransform = StaticMeshComponent->GetComponentTransform();

        for (int32 LODIndex = 0; LODIndex < StaticMesh->GetNumLODs(); ++LODIndex)
        {
            const FStaticMeshLODResources& LODResources = StaticMesh->GetRenderData()->LODResources[LODIndex];
            const FPositionVertexBuffer& VertexBuffer = LODResources.VertexBuffers.PositionVertexBuffer;
            const FStaticMeshVertexBuffer& StaticMeshVertexBuffer = LODResources.VertexBuffers.StaticMeshVertexBuffer;
            const FRawStaticIndexBuffer& IndexBuffer = LODResources.IndexBuffer;

            // 遍历所有三角形
            for (int32 TriangleIndex = 0; TriangleIndex < IndexBuffer.GetNumIndices() / 3; ++TriangleIndex)
            {
                FVector Vertices[3];
                FVector Normals[3];

                for (int32 CornerIndex = 0; CornerIndex < 3; ++CornerIndex)
                {
                    int32 VertexIndex = IndexBuffer.GetIndex(TriangleIndex * 3 + CornerIndex);
#if ENGINE_MAJOR_VERSION >= 5
                	Vertices[CornerIndex] = FVector(VertexBuffer.VertexPosition(VertexIndex)); //ue5
                	Normals[CornerIndex] = FVector(StaticMeshVertexBuffer.VertexTangentZ(VertexIndex)); //ue5
#else
                	Vertices[CornerIndex] = VertexBuffer.VertexPosition(VertexIndex);  //ue4
                	Normals[CornerIndex] = StaticMeshVertexBuffer.VertexTangentZ(VertexIndex); //ue4
#endif

                }

                // 将顶点和法线转换到世界坐标
                for (int32 i = 0; i < 3; ++i)
                {
                    Vertices[i] = ComponentTransform.TransformPosition(Vertices[i]);
                    Normals[i] = ComponentTransform.TransformVectorNoScale(Normals[i]);
                }

                // 泊松盘采样
                TArray<FVector> SamplePoints;
                TArray<FVector> SampleNormals;

                for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
                {
                    FVector Point = GenerateRandomPointInTriangle(Vertices[0], Vertices[1], Vertices[2]);
                    FVector Normal = (Normals[0] + Normals[1] + Normals[2]) / 3.0f;
                    Normal.Normalize();

                    bool bIsValid = true;
                    for (const FVector& ExistingPoint : SamplePoints)
                    {
                        if (FVector::Dist(ExistingPoint, Point) < MinDistance)
                        {
                            bIsValid = false;
                            break;
                        }
                    }

                    if (bIsValid)
                    {
                        SamplePoints.Add(Point);
                        SampleNormals.Add(Normal);
                    }
                }

                OutPoints.Append(SamplePoints);
                OutNormals.Append(SampleNormals);
            }
        }
    }
}


void UAssetCheckToolBPLibrary::SetRenderTargetSize(UTextureRenderTarget2D* RenderTarget, int32 NewWidth, int32 NewHeight)
{
	if (RenderTarget && NewWidth > 0 && NewHeight > 0)
	{
		RenderTarget->ResizeTarget(NewWidth, NewHeight);
	}
}



void UAssetCheckToolBPLibrary::SetDetailMode(UStaticMeshComponent* StaticMeshComponent, EDetailMode NewDetailMode)
{
	if (StaticMeshComponent == nullptr)
	{
		return; // 如果传入的组件是空指针，直接返回
	}

	StaticMeshComponent->DetailMode = NewDetailMode;
}


FMyMaterialBasePropertStruct UAssetCheckToolBPLibrary::GetMaterialBasePropertyOverrides(UMaterialInstanceConstant* MaterialInstance)
{
	FMyMaterialBasePropertStruct OverridesInfo{};

	if (UMaterialInstanceConstant* Material = Cast<UMaterialInstanceConstant>(MaterialInstance))
	{
		UMaterial* BaseMaterial = Material->GetBaseMaterial();

		// 获取覆盖信息
		OverridesInfo.OverrideBlendMode = Material->BasePropertyOverrides.bOverride_BlendMode;
		OverridesInfo.BlendMode = Material->BasePropertyOverrides.BlendMode;
		OverridesInfo.BlendModeBase = BaseMaterial ? BaseMaterial->BlendMode : TEnumAsByte<EBlendMode>(EBlendMode::BLEND_Opaque); // 安全检查

		OverridesInfo.OverrideTwoSided = Material->BasePropertyOverrides.bOverride_TwoSided;
		OverridesInfo.TwoSided = Material->BasePropertyOverrides.TwoSided;
		OverridesInfo.TwoSidedBase = BaseMaterial ? BaseMaterial->TwoSided : false; // 安全检查
		//Arashi
		/*
		OverridesInfo.OverrideUseHQIBL = Material->BasePropertyOverrides.bOverride_UseHQIBL;
		OverridesInfo.UseHQIBL = Material->BasePropertyOverrides.bUseHQIBL;
		OverridesInfo.UseHQIBLBase = BaseMaterial ? BaseMaterial->bUseHQIBL : false; // 安全检查
		*/
		OverridesInfo.OverrideSubsurfaceProfile = Material->bOverrideSubsurfaceProfile;
		OverridesInfo.SubsurfaceProfile = Material->SubsurfaceProfile;
		//OverridesInfo.SubsurfaceProfileBase = BaseMaterial ? BaseMaterial->SubsurfaceProfile : false; // 安全检查
		OverridesInfo.SubsurfaceProfileBase = BaseMaterial ? BaseMaterial->SubsurfaceProfile : nullptr; // 安全检查

		OverridesInfo.OverrideOpacityMaskClipValue = Material->BasePropertyOverrides.bOverride_OpacityMaskClipValue;
		OverridesInfo.OpacityMaskClipValue = Material->BasePropertyOverrides.OpacityMaskClipValue;
		OverridesInfo.OpacityMaskClipValueBase = BaseMaterial ? BaseMaterial->OpacityMaskClipValue : false; // 安全检查

		return OverridesInfo;
	}

	return OverridesInfo; // 如果 MaterialInstance 不是 UMaterialInstanceConstant，返回默认值
}


void UAssetCheckToolBPLibrary::RemoveMaterialBasePropertyOverrides(
	UMaterialInstanceConstant* MaterialInstance,
	bool bDisableBlendModeOverride,
	bool bDisableTwoSidedOverride,
	bool bDisableShadingModelOverride,
	bool bDisableOpacityMaskClipValueOverride)
{
	if (UMaterialInstanceConstant* Material = Cast<UMaterialInstanceConstant>(MaterialInstance))
	{
		// 取消 BlendMode 的覆盖（根据开关参数）
		if (bDisableBlendModeOverride && MaterialInstance->BasePropertyOverrides.bOverride_BlendMode)
		{
			MaterialInstance->BasePropertyOverrides.bOverride_BlendMode = false;
		}

		// 取消 TwoSided 的覆盖（根据开关参数）
		if (bDisableTwoSidedOverride && MaterialInstance->BasePropertyOverrides.bOverride_TwoSided)
		{
			MaterialInstance->BasePropertyOverrides.bOverride_TwoSided = false;
		}

		// 取消 ShadingModel 的覆盖（根据开关参数）
		if (bDisableShadingModelOverride && MaterialInstance->BasePropertyOverrides.bOverride_ShadingModel)
		{
			MaterialInstance->BasePropertyOverrides.bOverride_ShadingModel = false;
		}

		// 取消 OpacityMaskClipValue 的覆盖（根据开关参数）
		if (bDisableOpacityMaskClipValueOverride && MaterialInstance->BasePropertyOverrides.bOverride_OpacityMaskClipValue)
		{
			MaterialInstance->BasePropertyOverrides.bOverride_OpacityMaskClipValue = false;
		}
	}
}

bool UAssetCheckToolBPLibrary::ContainsExcludedKeyword(const FString& AssetName, const TArray<FString>& ExcludeKeywords, bool bInvertFilter)
{
	bool bContainsExcludedKeyword = ExcludeKeywords.ContainsByPredicate([&](const FString& Keyword) {
		return AssetName.Contains(Keyword);
	});
	return (bInvertFilter && bContainsExcludedKeyword) || (!bInvertFilter && !bContainsExcludedKeyword);
}

TArray<FAssetData> UAssetCheckToolBPLibrary::FindReferencesObject(
    const TSoftObjectPtr<UObject>& TargetObjectPtr,
    UClass* ReferenceClass,
    TArray<FString>& OutPackageNames,
    TArray<TSoftObjectPtr<UObject>>& OutSoftObjects,
    const TArray<FString>& ExcludeKeywords,
    bool bInvertFilter
)
{
    TArray<FAssetData> ResultAssets;

    if (!TargetObjectPtr.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("TargetObjectPtr is invalid!"));
        return ResultAssets;
    }

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FAssetIdentifier> Referencers;

    const FSoftObjectPath AssetPath = TargetObjectPtr.GetLongPackageName();
    const FName PackageName = FName(AssetPath.GetAssetPath().ToString());
    const FAssetIdentifier AssetId(PackageName);

    AssetRegistryModule.Get().GetReferencers(AssetId, Referencers, UE::AssetRegistry::EDependencyCategory::Package);
    //UE_LOG(LogTemp, Log, TEXT("Found %d referencers for asset: %s"), Referencers.Num(), *AssetPath.ToString());

    for (const FAssetIdentifier& RefId : Referencers)
    {
        const FString& AssetPackageName = RefId.PackageName.ToString();

        if (ContainsExcludedKeyword(AssetPackageName, ExcludeKeywords, bInvertFilter))
        {
            TArray<FAssetData> AssetDataArray;
            AssetRegistryModule.Get().GetAssetsByPackageName(RefId.PackageName, AssetDataArray);
            OutPackageNames.Add(AssetPackageName);

            for (const FAssetData& AssetData : AssetDataArray)
            {
                if (ContainsExcludedKeyword(AssetData.PackageName.ToString(), ExcludeKeywords, bInvertFilter))
                {
                    if (AssetData.AssetClassPath  == ReferenceClass->GetClassPathName())
                    {
                        ResultAssets.Add(AssetData);
                        OutSoftObjects.Add(TSoftObjectPtr<UObject>(AssetData.ToSoftObjectPath()));
                    }
                }
            }
        }
    }

    return ResultAssets;
}

TArray<FAssetData> UAssetCheckToolBPLibrary::FindReferencesObjectWithDepth(
    const TSoftObjectPtr<UObject>& TargetObjectPtr,
    UClass* ReferenceClass,
    TArray<FString>& OutPackageNames,
    TArray<TSoftObjectPtr<UObject>>& OutSoftObjects,
    const TArray<FString>& ExcludeKeywords,
    bool bInvertFilter,
    int32 Depth // 新增参数来控制查询深度
)
{
    TArray<FAssetData> ResultAssets;


	// 获取资产的路径
	FString AssetPathString = TargetObjectPtr.ToString();

	// 尝试加载资产
	UObject* LoadedAsset = nullptr;
	if (!TargetObjectPtr.IsValid())
	{
		LoadedAsset = UEditorAssetLibrary::LoadAsset(AssetPathString);
		if (!LoadedAsset)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load asset from path: %s"), *AssetPathString);
			return ResultAssets; // 如果无法加载对象，则返回空结果
		}
	}
	else
	{
		LoadedAsset = TargetObjectPtr.Get();
	}

	// 检查ReferenceClass是否有效
	if ( Depth <= 0 || ReferenceClass == nullptr)
	{
		return ResultAssets;
	}
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    TArray<FAssetIdentifier> Referencers;

    FSoftObjectPath AssetPath = TargetObjectPtr.GetLongPackageName();

#if ENGINE_MAJOR_VERSION >= 5
	FName PackageName = FName(AssetPath.GetAssetPath().ToString());
#else
	FName PackageName = FName(AssetPath.GetAssetPathName());
#endif
    FAssetIdentifier AssetId(PackageName);

    AssetRegistryModule.Get().GetReferencers(AssetId, Referencers, UE::AssetRegistry::EDependencyCategory::Package);

    for (const FAssetIdentifier& RefId : Referencers)
    {
        FString AssetPackageName = RefId.PackageName.ToString();
        bool bContainsExcludedKeyword = ExcludeKeywords.ContainsByPredicate([&](const FString& Keyword) { return AssetPackageName.Contains(Keyword); });

        if ((bInvertFilter && bContainsExcludedKeyword) || (!bInvertFilter && !bContainsExcludedKeyword))
        {
            TArray<FAssetData> AssetDataArray;
            AssetRegistryModule.Get().GetAssetsByPackageName(RefId.PackageName, AssetDataArray);
            OutPackageNames.Add(AssetPackageName);

            for (const FAssetData& AssetData : AssetDataArray)
            {
                FString AssetObjectName = AssetData.PackageName.ToString();
                bool bContainsExcludedKeyword2 = ExcludeKeywords.ContainsByPredicate([&](const FString& Keyword) { return AssetObjectName.Contains(Keyword); });

                if ((bInvertFilter && bContainsExcludedKeyword2) || (!bInvertFilter && !bContainsExcludedKeyword2))
                {
#if ENGINE_MAJOR_VERSION >= 5
                	if (AssetData.AssetClassPath == ReferenceClass->GetClassPathName())
#else
                	if (AssetData.AssetClass == ReferenceClass->GetFName())
#endif
                    {
                        ResultAssets.Add(AssetData);
                        TSoftObjectPtr<UObject> SoftObject(AssetData.ToSoftObjectPath());
                        OutSoftObjects.Add(SoftObject); // 添加Soft Object到输出数组

                        // 递归调用以处理更深层次的引用
                        if (Depth > 1)
                        {
                            FindReferencesObjectWithDepth(SoftObject, ReferenceClass, OutPackageNames, OutSoftObjects, ExcludeKeywords, bInvertFilter, Depth - 1);
                        }
                    }
                }
            }
        }
    }

    return ResultAssets;
}


TArray<FString> UAssetCheckToolBPLibrary::FindReferencers(
    const FString& PackageName,
    const TArray<UClass*>& ReferenceClasses,
    const TArray<FString>& ExcludeKeywords,
    bool WithOutClassCheck,
    bool bInvertFilter)
{
    TArray<FString> ResultReferencers;

    // 清理ExcludeKeywords中的空字符串
    TArray<FString> CleanedExcludeKeywords = ExcludeKeywords.FilterByPredicate([](const FString& Keyword) {
        return !Keyword.IsEmpty();
    });

    // 加载资产注册模块
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FAssetIdentifier> Referencers;

    FName PackageFName(*PackageName);
    FAssetIdentifier AssetId(PackageFName);

    // 获取所有引用了目标包的资产标识符
    AssetRegistryModule.Get().GetReferencers(AssetId, Referencers, UE::AssetRegistry::EDependencyCategory::Package);

    for (const FAssetIdentifier& RefId : Referencers)
    {
        FString AssetPackageName = RefId.PackageName.ToString();

        // 如果CleanedExcludeKeywords为空，跳过过滤判断
        bool bShouldProcess = true;
        if (CleanedExcludeKeywords.Num() > 0)
        {
            // 检查包名是否包含任何排除关键词
            bool bContainsExcludedKeyword = CleanedExcludeKeywords.ContainsByPredicate([&](const FString& Keyword) {
                return AssetPackageName.Contains(Keyword);
            });

            // 根据是否包含排除关键词和反转过滤标志决定是否处理此包名
            bShouldProcess = (bInvertFilter && bContainsExcludedKeyword) || (!bInvertFilter && !bContainsExcludedKeyword);
        }

        if (bShouldProcess)
        {
            TArray<FAssetData> AssetDataArray;
            AssetRegistryModule.Get().GetAssetsByPackageName(RefId.PackageName, AssetDataArray);
            for (const FAssetData& AssetData : AssetDataArray)
            {
                FString AssetObjectName = AssetData.PackageName.ToString();

                // 如果CleanedExcludeKeywords为空，跳过过滤判断
                bool bShouldProcessAsset = true;
                if (CleanedExcludeKeywords.Num() > 0)
                {
                    // 检查资产对象名是否包含任何排除关键词
                    bool bContainsExcludedKeyword2 = CleanedExcludeKeywords.ContainsByPredicate([&](const FString& Keyword) {
                        return AssetObjectName.Contains(Keyword);
                    });

                    // 根据是否包含排除关键词和反转过滤标志决定是否处理此资产
                    bShouldProcessAsset = (bInvertFilter && bContainsExcludedKeyword2) || (!bInvertFilter && !bContainsExcludedKeyword2);
                }

                if (bShouldProcessAsset)
                {
                    // 检查资产类别是否匹配
                    bool bClassMatch = ReferenceClasses.IsEmpty() || ReferenceClasses.ContainsByPredicate([&](const UClass* Class) {
#if ENGINE_MAJOR_VERSION >= 5
                        return AssetData.AssetClassPath == Class->GetClassPathName();
#else
                        return AssetData.AssetClass == Class->GetFName();
#endif
                    });

                    if (bClassMatch || WithOutClassCheck)
                    {
                        if (!ResultReferencers.Contains(AssetPackageName))
                        {
                            ResultReferencers.Add(AssetPackageName);
                        }
                    }
                }
            }
        }
    }

    return ResultReferencers;
}


TArray<FString> UAssetCheckToolBPLibrary::FindDependenciesWithClass(
	const FString& PackageName,
	const TArray<UClass*>& DependencyClasses,
	const TArray<FString>& ExcludeKeywords,
	bool WithOutClassCheck,
	bool bInvertFilter)
{
	TArray<FString> ResultDependencies;
	TSet<FString> ResultDependenciesSet; // 使用TSet避免重复

	// 加载资产注册模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetIdentifier> Dependencies;

	FName PackageFName(*PackageName);
	FAssetIdentifier AssetId(PackageFName);

	// 获取所有目标包的依赖资产标识符
	AssetRegistryModule.Get().GetDependencies(AssetId, Dependencies, UE::AssetRegistry::EDependencyCategory::Package);

	// 使用TSet优化关键词查找
	TSet<FString> ExcludedKeywordsSet(ExcludeKeywords);

	auto ContainsExcludedKeyword = [&](const FString& AssetName) {
		for (const FString& Keyword : ExcludedKeywordsSet) {
			if (AssetName.Contains(Keyword)) {
				return true;
			}
		}
		return false;
	};

	TSet<FName> UniquePackageNames;
	for (const FAssetIdentifier& DepId : Dependencies)
	{
		FString DepPackageName = DepId.PackageName.ToString();
		if ((bInvertFilter && ContainsExcludedKeyword(DepPackageName)) || (!bInvertFilter && !ContainsExcludedKeyword(DepPackageName)))
		{
			UniquePackageNames.Add(DepId.PackageName);
		}
	}

	TArray<FAssetData> AssetDataArray;
	AssetDataArray.Reserve(100); // 预分配空间以减少内存重新分配
	for (const FName& UniquePackageName : UniquePackageNames) // 修改这里的变量名
	{
		AssetDataArray.Empty(); // 清空数组以避免数据累积
		AssetRegistryModule.Get().GetAssetsByPackageName(UniquePackageName, AssetDataArray);

		for (const FAssetData& AssetData : AssetDataArray)
		{
			FString AssetPackageName = AssetData.PackageName.ToString();
			if ((bInvertFilter && ContainsExcludedKeyword(AssetPackageName)) || (!bInvertFilter && !ContainsExcludedKeyword(AssetPackageName)))
			{


				if (bool bClassMatch = WithOutClassCheck || DependencyClasses.IsEmpty() || DependencyClasses.ContainsByPredicate([&](const UClass* Class) { return AssetData.AssetClassPath  == Class->GetClassPathName();}))
				{
					ResultDependenciesSet.Add(AssetPackageName);
				}
			}
		}
	}

	ResultDependencies.Append(ResultDependenciesSet.Array()); // 将结果转换回TArray
	return ResultDependencies;
}


TArray<FAssetData> UAssetCheckToolBPLibrary::FindDependencies(
	const FString& PackageName,
	const TArray<FString>& ExcludeKeywords,
	bool bInvertFilter)
{
	TArray<FAssetData> ResultDependencies;
	TSet<FName> ProcessedPackages; // 使用TSet避免重复处理同一个包

	// 加载资产注册模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetIdentifier> Dependencies;

	FName PackageFName(*PackageName);
	FAssetIdentifier AssetId(PackageFName);

	// 获取所有目标包的依赖资产标识符
	//AssetRegistryModule.Get().GetDependencies(AssetId, Dependencies, UE::AssetRegistry::EDependencyCategory::All);
	AssetRegistryModule.Get().GetDependencies(AssetId, Dependencies);


	// 使用TSet优化关键词查找
	TSet<FString> ExcludedKeywordsSet(ExcludeKeywords);

	auto ContainsExcludedKeyword = [&](const FString& AssetName) {
		for (const FString& Keyword : ExcludedKeywordsSet) {
			if (AssetName.Contains(Keyword)) {
				return true;
			}
		}
		return false;
	};

	TSet<FName> UniquePackageNames;
	for (const FAssetIdentifier& DepId : Dependencies)
	{
		if ((bInvertFilter && ContainsExcludedKeyword(DepId.PackageName.ToString())) || (!bInvertFilter && !ContainsExcludedKeyword(DepId.PackageName.ToString())))
		{
			UniquePackageNames.Add(DepId.PackageName);
		}
	}

	for (const FAssetIdentifier& DepId : Dependencies)
	{
		FString AssetPackageName = DepId.PackageName.ToString();
		if ((bInvertFilter && ContainsExcludedKeyword(AssetPackageName)) || (!bInvertFilter && !ContainsExcludedKeyword(AssetPackageName)))
		{
			if (!ProcessedPackages.Contains(DepId.PackageName))
			{
				TArray<FAssetData> AssetDataArray;
				AssetRegistryModule.Get().GetAssetsByPackageName(DepId.PackageName, AssetDataArray);
				for (const FAssetData& AssetData : AssetDataArray)
				{
					ResultDependencies.Add(AssetData);
				}
				ProcessedPackages.Add(DepId.PackageName); // 记录已处理的包名，避免重复处理
			}
		}
	}

	return ResultDependencies;
}

void UAssetCheckToolBPLibrary::GetOverriedMaterialInstanceTextureParameters(
	UMaterialInstance* MaterialInstance,
	TArray<FString>& ChangedParameterNames,
	TArray<FString>& ChangedParameterValues)
{
	if (!MaterialInstance)
	{
		return;
	}

	UMaterialInterface* ParentMaterial = MaterialInstance->Parent;
	if (!ParentMaterial)
	{
		return;
	}

	TArray<FMaterialParameterInfo> ParameterInfos;
	TArray<FGuid> OutParameterIds;
	MaterialInstance->GetAllTextureParameterInfo(ParameterInfos, OutParameterIds);

	for (const FMaterialParameterInfo& ParamInfo : ParameterInfos)
	{
		FString ParamName = ParamInfo.Name.ToString();
		UTexture* InputValue;
		bool bResult = MaterialInstance->GetTextureParameterValue(ParamInfo, InputValue, true);

		if (bResult && InputValue != nullptr) // Ensure the texture is not null and the parameter is overridden
		{
			ChangedParameterNames.Add("t:" + ParamName);
			ChangedParameterValues.Add(InputValue->GetName()); // Assuming GetName() gives the name of the texture
		}
	}

	// Optionally, you can repeat similar checks for other parameter types like Vector, Scalar, etc.
}

FString UAssetCheckToolBPLibrary::FormatFloat(float Value)
{
	FString Result = FString::Printf(TEXT("%f"), Value);
	// Manually remove trailing zeros and decimal point if necessary
	if (Result.Contains(TEXT("."))) // Check if there is a decimal point
	{
		// Remove trailing zeros
		while (Result.EndsWith(TEXT("0")))
		{
			Result.RemoveAt(Result.Len() - 1);
		}
		// Remove trailing decimal point if it exists
		if (Result.EndsWith(TEXT(".")))
		{
			Result.RemoveAt(Result.Len() - 1);
		}
	}
	return Result;
}

FString UAssetCheckToolBPLibrary::FormatNumber(float Number, int32 TotalLength, int32 DecimalPlaces)
{
	// 分离整数和小数部分
	int32 IntPart = static_cast<int32>(Number);
	float FractionalPart = Number - IntPart;

	// 格式化整数部分（补零）
	std::stringstream IntStream;
	IntStream << std::setw(TotalLength) << std::setfill('0') << IntPart;

	// 格式化小数部分（保留指定位数）
	std::stringstream FractionalStream;
	FractionalStream << std::fixed << std::setprecision(DecimalPlaces) << FractionalPart;

	// 拼接结果
	std::string Result = IntStream.str() + FractionalStream.str().substr(1); // 去掉小数部分的前导0

	return FString(Result.c_str());
}

FString UAssetCheckToolBPLibrary::RemoveSpaces(const FString& Original)
{
	FString Result = Original;
	Result.ReplaceInline(TEXT(" "), TEXT("")); // Replace all spaces with empty string
	return Result;
}

void UAssetCheckToolBPLibrary::GetOverriedMaterialInstanceScalarParameters(
	UMaterialInstance* MaterialInstance,
	TArray<FString>& ChangedParameterNames,
	TArray<FString>& ChangedParameterValues)
{
	if (!MaterialInstance)
	{
		return;
	}

	UMaterialInterface* ParentMaterial = MaterialInstance->Parent;
	if (!ParentMaterial)
	{
		return;
	}

	TArray<FMaterialParameterInfo> ParameterInfos;
	TArray<FGuid> OutParameterIds;
	MaterialInstance->GetAllScalarParameterInfo(ParameterInfos, OutParameterIds);

	for (const FMaterialParameterInfo& ParamInfo : ParameterInfos)
	{
		FString ParamName = ParamInfo.Name.ToString();
		float InputValue;


		if (bool bResult = MaterialInstance->GetScalarParameterValue(ParamInfo, InputValue, true)) // Check if the parameter value was successfully retrieved and is overridden
		{
			ChangedParameterNames.Add("f:" + ParamName);
			ChangedParameterValues.Add(FormatFloat(InputValue));
		}
	}

	// Optionally, you can repeat similar checks for other parameter types like Vector, etc.
}

void UAssetCheckToolBPLibrary::GetOverriedMaterialInstanceVectorParameters(
	UMaterialInstance* MaterialInstance,
	TArray<FString>& ChangedParameterNames,
	TArray<FString>& ChangedParameterValues)
{
	if (!MaterialInstance)
	{
		return;
	}

	UMaterialInterface* ParentMaterial = MaterialInstance->Parent;
	if (!ParentMaterial)
	{
		return;
	}

	TArray<FMaterialParameterInfo> ParameterInfos;
	TArray<FGuid> OutParameterIds;
	MaterialInstance->GetAllVectorParameterInfo(ParameterInfos, OutParameterIds);

	for (const FMaterialParameterInfo& ParamInfo : ParameterInfos)
	{
		FString ParamName = ParamInfo.Name.ToString();
		FLinearColor InputValue;


		if (bool bResult = MaterialInstance->GetVectorParameterValue(ParamInfo, InputValue, true))
		{
			ChangedParameterNames.Add("v:" + ParamName);
			// Convert FLinearColor to FString
			//FString ValueStr = FString::Printf(TEXT("(%.2f, %.2f, %.2f,%.2f)"), InputValue.R, InputValue.G, InputValue.B, InputValue.A);
			FString ValueStr = FString::Printf(TEXT("(%s,%s,%s,%s)"),
				*FormatFloat(InputValue.R),
				*FormatFloat(InputValue.G),
				*FormatFloat(InputValue.B),
				*FormatFloat(InputValue.A));

			ChangedParameterValues.Add(ValueStr);
		}
	}

	// Optionally, you can repeat similar checks for other parameter types if needed.
}

void UAssetCheckToolBPLibrary::GetOverriedMaterialInstanceStaticSwitchParameters(
	UMaterialInstance* MaterialInstance,
	TArray<FString>& ChangedParameterNames,
	TArray<FString>& ChangedParameterValues)
{
	if (!MaterialInstance)
	{
		//UE_LOG(LogTemp, Warning, TEXT("MaterialInstance is nullptr."));
		return;
	}

	UMaterialInterface* ParentMaterial = MaterialInstance->Parent;
	if (!ParentMaterial)
	{
		//UE_LOG(LogTemp, Warning, TEXT("MaterialInstance does not have a parent material."));
		return;
	}

	TArray<FMaterialParameterInfo> ParameterInfos;
	TArray<FGuid> OutParameterIds;
	MaterialInstance->GetAllStaticSwitchParameterInfo(ParameterInfos, OutParameterIds);

	for (const FMaterialParameterInfo& ParamInfo : ParameterInfos)
	{
		FString ParamName = ParamInfo.Name.ToString();
		bool InputValue;
		FGuid OutExpressionGuid;

		// Assuming GetStaticSwitchParameterValue is correctly implemented to check overrides
		if (MaterialInstance->GetStaticSwitchParameterValue(ParamInfo, InputValue, OutExpressionGuid, true))
		{
			ChangedParameterNames.Add("S:" + ParamName);
			ChangedParameterValues.Add(InputValue ? TEXT("True") : TEXT("False"));
			//UE_LOG(LogTemp, Log, TEXT("Parameter overridden: %s, Value: %s"), *ParamName, InputValue ? TEXT("True") : TEXT("False"));
		}
	}

	// Optionally, you can repeat similar checks for other parameter types if needed.
}

void UAssetCheckToolBPLibrary::GetMaterialInstanceStaticSwitchParameters(
	UMaterialInstance* MaterialInstance,
	TArray<FString>& ParameterNames,
	TArray<FString>& ParameterValues)
{
	if (!MaterialInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("MaterialInstance is null"));
		return;
	}

	// 获取材质的参数信息
	FStaticParameterSet ParameterSet;
	MaterialInstance->GetStaticParameterValues(ParameterSet);

	// 遍历所有的静态开关参数
	for (const FStaticSwitchParameter& SwitchParameter : ParameterSet.StaticSwitchParameters)
	{
		// 将参数名称添加到ParameterNames数组
		ParameterNames.Add(SwitchParameter.ParameterInfo.Name.ToString());

		// 将参数值（true/false）转换为字符串，并添加到ParameterValues数组
		ParameterValues.Add(SwitchParameter.Value ? TEXT("True") : TEXT("False"));
	}
}
void UAssetCheckToolBPLibrary::GetMaterialInstanceStaticSwitchParametersNew(
	UMaterialInstance* MaterialInstance,
	TArray<FString>& ParameterNames,
	TArray<FString>& ParameterValues)
{
	ParameterNames.Empty();
	ParameterValues.Empty();

	if (!IsValid(MaterialInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid MaterialInstance"));
		return;
	}

	// 方案1：使用当前世界的FeatureLevel（推荐）
	UWorld* World = GEngine->GetWorldFromContextObject(MaterialInstance, EGetWorldErrorMode::LogAndReturnNull);
	ERHIFeatureLevel::Type FeatureLevel = World ? World->GetFeatureLevel() : ERHIFeatureLevel::Num;

	// 方案2：使用默认FeatureLevel（备选）
	if (FeatureLevel == ERHIFeatureLevel::Num)
	{
		FeatureLevel = GEngine->GetDefaultWorldFeatureLevel();
	}

	// 获取材质资源
	FMaterialResource* MaterialResource = MaterialInstance->GetMaterialResource(FeatureLevel);
	if (!MaterialResource)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get material resource"));
		return;
	}

	// 获取静态参数
	FStaticParameterSet StaticParameters;
	MaterialInstance->GetStaticParameterValues(StaticParameters);

	// 处理静态开关参数
	for (const FStaticSwitchParameter& SwitchParam : StaticParameters.StaticSwitchParameters)
	{
		if (SwitchParam.bOverride)
		{
			ParameterNames.Add(SwitchParam.ParameterInfo.Name.ToString());
			ParameterValues.Add(SwitchParam.Value ? TEXT("True") : TEXT("False"));
		}
	}
}

FString UAssetCheckToolBPLibrary::GetUserTextInput(const FString& PromptMessage)
{
	TSharedPtr<SEditableTextBox> EditText;

	FString UserInput;

	// 创建一个窗口
	const TSharedPtr<SWindow> ModalWindow = SNew(SWindow)
		.Title(FText::FromString("Input Required"))
		.ClientSize(FVector2D(400, 100))
		.SupportsMinimize(false)
		.SupportsMaximize(false);

	// 创建窗口内容
	ModalWindow->SetContent
	(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(PromptMessage))
		]
	+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SAssignNew(EditText, SEditableTextBox)
		]
	+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SButton)
			.Text(FText::FromString("OK"))
		.OnClicked_Lambda([&]()
			{
				UserInput = EditText->GetText().ToString();
	FSlateApplication::Get().RequestDestroyWindow(ModalWindow.ToSharedRef());
	return FReply::Handled();
			})
		]
	);

	// 显示窗口
	FSlateApplication::Get().AddModalWindow(ModalWindow.ToSharedRef(), nullptr);

	return UserInput;
}



void UAssetCheckToolBPLibrary::SyncAssets(const TArray<FString> PackageNames)
{
	AssetViewUtils::SyncPackagesFromSourceControl(PackageNames);
}




TArray<FString> UAssetCheckToolBPLibrary::GetObjectPathNames(const TArray<UObject*>& Objects)
{
	TArray<FString> PathNames;
	PathNames.Reserve(Objects.Num()); // 预留空间以提高性能

	for (UObject* Object : Objects)
	{
		if (Object) // 确保对象不为空
		{
			// 获取对象的路径名称
			FString PathName = Object->GetPathName();
			PathNames.Add(PathName); // 将路径名称添加到数组中
		}
		else
		{
			// 如果对象为空，添加一个空字符串
			PathNames.Add(""); // 添加空字符串
		}
	}

	return PathNames; // 返回路径名称数组
}



void UAssetCheckToolBPLibrary::CheckAssetsIsCurrent(const TArray<FString> PathNames, TArray<FString>& OutdatedAssets, TArray<FString>& CurrentAssets)
{
	ISourceControlProvider& SCCProvider = ISourceControlModule::Get().GetProvider();
	TArray<FString> DependencyFilenames = SourceControlHelpers::PackageFilenames(PathNames);

	// 更新源控制状态
	SCCProvider.Execute(ISourceControlOperation::Create<FUpdateStatus>(), PathNames);

	for (int32 DependencyIndex = 0; DependencyIndex < PathNames.Num(); ++DependencyIndex)
	{
		const FString& DependencyName = PathNames[DependencyIndex];
		const FString& DependencyFilename = DependencyFilenames[DependencyIndex];

		// 获取源控制状态
		FSourceControlStatePtr SCCState = SCCProvider.GetState(DependencyFilename, EStateCacheUsage::Use);

		// 检查状态并添加到相应的输出数组
		if (SCCState.IsValid() && !SCCState->IsCurrent())
		{
			OutdatedAssets.Add(DependencyName);  // 过时
		}
		else
		{
			CurrentAssets.Add(DependencyName); // 最新
		}
	}
}
/*
#include "SourceControlOperations.h"

#include "Modules/ModuleManager.h"
bool UMySourceControlFunctionLibrary::IsAssetLatestVersion(const FString& AssetPath)
{
	// 获取源控制模块
	ISourceControlModule& SourceControlModule = ISourceControlModule::Get();
	if (!SourceControlModule.IsEnabled())
	{
		return false; // 源控制未启用
	}

	// 获取资产注册模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// 获取资产数据
	FAssetData AssetData;
	AssetRegistry.GetAssetByObjectPath(FName(*AssetPath), true);
	if (!AssetData.IsValid())
	{
		return false; // 资产无效
	}

	// 创建源控制操作
	TSharedRef<FCheckOutAsset> CheckOutOperation = FSourceControlModule::Get().CreateCheckOutOperation();
	CheckOutOperation->SetAssetData(AssetData);

	// 检查资产是否是最新版本
	if (SourceControlModule.GetProvider().IsAvailable())
	{
		// 获取资产的最新版本信息
		TSharedPtr<FSourceControlState> State = SourceControlModule.GetProvider().GetState(AssetPath, EStateCacheUsage::ForceUpdate);
		if (State.IsValid())
		{
			return State->IsCurrent(); // 返回是否是最新版本
		}
	}

	return false; // 默认返回 false
}
*/





TArray<TSoftObjectPtr<UObject>> UAssetCheckToolBPLibrary::FindReferenceFoliageType(
	const TSoftObjectPtr<UObject>& TargetObjectPtr,
	TArray<FString>& OutPackageNames
)
{
	TSet<TSoftObjectPtr<UObject>> ResultSet; // 使用 TSet 避免重复
	TArray<TSoftObjectPtr<UObject>> ResultPaths;

	// 检查目标对象指针是否有效
	if (!TargetObjectPtr.IsValid())
	{
		return ResultPaths;
	}

	// 检查目标对象是否为 UStaticMesh
	UStaticMesh* TargetStaticMesh = Cast<UStaticMesh>(TargetObjectPtr.Get());
	if (!TargetStaticMesh)
	{
		return ResultPaths; // 如果不是 UStaticMesh，返回空结果
	}

	// 加载资产注册模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// 获取所有 UFoliageType_InstancedStaticMesh 资产
	TArray<FAssetData> FoliageAssets;
#if ENGINE_MAJOR_VERSION >= 5
	FTopLevelAssetPath ClassPath = UFoliageType_InstancedStaticMesh::StaticClass()->GetClassPathName(); //ue5
	AssetRegistryModule.Get().GetAssetsByClass(ClassPath, FoliageAssets);
#else
	AssetRegistryModule.Get().GetAssetsByClass(UFoliageType_InstancedStaticMesh::StaticClass()->GetFName(), FoliageAssets); //ue4
#endif




	// 遍历所有 UFoliageType_InstancedStaticMesh 资产
	for (const FAssetData& AssetData : FoliageAssets)
	{
		UFoliageType_InstancedStaticMesh* FoliageType = Cast<UFoliageType_InstancedStaticMesh>(AssetData.GetAsset());
		if (FoliageType && FoliageType->GetStaticMesh() == TargetStaticMesh)
		{
			// 获取资产路径并添加到结果集中
#if ENGINE_MAJOR_VERSION >= 5
			FSoftObjectPath ObjectPath(AssetData.GetObjectPathString());
#else
			FSoftObjectPath ObjectPath(AssetData.ObjectPath.ToString());
#endif


			TSoftObjectPtr<UObject> SoftObjectPtr(ObjectPath);
			ResultSet.Add(SoftObjectPtr); // 使用 TSet 避免重复

			// 记录包名
			OutPackageNames.Add(AssetData.PackageName.ToString());
		}
	}

	// 将结果集转换为数组
	ResultPaths.Append(ResultSet.Array());

	return ResultPaths;
}

TArray<FString> UAssetCheckToolBPLibrary::GetPackageNamesFromAssetDataArray(const TArray<FAssetData>& AssetDataArray)
{
	TArray<FString> PackageNames;

	for (const FAssetData& AssetData : AssetDataArray)
	{
		// 获取包名并添加到输出数组
		PackageNames.Add(AssetData.PackageName.ToString());
	}

	return PackageNames;
}


void UAssetCheckToolBPLibrary::MoveFolderFiles(const FString& SourceDirectory, const FString& DestinationDirectory)
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// 获取资产
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FARFilter Filter;
	Filter.PackagePaths.Add(*SourceDirectory.TrimStartAndEnd());		// 注意 清理路径字符串 删除多余的空格或换行符
	Filter.bRecursivePaths = true;
	TArray<FAssetData> AssetDataList;
	AssetRegistryModule.Get().GetAssets(Filter, AssetDataList);

	if (AssetDataList.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No assets found in the source path: %s"), *SourceDirectory);
		return;
	}

	TArray<FAssetRenameData> AssetsToRename;
	for (auto& Asset : AssetDataList)
	{
		// 构建新的资产路径，保持子目录结构
		FString AssetPath = Asset.PackagePath.ToString();
		FString RelativePath = AssetPath.Replace(*SourceDirectory, TEXT("")); // 获取相对于源目录的路径
		FString NewPath = DestinationDirectory + RelativePath; // 构建新的完整路径

		if (Asset.PackagePath.ToString() != NewPath)
		{
			AssetsToRename.Add(FAssetRenameData(Asset.GetAsset(), NewPath, Asset.AssetName.ToString()));
		}
	}

	if (AssetsToRename.Num() > 0)
	{
		// 执行重命名和移动
		if (!AssetTools.RenameAssets(AssetsToRename))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to move some assets from %s to %s"), *SourceDirectory, *DestinationDirectory);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No assets need to be moved."));
	}
}

void UAssetCheckToolBPLibrary::FixUpRedirectorsInFolder(const FString& Path)
{
	// 获取资产
	// 获取 AssetRegistry 模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	// 创建过滤器
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Reset();
	Filter.PackagePaths.Add(*Path.TrimStartAndEnd());		// 注意 清理路径字符串 删除多余的空格或换行符


#if ENGINE_MAJOR_VERSION >= 5
if (UClass* RedirectorClass = UObjectRedirector::StaticClass())
{
    Filter.ClassPaths.Add(FTopLevelAssetPath(RedirectorClass->GetPathName()));
}
#else
	Filter.ClassNames.Add(TEXT("ObjectRedirector"));
#endif

	// 查询指定路径下的资产
	TArray<FAssetData> AssetList;
	AssetList.Empty();
	AssetRegistryModule.Get().GetAssets(Filter, AssetList);

	if (AssetList.Num() > 0)
	{
		TArray<FString> ObjectPaths;
		for (const auto& Asset : AssetList)
		{
#if ENGINE_MAJOR_VERSION >= 5
			ObjectPaths.Add(Asset.GetObjectPathString());
#else
			ObjectPaths.Add(Asset.ObjectPath.ToString());
#endif
		}

		TArray<UObject*> Objects;
		const bool bAllowedToPromptToLoadAssets = true;
		const bool bLoadRedirects = true;

		// 加载资产
#if ENGINE_MAJOR_VERSION >= 5
		AssetViewUtils::FLoadAssetsSettings Settings{
			Settings.bAlwaysPromptBeforeLoading = bAllowedToPromptToLoadAssets,
			Settings.bFollowRedirectors = bLoadRedirects,
		};
		const AssetViewUtils::ELoadAssetsResult Result = AssetViewUtils::LoadAssetsIfNeeded(ObjectPaths, Objects, Settings);

		// 宣告变量在 switch 语句之外
		TArray<UObjectRedirector*> Redirectors;
		FAssetToolsModule* AssetToolsModule = nullptr;

		switch (Result) {
		case AssetViewUtils::ELoadAssetsResult::Success:
			// 资产加载成功，执行后续操作
				// 转换 Objects 数组为 ObjectRedirectors 数组
					Redirectors.Reserve(Objects.Num());
			for (auto Object : Objects)
			{
				// 使用 CastChecked 确保转换成功
				auto Redirector = CastChecked<UObjectRedirector>(Object);
				Redirectors.Add(Redirector);
			}

			// 加载 AssetTools 模块
			AssetToolsModule = &FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
			AssetToolsModule->Get().FixupReferencers(Redirectors);
			break;

		case AssetViewUtils::ELoadAssetsResult::SomeFailed:
			// 资产加载失败，处理错误情况
				UE_LOG(LogTemp, Warning, TEXT("资产加载失败."));
			break;

		case AssetViewUtils::ELoadAssetsResult::Cancelled:
			// 用户取消了加载操作，执行清理或通知
				UE_LOG(LogTemp, Warning, TEXT("用户取消了加载操作."));
			break;

		default:
			// 处理未预见的情况
				UE_LOG(LogTemp, Warning, TEXT("未知的结果."));
			break;
		}


#else
		if (AssetViewUtils::LoadAssetsIfNeeded(ObjectPaths, Objects, bAllowedToPromptToLoadAssets, bLoadRedirects))
		{
			// 转换 Objects 数组为 ObjectRedirectors 数组
			TArray<UObjectRedirector*> Redirectors;
			for (auto Object : Objects)
			{
				// 使用 CastChecked 确保转换成功
				auto Redirector = CastChecked<UObjectRedirector>(Object);
				Redirectors.Add(Redirector);
			}

			// 加载 AssetTools 模块
			FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
			AssetToolsModule.Get().FixupReferencers(Redirectors);
		}
#endif

	}

}

bool UAssetCheckToolBPLibrary::MoveAssetAndCleanupRedirector(const FString& SourcePath, const FString& TargetPath)
{
    // 获取资产工具模块
    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

    // 检查源路径是否存在
    if (!UEditorAssetLibrary::DoesAssetExist(SourcePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Source path %s does not exist."), *SourcePath);
        return false;
    }

    // 检查目标路径是否已经存在
    if (UEditorAssetLibrary::DoesAssetExist(TargetPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Target path %s already exists."), *TargetPath);
        return false;
    }

    // 获取源控制模块
    ISourceControlModule& SourceControlModule = ISourceControlModule::Get();
    ISourceControlProvider& SourceControlProvider = SourceControlModule.GetProvider();

    // Checkout源文件和目标文件
    SourceControlProvider.Execute(ISourceControlOperation::Create<FCheckOut>(), SourcePath);
    SourceControlProvider.Execute(ISourceControlOperation::Create<FCheckOut>(), TargetPath);

    // 移动资产
    TArray<FAssetRenameData> AssetsToRename;
    AssetsToRename.Add(FAssetRenameData(SourcePath, TargetPath));
    AssetToolsModule.Get().RenameAssets(AssetsToRename);

	FixUpRedirectorForAsset(SourcePath);

    UE_LOG(LogTemp, Log, TEXT("Successfully moved asset from %s to %s and cleaned up redirector."), *SourcePath, *TargetPath);
    return true;
}



void UAssetCheckToolBPLibrary::FixUpRedirectorForAsset(const FString& AssetPath)
{
	// 获取 AssetRegistry 模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");


#if ENGINE_MAJOR_VERSION >= 5
	// 创建一个 Soft Object Path
	FSoftObjectPath SoftObjectPath(AssetPath);  // Assuming AssetPath is an FString
	// 查询指定资产
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(SoftObjectPath);
#else
	// 查询指定资产
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FName(*AssetPath));
#endif

	if (AssetData.IsValid())
	{
		// 加载资产
		UObject* Asset = AssetData.GetAsset();
		if (Asset)
		{
			// 确保资产是重定向器
			if (UObjectRedirector* Redirector = Cast<UObjectRedirector>(Asset))
			{
				// 加载 AssetTools 模块
				FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
				TArray<UObjectRedirector*> Redirectors;
				Redirectors.Add(Redirector);

				// 修复重定向器
				AssetToolsModule.Get().FixupReferencers(Redirectors);
			}

		}

	}

}

/*{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	// Form a filter from the paths
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace(*Path);
	Filter.ClassNames.Emplace(TEXT("ObjectRedirector"));

	// Query for a list of assets in the selected paths
	TArray<FAssetData> AssetList;
	AssetRegistryModule.Get().GetAssets(Filter, AssetList);

	if (AssetList.Num() > 0)
	{
		TArray<UObjectRedirector*> Redirectors;
		for (const auto& Asset : AssetList)
		{
			UObject* Obj = Asset.GetAsset();

			if (UObjectRedirector* Redirector = Cast<UObjectRedirector>(Obj))
			{
				Redirectors.Add(Redirector);
			}
		}

		// Load the asset tools module
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		AssetToolsModule.Get().FixupReferencers(Redirectors);
	}
}
*/

void UAssetCheckToolBPLibrary::OpenEditorForAsset(UObject* Asset)
{
	if (Asset)
	{
		// 确保 GEditor 和 AssetEditorSubsystem 可用
		if (GEditor)
		{

			if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
			{
				// 打开资产编辑器
				AssetEditorSubsystem->OpenEditorForAsset(Asset);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Asset is null!"));
	}
}


void UAssetCheckToolBPLibrary::SelectAssetInBrowser(UObject* Asset)
{
    if (Asset)
    {
        // 获取资产注册模块
        FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
        IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

        // 获取资产的路径并转换为 FName
        FString  AssetPathName = *FSoftObjectPath(Asset).GetAssetPathString();
#if ENGINE_MAJOR_VERSION >= 5
    	// 创建一个 Soft Object Path
    	FSoftObjectPath SoftObjectPath(AssetPathName);  // Assuming AssetPath is an FString
    	// 查询指定资产
    	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(SoftObjectPath);
#else
    	// 查询指定资产
    	FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(AssetPathName);
#endif

        if (AssetData.IsValid())
        {
            // 选中资产
            GEditor->GetSelectedObjects()->DeselectAll(); // 先取消所有选择
            GEditor->GetSelectedObjects()->Select(Asset); // 重新选择目标资产

            // 刷新内容浏览器
            GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Asset);
            //GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FocusEditorForAsset(Asset);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Asset not found in the asset registry!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Asset is null!"));
    }
}

void UAssetCheckToolBPLibrary::SelectAssetsInBrowser(const TArray<UObject*>& Assets)
{
	// 获取内容浏览器模块的引用
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	// 准备一个资产的数组，用于选中操作
	TArray<FAssetData> AssetDataArray;

	// 遍历所有传入的资产，转换为FAssetData
	for (UObject* Asset : Assets)
	{
		if (Asset)
		{
			FAssetData AssetData(Asset);
			AssetDataArray.Add(AssetData);
		}
	}

	// 使用内容浏览器的单例来选中这些资产
	ContentBrowserModule.Get().SyncBrowserToAssets(AssetDataArray);
}

bool UAssetCheckToolBPLibrary::UpdateTexture(UTexture2D* Texture)
{
	if (!Texture)
	{
		return false; // 确保输入有效
	}

	// 需要调用 UpdateResource() 来应用更改
	Texture->UpdateResource();

	return true; // 成功更新
}

FString UAssetCheckToolBPLibrary::GetTextureSourcePath(UTexture* Texture)
{
	if (Texture)
	{
		const UTexture2D* Texture2D = Cast<UTexture2D>(Texture);
		if (Texture2D && Texture2D->AssetImportData)
		{
			return Texture2D->AssetImportData->GetFirstFilename();
		}
	}
	return FString();
}


TArray<AActor*> UAssetCheckToolBPLibrary::GetActorsWithStaticMesh(UStaticMesh* Mesh)
{
	TArray<AActor*> Result;

	if (!Mesh)
	{
		return Result;
	}

	// 获取当前世界
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		return Result;
	}

	// 遍历所有actor
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor)
		{
			// 检查这个Actor的所有StaticMeshComponent
			TArray<UStaticMeshComponent*> Components;
			Actor->GetComponents<UStaticMeshComponent>(Components);

			for (UStaticMeshComponent* Component : Components)
			{
				if (Component && Component->GetStaticMesh() == Mesh)
				{
					Result.Add(Actor);
					break; // 找到匹配的组件后，不需要检查这个Actor的其他组件
				}
			}
		}
	}

	return Result;
}

TArray<AActor*> UAssetCheckToolBPLibrary::FindActorsByStaticMesh(
	UStaticMesh* Mesh,
	const TArray<UClass*>& AllowedClasses
)
{
	TArray<AActor*> Result;

	if (!Mesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Mesh is null!"));
		return Result;
	}

	// 获取当前世界
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get current world!"));
		return Result;
	}

	// 遍历所有Actor
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor)
		{
			continue;
		}

		// 检查Actor的Class是否在AllowedClasses中
		bool bIsClassAllowed = AllowedClasses.Num() == 0; // 如果AllowedClasses为空，允许所有Class
		for (UClass* AllowedClass : AllowedClasses)
		{
			if (Actor->IsA(AllowedClass))
			{
				bIsClassAllowed = true;
				break;
			}
		}

		if (!bIsClassAllowed)
		{
			continue; // 如果Class不在AllowedClasses中，跳过这个Actor
		}

		// 检查这个Actor的所有StaticMeshComponent
		TArray<UStaticMeshComponent*> Components;
		Actor->GetComponents<UStaticMeshComponent>(Components);

		for (UStaticMeshComponent* Component : Components)
		{
			if (Component && Component->GetStaticMesh() == Mesh)
			{
				Result.Add(Actor);
				break; // 找到匹配的组件后，不需要检查这个Actor的其他组件
			}
		}
	}

	return Result;
}


TArray<FString> UAssetCheckToolBPLibrary::GetSelectedPathsInContentBrowser()
{
	// 获取Content Browser模块
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	// 用于存储选中的资产和文件夹的路径
	TArray<FString> SelectedPaths;

	// 获取当前选中的资产
	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

	// 遍历选中的资产，添加每个资产的路径到数组
	for (const FAssetData& Asset : SelectedAssets)
	{

#if ENGINE_MAJOR_VERSION >= 5
		SelectedPaths.Add(Asset.GetObjectPathString());
#else
		SelectedPaths.Add(Asset.ObjectPath.ToString());
#endif
	}

	// 获取当前选中的文件夹
	TArray<FString> SelectedFolders;
	ContentBrowserModule.Get().GetSelectedFolders(SelectedFolders);

	// 遍历选中的文件夹，添加每个文件夹的路径到数组
	for (const FString& Folder : SelectedFolders)
	{
		FString ConvertedPath = Folder;

		// 方案一：直接替换
		ConvertedPath.RemoveFromStart("/All");

		// 方案二：使用引擎API（二选一）
		// FPackageName::TryConvertFilenameToLongPackageName(Folder, ConvertedPath);

		SelectedPaths.Add(ConvertedPath);
	}
	if(SelectedPaths.Num()>0)
	{
		return SelectedPaths;
	}
	TArray<FString> SelectedPathViewFolders;
	ContentBrowserModule.Get().GetSelectedPathViewFolders(SelectedPathViewFolders);

	for (const FString& Folder : SelectedPathViewFolders)
	{
		FString ConvertedPath = Folder;

		// 方案一：直接替换
		ConvertedPath.RemoveFromStart("/All");

		// 方案二：使用引擎API（二选一）
		// FPackageName::TryConvertFilenameToLongPackageName(Folder, ConvertedPath);

		SelectedPaths.Add(ConvertedPath);
	}

	// 返回包含所有选中资产和文件夹路径的数组
	return SelectedPaths;
}

void UAssetCheckToolBPLibrary::GetAssetSourceControlStatus(
    const FString& PathName,
    bool bUpdateStatus,
    bool& OutIsCurrent,
    bool& OutIsCheckedOutByOthers,
    FString& OutCheckedOutByOtherUserName,
    bool& OutIsCheckedOutBySelf,
    bool& OutIsAdded,
    bool& OutIsUnknown)
{
	ISourceControlProvider& SCCProvider = ISourceControlModule::Get().GetProvider();

	// 直接使用单个文件路径
	const FString DependencyFilename = SourceControlHelpers::PackageFilenames({ PathName })[0];

	// 更新源控制状态（同步）
	if(bUpdateStatus)
	{
		SCCProvider.Execute(ISourceControlOperation::Create<FUpdateStatus>(), { DependencyFilename }, EConcurrency::Synchronous);
	}
	// 获取源控制状态
	FSourceControlStatePtr SCCState = SCCProvider.GetState(DependencyFilename, EStateCacheUsage::Use);

	if (SCCState.IsValid())
	{
		if(SCCState->IsUnknown())
		{
			SCCProvider.Execute(ISourceControlOperation::Create<FUpdateStatus>(), { DependencyFilename }, EConcurrency::Synchronous);
		}
		OutIsUnknown = SCCState->IsUnknown();
		OutIsCurrent = SCCState->IsCurrent();
		OutIsCheckedOutByOthers = SCCState->IsCheckedOutOther(&OutCheckedOutByOtherUserName);
		OutIsCheckedOutBySelf = SCCState->IsCheckedOut() && !SCCState->IsCheckedOutOther(&OutCheckedOutByOtherUserName);
		OutIsAdded = SCCState->IsAdded();

	}
	else
	{
		// Reset all outputs if the state is invalid
		OutIsCurrent = false;
		OutIsCheckedOutByOthers = false;
		OutCheckedOutByOtherUserName = TEXT("");
		OutIsCheckedOutBySelf = false;
		OutIsAdded = false;
		OutIsUnknown = false;
	}
}

FString UAssetCheckToolBPLibrary::GetAssetFileDiskPath(const FString& AssetPath)
{
	// 确保资产路径是有效的
	if (AssetPath.IsEmpty())
	{
		return "";
	}

	// 获取资产注册模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// 标准化路径
	FString StandardizedPath = ConvertResourcePath(AssetPath);
#if ENGINE_MAJOR_VERSION >= 5
	// 创建一个 Soft Object Path
	FSoftObjectPath SoftObjectPath(StandardizedPath);
	// 查找资产 using FSoftObjectPath
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(SoftObjectPath);
#else
	FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(FName(*StandardizedPath));
#endif


	if (AssetData.IsValid())
	{
		// 获取资产的包名
		FString PackageName = AssetData.PackageName.ToString();

		// 获取资产的类名
#if ENGINE_MAJOR_VERSION >= 5
		FString AssetClass = AssetData.AssetClassPath.ToString();
#else
		FString AssetClass = AssetData.AssetClass.ToString();
#endif
		// 根据资产类型确定文件扩展名
		FString FileExtension;
		if (AssetClass == "World") // 关卡资产的类名通常是 "World"
		{
			FileExtension = TEXT(".umap");
		}
		else
		{
			FileExtension = TEXT(".uasset");
		}

		// 构建文件的完整路径
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()) + PackageName.Replace(TEXT("/Game/"), TEXT("")) + FileExtension;
	}

	// 如果资产无效，返回空字符串
	return "";
}

void UAssetCheckToolBPLibrary::UpdateAssetSourceControlStatus(
	const TArray<FString>& PathNames)
{
	ISourceControlProvider& SCCProvider = ISourceControlModule::Get().GetProvider();

	// 调用转换函数
	//TArray<FString> PackageFilenames = SourceControlHelpers::PackageFilenames({ PathNames });
	TArray<FString> FilePaths;

	// 遍历所有传入的资产路径，并转换为文件系统路径
	for (const FString& AssetPath : PathNames)
	{
		FString FilePath = GetAssetFileDiskPath(AssetPath);
		if (!FilePath.IsEmpty())
		{
			FilePaths.Add(FilePath);
		}
	}
	// 更新源控制状态（同步）
	if (FilePaths.Num() > 0)
	{
		SCCProvider.Execute(ISourceControlOperation::Create<FUpdateStatus>(), FilePaths, EConcurrency::Synchronous);
		//SCCProvider.Execute(ISourceControlOperation::Create<FUpdateStatus>(), FilePaths, EConcurrency::Asynchronous);
	}
}

bool UAssetCheckToolBPLibrary::DeleteEmptyFolder(const FString& FolderPath)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// 获取指定文件夹中的资产
	TArray<FAssetData> AssetsInOriginalFolder;
	AssetRegistryModule.Get().GetAssetsByPath(*FolderPath, AssetsInOriginalFolder, true);

	// 检查文件夹是否为空
	if (AssetsInOriginalFolder.Num() == 0)
	{
		TArray<FString> FoldersToDelete;
		FoldersToDelete.Add(FolderPath);

		// 尝试删除文件夹
		bool bDeleted = AssetViewUtils::DeleteFolders(FoldersToDelete);
		if (bDeleted)
		{
			return true; // 返回成功
		}
		else
		{
			return false; // 返回失败
		}
	}
	else
	{
		return false; // 返回失败，因为文件夹不为空
	}
}

FString UAssetCheckToolBPLibrary::ConvertResourcePath(const FString& ResourcePath)
{
	// 提取目录路径，并确保以斜杠结尾
	FString DirectoryPath = FPaths::GetPath(ResourcePath);
	if (!DirectoryPath.EndsWith(TEXT("/")))
	{
		DirectoryPath += TEXT("/");
	}

	// 提取文件名（包括扩展名）
	FString FileNameWithExtension = FPaths::GetCleanFilename(ResourcePath);

	// 提取不带扩展名的文件名
	FString FileNameWithoutExtension = FPaths::GetBaseFilename(FileNameWithExtension);

	// 检查文件名是否已经包含文件名作为扩展名
	if (FileNameWithExtension == FileNameWithoutExtension + TEXT(".") + FileNameWithoutExtension)
	{
		// 如果已经是所需的格式，则直接返回原始路径
		return ResourcePath;
	}

	// 如果不是所需的格式，则进行转换
	FString ConvertedPath = DirectoryPath + FileNameWithoutExtension + TEXT(".") + FileNameWithoutExtension;

	return ConvertedPath;
}

FString UAssetCheckToolBPLibrary::GetMetaData(const FString& ObjectPath, const FString& Key)
{
	if (ObjectPath.IsEmpty())
	{
		return FString(); // 如果对象路径为空，直接返回空字符串
	}

	// 标准化路径

	FString StandardizedPath = ConvertResourcePath(ObjectPath);

	// 加载资产注册表模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

#if ENGINE_MAJOR_VERSION >= 5
	// 创建一个 Soft Object Path
	FSoftObjectPath SoftObjectPath(StandardizedPath);
	// 查找资产 using FSoftObjectPath
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(SoftObjectPath);
#else
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(*StandardizedPath);
#endif
	if (AssetData.IsValid())
	{
		// 加载资产
		UObject* LoadedObject = AssetData.GetAsset();
		if (LoadedObject)
		{
			// 获取元数据
			const FString& MetaDataString = UEditorAssetLibrary::GetMetadataTag(LoadedObject, FName(*Key));
			return MetaDataString;
		}
	}

	return FString(); // 如果没有找到或资产数据无效，返回空字符串
}


void UAssetCheckToolBPLibrary::GetEnumNames(const UEnum* InEnum, TArray<FString>& OutNames)
{
	// 清空输出数组
	OutNames.Empty();

	if (InEnum != nullptr)
	{
		// 遍历枚举值
		for (int32 i = 0; i < InEnum->NumEnums(); ++i)
		{
			// 获取枚举名称
			FString EnumName = InEnum->GetNameStringByIndex(i);

			// 检查是否为有效枚举值（排除 _MAX 和其他无效值）
			if (!EnumName.IsEmpty() && !EnumName.EndsWith(TEXT("_MAX")))
			{
				// 获取显示名称
				FText DisplayName = InEnum->GetDisplayNameTextByIndex(i);

				// 检查显示名称是否有效
				if (!DisplayName.IsEmpty())
				{
					OutNames.Add(DisplayName.ToString());
				}
			}
		}
	}
}

void UAssetCheckToolBPLibrary::GetEnumTips(const UEnum* InEnum, TArray<FString>& OutNames)
{
	if (InEnum != nullptr)
	{
		for (int32 i = 0; i < InEnum->NumEnums(); ++i)
		{
			FString EnumName = InEnum->GetNameStringByIndex(i);
			if (!EnumName.EndsWith(TEXT("_MAX")))
			{
				// 使用 GetToolTipTextByIndex 来获取枚举的描述
				OutNames.Add(InEnum->GetToolTipTextByIndex(i).ToString());
			}
		}
	}
}


static bool IsStringContainsKeyword(const FString& Str, const TArray<FString>& KeyWords)
{
	for (const FString& KeyWord : KeyWords)
	{
		if (Str.Contains(KeyWord)) return true;
	}
	return false;
}








TArray<FString> UAssetCheckToolBPLibrary::SortByElementAtIndex(const TArray<FString>& StringArray, const FString& SplitString, int32 Id) {
	TArray<TPair<FString, FString>> Elements;

	for (const FString& Str : StringArray) {
		TArray<FString> Items;
		Str.ParseIntoArray(Items, *SplitString, true); // 使用 Unreal 的 ParseIntoArray 方法

		if (Items.Num() > Id) {
			Elements.Add(TPair<FString, FString>(Items[Id], Str)); // 获取指定 ID 位置的元素
		}
	}

	// 按照指定 ID 位置的元素进行排序
	Elements.Sort([](const TPair<FString, FString>& A, const TPair<FString, FString>& B) {
		return A.Key < B.Key; // 按照元素的字典顺序排序
	});

	// 创建一个结果数组来存储排序后的字符串
	TArray<FString> SortedArray;
	for (const TPair<FString, FString>& Pair : Elements) {
		SortedArray.Add(Pair.Value); // 只提取原始字符串
	}

	return SortedArray;
}


// 实现解析函数
TArray<FKeyValueRow> UAssetCheckToolBPLibrary::ParseStringArray(const TArray<FString>& StringArray, const FString& Delimiter) {
	TArray<FKeyValueRow> Result;

	if (StringArray.Num() == 0) {
		return Result; // 如果输入数组为空，返回空结果
	}

	// 解析第一行，获取所有的 key
	TArray<FString> Keys;
	FString FirstLine = StringArray[0];
	FirstLine.ParseIntoArray(Keys, *Delimiter, true);

	// 解析剩余的行，填充对应的 value
	for (int32 i = 1; i < StringArray.Num(); ++i) {
		FKeyValueRow KeyValueRow; // 每一行的 KeyValueRow
		TArray<FString> Values;
		StringArray[i].ParseIntoArray(Values, *Delimiter, true);

		// 确保每一行的值数量与键数量相同
		for (int32 j = 0; j < Keys.Num() && j < Values.Num(); ++j) {
			FKeyValue KeyValuePair;
			KeyValuePair.Key = Keys[j];
			KeyValuePair.Value = Values[j]; // 将 value 设置为对应的字符串
			KeyValueRow.KeyValueArray.Add(KeyValuePair); // 添加到当前行的 KeyValue 数组
		}

		Result.Add(KeyValueRow); // 将当前行的 KeyValueRow 添加到结果数组
	}

	return Result;
}


// 实现排序函数
TArray<FKeyValueRow> UAssetCheckToolBPLibrary::SortKeyValueRowsByColumn(
    TArray<FKeyValueRow> KeyValueRows,
    int32 ColumnIndex,
    bool bDescending,
    TArray<int32>& SortedRowIDs) // 新增参数，用于输出排序后的行ID
{
    // 分离空值和非空值
    TArray<FKeyValueRow> NonEmptyRows;
    TArray<FKeyValueRow> EmptyRows;
    TArray<int32> NonEmptyRowIDs;
    TArray<int32> EmptyRowIDs;

    for (int32 i = 0; i < KeyValueRows.Num(); ++i) {
        const auto& Row = KeyValueRows[i];
        if (Row.KeyValueArray.IsValidIndex(ColumnIndex)) {
            FString Value = Row.KeyValueArray[ColumnIndex].Value.TrimStartAndEnd();
            if (Value.IsEmpty()) {
                EmptyRows.Add(Row);
                EmptyRowIDs.Add(i);
            } else {
                NonEmptyRows.Add(Row);
                NonEmptyRowIDs.Add(i);
            }
        } else {
            EmptyRows.Add(Row); // ColumnIndex 超出范围也视为“空”
            EmptyRowIDs.Add(i);
        }
    }

    // 将 NonEmptyRows 和 NonEmptyRowIDs 打包在一起
    TArray<TPair<FKeyValueRow, int32>> PackedRows;
    for (int32 i = 0; i < NonEmptyRows.Num(); ++i) {
        PackedRows.Add(TPair<FKeyValueRow, int32>(NonEmptyRows[i], NonEmptyRowIDs[i]));
    }

    // 对打包后的数组进行排序
    PackedRows.Sort([&](const TPair<FKeyValueRow, int32>& A, const TPair<FKeyValueRow, int32>& B) {
        if (A.Key.KeyValueArray.IsValidIndex(ColumnIndex) && B.Key.KeyValueArray.IsValidIndex(ColumnIndex)) {
            const FString& AValue = A.Key.KeyValueArray[ColumnIndex].Value.TrimStartAndEnd();
            const FString& BValue = B.Key.KeyValueArray[ColumnIndex].Value.TrimStartAndEnd();
            return bDescending ? (AValue > BValue) : (AValue < BValue);
        }
        return false;
    });

    // 解包排序后的数组
    NonEmptyRows.Empty();
    TArray<int32> SortedNonEmptyRowIDs; // 新增：用于存储排序后的非空行ID
    for (const auto& Pair : PackedRows) {
        NonEmptyRows.Add(Pair.Key);
        SortedNonEmptyRowIDs.Add(Pair.Value);
    }

    // 根据 bDescending 将空值数组添加到排序结果的适当位置
    if (bDescending) {
        // 降序：空值在末尾
        NonEmptyRows.Append(EmptyRows);
        SortedRowIDs = SortedNonEmptyRowIDs;
        SortedRowIDs.Append(EmptyRowIDs);
    } else {
        // 升序：空值在开头
        EmptyRows.Append(NonEmptyRows);
        SortedRowIDs = EmptyRowIDs;
        SortedRowIDs.Append(SortedNonEmptyRowIDs); // 修复：追加 SortedNonEmptyRowIDs
    }

    return bDescending ? NonEmptyRows : EmptyRows;
}

TArray<FString> UAssetCheckToolBPLibrary::FindReferencingPackages(const FString& AssetPath)
{
	TArray<FString> OutPackagePaths;

	// 获取资产注册模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// 将资产路径转换为包名
	FName PackageName = FName(*FPackageName::ObjectPathToPackageName(AssetPath));
	FAssetIdentifier AssetId(PackageName);

	// 查询引用该资产的包
	TArray<FAssetIdentifier> Referencers;
	AssetRegistryModule.Get().GetReferencers(AssetId, Referencers, UE::AssetRegistry::EDependencyCategory::Package);

	// 遍历引用包并输出包路径
	for (const FAssetIdentifier& RefId : Referencers)
	{
		FString PackagePath = RefId.PackageName.ToString();
		OutPackagePaths.Add(PackagePath);
	}

	return OutPackagePaths;
}

TArray<FString> UAssetCheckToolBPLibrary::FindAssetsReferencingPackages(const TArray<FString>& AssetPaths)
{
	TSet<FString> UniquePackagePaths; // 使用 TSet 自动去重

	// 获取资产注册模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// 遍历每个资产路径
	for (const FString& AssetPath : AssetPaths)
	{
		// 将资产路径转换为包名
		FName PackageName = FName(*FPackageName::ObjectPathToPackageName(AssetPath));
		FAssetIdentifier AssetId(PackageName);

		// 查询引用该资产的包
		TArray<FAssetIdentifier> Referencers;
		AssetRegistryModule.Get().GetReferencers(AssetId, Referencers, UE::AssetRegistry::EDependencyCategory::Package);

		// 遍历引用包并添加到 TSet 中
		for (const FAssetIdentifier& RefId : Referencers)
		{
			FString PackagePath = RefId.PackageName.ToString();
			UniquePackagePaths.Add(PackagePath); // TSet 会自动去重
		}
	}

	// 将 TSet 转换为 TArray 并返回
	return UniquePackagePaths.Array();
}

bool UAssetCheckToolBPLibrary::GetAssetData(const FString& PathName, FAssetData& OutAssetData)
{

	// 标准化路径
	FString StandardizedPath = ConvertResourcePath(PathName);

	// 加载资产注册表模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	// 获取指定路径的 AssetData

#if ENGINE_MAJOR_VERSION >= 5
	// 创建一个 Soft Object Path
	FSoftObjectPath SoftObjectPath(StandardizedPath);
	// 查找资产 using FSoftObjectPath
	OutAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(SoftObjectPath);
#else
	OutAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(*StandardizedPath);
#endif
	// 检查是否找到资产
	if (OutAssetData.IsValid())
	{
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No asset found at path: %s"), *StandardizedPath);
		return false;
	}
}



void UAssetCheckToolBPLibrary::NavigateToFolder(const FString& FolderPath) {
	// 获取 Content Browser 模块
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	// 处理路径：如果以/结尾则去掉
	FString ProcessedPath = FolderPath;
	if (ProcessedPath.EndsWith("/")) {
		ProcessedPath.LeftChopInline(1); // 移除最后一个字符
	}

	// 将路径转换为虚拟路径
	FName VirtualPath = *ProcessedPath;

	// 跳转到指定文件夹
	TArray<FString> Folders;
	Folders.Add(ProcessedPath);
	ContentBrowserModule.Get().SyncBrowserToFolders(Folders, false, true);
}


TArray<FString> UAssetCheckToolBPLibrary::GetSubFolders(const FString& Path, bool bRecursive)
{
	TArray<FString> SubFolders;

	// 加载 AssetRegistry 模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// 获取指定路径下的所有文件夹
	TArray<FString> FoundFolders;
	AssetRegistry.GetSubPaths(Path, FoundFolders, bRecursive);

	// 将结果添加到返回数组中
	for (const FString& Folder : FoundFolders)
	{
		SubFolders.Add(Folder);
	}

	return SubFolders;
}

TArray<FString> UAssetCheckToolBPLibrary::GetSubFoldersByArray(const TArray<FString>& Paths, bool bRecursive)
{
	TArray<FString> SubFolders;

	// 加载 AssetRegistry 模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// 遍历每个路径
	for (const FString& Path : Paths)
	{
		// 获取指定路径下的所有文件夹
		TArray<FString> FoundFolders;
		AssetRegistry.GetSubPaths(Path, FoundFolders, bRecursive);

		// 将结果添加到返回数组中
		for (const FString& Folder : FoundFolders)
		{
			SubFolders.Add(Folder);
		}
	}

	return SubFolders;
}
bool UAssetCheckToolBPLibrary::MoveAsset(const FString& SourcePath, const FString& TargetPath)
{
	// 获取 AssetRegistry 模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// 获取资产数据


#if ENGINE_MAJOR_VERSION >= 5
	// 创建一个 Soft Object Path
	FSoftObjectPath SoftObjectPath(*SourcePath);
	// 查找资产 using FSoftObjectPath
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(SoftObjectPath);
#else
	FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(FName(*SourcePath));
#endif


	if (!AssetData.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid source asset path: %s"), *SourcePath);
		return false;
	}

	// 获取资产工具接口
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	IAssetTools& AssetTools = AssetToolsModule.Get();

	// 移动资产
	TArray<FAssetRenameData> AssetsToRename;
	AssetsToRename.Add(FAssetRenameData(AssetData.GetAsset(), FPaths::GetPath(TargetPath), FPaths::GetBaseFilename(TargetPath)));

	if (AssetTools.RenameAssets(AssetsToRename) == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to rename asset: %s"), *SourcePath);
		return false;
	}

	return true;
}

void UAssetCheckToolBPLibrary::MoveAssets(const TArray<FString>& SourcePaths, const TArray<FString>& TargetPaths)
{
    if (SourcePaths.Num() != TargetPaths.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("SourcePaths and TargetPaths must have the same length!"));
        return;
    }

    // 获取 AssetTools 模块
    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
    IAssetTools& AssetTools = AssetToolsModule.Get();

    // 遍历路径数组，逐个移动资产
    for (int32 i = 0; i < SourcePaths.Num(); i++)
    {
        const FString& SourcePath = SourcePaths[i];
        const FString& TargetPath = TargetPaths[i];

        if (SourcePath.IsEmpty() || TargetPath.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("Empty path at index %d, skipping."), i);
            continue;
        }

        // 移动资产
        if (!MoveAsset(SourcePath, TargetPath))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to move asset from %s to %s"), *SourcePath, *TargetPath);
        }
    }
}


UPackage* UAssetCheckToolBPLibrary::GetClassPackage(TSubclassOf<UObject> InClass)
{
	return InClass->GetPackage();
}
FString UAssetCheckToolBPLibrary::GetCurrentDirectory()
{
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
}

void UAssetCheckToolBPLibrary::CompareStrings(const FString& String1, const FString& String2, bool bIgnoreCase, int32& OutLastMatchIndex, FString& OutMatchingSubstring)
{
	OutLastMatchIndex = -1;
	OutMatchingSubstring = "";

	int32 MinLength = FMath::Min(String1.Len(), String2.Len());

	for (int32 i = 0; i < MinLength; ++i)
	{
		TCHAR Char1 = bIgnoreCase ? FChar::ToLower(String1[i]) : String1[i];
		TCHAR Char2 = bIgnoreCase ? FChar::ToLower(String2[i]) : String2[i];

		if (Char1 != Char2)
		{
			break;
		}

		OutLastMatchIndex = i;
		OutMatchingSubstring += String1[i];
	}
}



EAssetRenameResult UAssetCheckToolBPLibrary::RenameAssetsWithDialog(const TArray<FAssetRenameData>& AssetsAndNames, bool bAutoCheckout)
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	return AssetTools.RenameAssetsWithDialog(AssetsAndNames, bAutoCheckout);
}

bool UAssetCheckToolBPLibrary::RenameAssets(const TArray<FAssetRenameData>& AssetsAndNames)
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	return AssetTools.RenameAssets(AssetsAndNames);
}

bool UAssetCheckToolBPLibrary::RenameAssetsWithRetries(const TArray<FAssetRenameData>& AssetsAndNames, int32 MaxRetries)
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	bool bSuccess = false;
	int32 RetryCount = 0;

	while (!bSuccess && RetryCount < MaxRetries)
	{
		bSuccess = AssetTools.RenameAssets(AssetsAndNames);
		RetryCount++;
	}

	return bSuccess;
}

EAssetRenameResult UAssetCheckToolBPLibrary::RenameAssetsWithDialogWithRetries(const TArray<FAssetRenameData>& AssetsAndNames, bool bAutoCheckout, int32 MaxRetries)
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	EAssetRenameResult Result = EAssetRenameResult::Failure;
	int32 RetryCount = 0;

	while (Result != EAssetRenameResult::Success && RetryCount < MaxRetries)
	{
		Result = AssetTools.RenameAssetsWithDialog(AssetsAndNames, bAutoCheckout);
		if (Result == EAssetRenameResult::Pending)
		{
			// Optionally handle pending state, e.g., wait or log
			// For now, we treat it as a reason to retry
		}
		RetryCount++;
	}

	return Result;
}


float UAssetCheckToolBPLibrary::CompareStringsSimilarity(const FString& StringA, const FString& StringB, bool bCaseSensitive)
{
	FString A = StringA;
	FString B = StringB;

	if (!bCaseSensitive)
	{
		A = A.ToLower();
		B = B.ToLower();
	}

	int32 MaxLength = FMath::Max(A.Len(), B.Len());
	if (MaxLength == 0) return 1.0f; // Both strings are empty

	int32 LevenshteinDist = ComputeLevenshteinDistance(A, B);
	return 1.0f - static_cast<float>(LevenshteinDist) / static_cast<float>(MaxLength);
}

int32 UAssetCheckToolBPLibrary::ComputeLevenshteinDistance(const FString& S1, const FString& S2)
{
	const size_t len1 = S1.Len(), len2 = S2.Len();
	TArray<int32> col, prevCol;

	col.Init(0, len2 + 1);
	prevCol.Init(0, len2 + 1);

	for (size_t i = 0; i <= len2; i++)
		prevCol[i] = i;

	for (size_t i = 0; i < len1; i++) {
		col[0] = i + 1;
		for (size_t j = 0; j < len2; j++) {
			col[j + 1] = std::min({ prevCol[j + 1] + 1, col[j] + 1, prevCol[j] + (S1[i] == S2[j] ? 0 : 1) });
		}
		Exchange(col, prevCol); // Use Exchange instead of Swap
	}
	return prevCol[len2];
}



TArray<AActor*> UAssetCheckToolBPLibrary::GetAllLevelActorsEditable()
{
	TArray<AActor*> Result;

	// 检查我们是否在编辑器中且不在播放模式
	if (GEditor && !GEditor->PlayWorld)
	{
		const EActorIteratorFlags Flags = EActorIteratorFlags::SkipPendingKill;
		for (TActorIterator<AActor> It(GEditor->GetEditorWorldContext().World(), AActor::StaticClass(), Flags); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor->IsEditable() &&
				Actor->IsListedInSceneOutliner() &&                 // 只添加允许在编辑器中被选择和显示的演员
				!Actor->IsTemplate() &&                             // 永远不应该发生，但我们不想要类默认对象（CDOs）
				!Actor->HasAnyFlags(RF_Transient) &&                // 不添加在非播放世界中的瞬态演员
				!Actor->IsA(AWorldSettings::StaticClass()))         // 不添加世界设置演员，尽管它技术上是可编辑的
			{
				Result.Add(Actor);
			}
		}
	}

	return Result;
}





bool UAssetCheckToolBPLibrary::RenameDirectory(const FString& OriginalPath, const FString& TargetPath)
{
	IFileManager& FileManager = IFileManager::Get();

	// 检查原始路径是否存在
	if (!FileManager.DirectoryExists(*OriginalPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Directory does not exist: %s"), *OriginalPath);
		return false;
	}

	// 尝试重命名目录
	if (FileManager.Move(*TargetPath, *OriginalPath, true, true, false, true))
	{
		UE_LOG(LogTemp, Log, TEXT("Directory renamed from %s to %s"), *OriginalPath, *TargetPath);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to rename directory from %s to %s"), *OriginalPath, *TargetPath);
		return false;
	}
}

FString UAssetCheckToolBPLibrary::ConvertVirtualToAbsolutePath(const FString& VirtualPath)
{
	// 将虚拟路径 `/Game/...` 转换为相对于项目内容目录的路径
	FString RelativePath = FPaths::ProjectContentDir();
	// 假设虚拟路径以 `/Game/` 开头，我们需要去掉 `/Game/` 并添加实际的内容目录路径
	if (VirtualPath.StartsWith(TEXT("/Game/")))
	{
		RelativePath += VirtualPath.Mid(6); // 从 `/Game/` 后面开始取剩余的路径部分
	}

	// 将相对路径转换为绝对路径
	FString AbsolutePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*RelativePath);

	return AbsolutePath;
}



void UAssetCheckToolBPLibrary::RefreshFolder(const FString& FolderPath)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().ScanPathsSynchronous({FolderPath}, true);
}





bool UAssetCheckToolBPLibrary::RegexMatch(const FString& InputString, const FString& Pattern, TArray<FString>& OutMatches, bool bCaseSensitive)
{
	OutMatches.Empty();

	try
	{
		// Convert FString to std::string
		std::string inputStr(TCHAR_TO_UTF8(*InputString));
		std::string patternStr(TCHAR_TO_UTF8(*Pattern));

		// Create a regex object with or without case insensitive flag based on bCaseSensitive
		std::regex_constants::syntax_option_type flags = bCaseSensitive ? std::regex_constants::ECMAScript : std::regex_constants::ECMAScript | std::regex_constants::icase;
		std::regex rx(patternStr, flags);
		std::sregex_iterator it(inputStr.begin(), inputStr.end(), rx);
		std::sregex_iterator end;

		bool bFoundMatch = false;

		while (it != end)
		{
			std::smatch match = *it;
			OutMatches.Add(FString(UTF8_TO_TCHAR(match.str().c_str())));
			bFoundMatch = true;
			++it;
		}

		return bFoundMatch;
	}
	catch (const std::regex_error& e)
	{
		UE_LOG(LogTemp, Error, TEXT("Regex error: %s"), UTF8_TO_TCHAR(e.what()));
		return false;
	}
	catch (...)
	{
		UE_LOG(LogTemp, Error, TEXT("Unknown regex error"));
		return false;
	}
}


bool UAssetCheckToolBPLibrary::RegexMatchWithScore(const FString& InputString, const FString& Pattern, bool bCaseSensitive, float& MatchScore, const FString& Delimiter, const float& math1Wight, const float& math2Wight, const float& math3Wight)
{
    // 输入验证
    if (InputString.IsEmpty() || Pattern.IsEmpty()) {
        MatchScore = 0.0f;
        return false;
    }

    std::string InputStr(TCHAR_TO_UTF8(*InputString));
    std::string PatternStr(TCHAR_TO_UTF8(*Pattern));

    // 去除模式中的 ^ 和 $ 符号
    PatternStr.erase(std::remove_if(PatternStr.begin(), PatternStr.end(),
                                   [](char c) { return c == '^' || c == '$'; }),
                    PatternStr.end());

    std::regex_constants::syntax_option_type CaseFlag = bCaseSensitive ? std::regex::ECMAScript : std::regex::ECMAScript | std::regex::icase;

    try {
        std::regex fullPattern(PatternStr, CaseFlag);
        if (std::regex_match(InputStr, fullPattern)) {
            MatchScore = 1.0f;
            return true;
        } else {
            std::vector<std::string> subPatterns;
            std::string delimiter = TCHAR_TO_UTF8(*Delimiter);

            // 根据 Delimiter 是否为空选择拆分方式
            if (Delimiter.IsEmpty()) {
                size_t start = 0;
                int bracketLevel = 0;
                for (size_t i = 0; i < PatternStr.size(); ++i) {
                    if (PatternStr[i] == '(') {
                        bracketLevel++;
                    } else if (PatternStr[i] == ')') {
                        bracketLevel--;
                    } else if (PatternStr[i] == '|' && bracketLevel == 0) {
                        subPatterns.push_back(PatternStr.substr(start, i - start));
                        start = i + 1;
                    }
                }
                subPatterns.push_back(PatternStr.substr(start));
            } else {
                size_t delimLen = delimiter.length();
                if (delimLen == 0) {
                    subPatterns.push_back(PatternStr);
                } else {
                    size_t start = 0;
                    int bracketLevel = 0;
                    for (size_t i = 0; i <= PatternStr.size() - delimLen; ) {
                        bool match = true;
                        for (size_t j = 0; j < delimLen; ++j) {
                            if (PatternStr[i + j] != delimiter[j]) {
                                match = false;
                                break;
                            }
                        }
                        if (match && bracketLevel == 0) {
                            subPatterns.push_back(PatternStr.substr(start, i - start));
                            i += delimLen;
                            start = i;
                        } else {
                            if (PatternStr[i] == '(') {
                                bracketLevel++;
                            } else if (PatternStr[i] == ')') {
                                bracketLevel--;
                            }
                            i++;
                        }
                    }
                    if (PatternStr.substr(start) != "") {
                        subPatterns.push_back(PatternStr.substr(start));
                    }
                }
            }

            if (subPatterns.empty()) {
                MatchScore = 0.0f;
                return false;
            }

            // 拆分被检字符串
            std::vector<std::string> subInputStrs;
            if (Delimiter.IsEmpty()) {
                // 如果没有分隔符，将整个字符串作为一个子串
                subInputStrs.push_back(InputStr);
            } else {
                size_t start = 0;
                size_t delimLen = delimiter.length();
                for (size_t i = 0; i <= InputStr.size() - delimLen; ) {
                    bool match = true;
                    for (size_t j = 0; j < delimLen; ++j) {
                        if (InputStr[i + j] != delimiter[j]) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        subInputStrs.push_back(InputStr.substr(start, i - start));
                        i += delimLen;
                        start = i;
                    } else {
                        i++;
                    }
                }
                if (InputStr.substr(start) != "") {
                    subInputStrs.push_back(InputStr.substr(start));
                }
            }
        	// 清除空的子模式
        	subPatterns.erase(std::remove_if(subPatterns.begin(), subPatterns.end(),
											[](const std::string& str) { return str.empty(); }),
							 subPatterns.end());

        	// 清除空的子输入
        	subInputStrs.erase(std::remove_if(subInputStrs.begin(), subInputStrs.end(),
											 [](const std::string& str) { return str.empty(); }),
							  subInputStrs.end());
            // 如果子模式数量与子输入数量不匹配，直接返回失败
            if (subPatterns.size() != subInputStrs.size()) {
			    int foundCount = 0;

			    // 检查每个子模式是否能在输入字符串中找到
			    for (const auto& subPattern : subPatterns) {
			        std::regex pattern(subPattern, CaseFlag);
			        std::smatch matchResult;

			        // 检查子模式是否在输入字符串中出现
			        if (std::regex_search(InputStr, matchResult, pattern)) {
			            foundCount++;  // 如果找到子模式则计数加1
			        }
			    }

			    // 根据找到的子模式数量计算匹配分数
			    MatchScore = static_cast<float>(foundCount) / std::max( static_cast<float>(subPatterns.size()),static_cast<float>(subInputStrs.size()))*.8;//降低点数量不匹配的分数
			    return false;  // 如果找到任意子模式返回true，否则返回false

            }

            // 初始化匹配分数
            float matchedScore = 0.0f;

            // 逐一对子模式和子输入进行匹配
            for (size_t i = 0; i < subPatterns.size(); ++i) {
                const std::string& subPattern = subPatterns[i];
                const std::string& subInputStr = subInputStrs[i];

                std::regex pattern(subPattern, CaseFlag);
                std::smatch matchResult;

                // 检测子模式是否匹配
                if (std::regex_match(subInputStr, matchResult, pattern)) {
                    matchedScore += 1.0f;
                } else {
                    // 转换为 FString 进行模糊检测
                    FString inputF = UTF8_TO_TCHAR(subInputStr.c_str());
                    FString subPatternF = UTF8_TO_TCHAR(subPattern.c_str());

                    // 计算模糊匹配分数
                    float similarity1 = PrefixWeightedSimilarity(inputF, subPatternF);
                    float similarity2 = CalculateJaccardSimilarity(inputF, subPatternF);
                    //float similarity2 = SmithWatermanAlignment(inputF, subPatternF);
                	float similarity3 = CheckNumericMatch(inputF, subPatternF);


                    float thistscore = similarity1 * math1Wight + similarity2 * math2Wight + similarity3 * math3Wight;
                    matchedScore += std::max(std::min(thistscore, 0.99f), 0.0f);
                }
            }
			float bad =  abs(static_cast<float>(subInputStrs.size()) - static_cast<float>(subPatterns.size())); // 子集不同 ，降低不匹配的分数

            // 计算匹配分数
            MatchScore = matchedScore / static_cast<float>(subInputStrs.size()) - bad/subInputStrs.size()*.1;
        	//MatchScore = matchedScore / static_cast<float>(subInputStrs.size()) ;
        }
    } catch (const std::regex_error& e) {
        UE_LOG(LogTemp, Warning, TEXT("Regex error: %s"), *FString(e.what()));
        MatchScore = 0.0f;
        return false;
    }

    return false;
}

float UAssetCheckToolBPLibrary::CheckNumericMatch(const FString& Str1, const FString& Str2)
{
        // 提取字符串中的数字部分
        auto ExtractNumbers = [](const FString& Str) -> FString {
                FString Result;
                for (TCHAR Char : Str)
                {
                        if (FChar::IsDigit(Char))
                        {
                                Result += Char;
                        }
                }
                return Result;
        };

        FString Numbers1 = ExtractNumbers(Str1);
        FString Numbers2 = ExtractNumbers(Str2);
        if(Numbers1 == Numbers2) return 1;
        if(Numbers1.IsEmpty() || Numbers2.IsEmpty()) return 0;

        int32 Int1 = FCString::Atoi(*Numbers1);
        int32 Int2 = FCString::Atoi(*Numbers2);
        float f = Int1==Int2?1:(1- FMath::Clamp(abs(Int1-Int2)/10.0,0,1))*0.5;

        int32 Len1 = Numbers1.Len();
        int32 Len2 = Numbers2.Len();

        // 动态规划表（二维数组）
        TMap<FIntPoint, int32> DP; // 使用 FIntPoint (i, j) 作为键

        // 初始化边界条件
        for (int32 i = 0; i <= Len1; ++i)
        {
                DP.Add(FIntPoint(i, 0), i);
        }
        for (int32 j = 0; j <= Len2; ++j)
        {
                DP.Add(FIntPoint(0, j), j);
        }

        // 填充 DP 表
        for (int32 i = 1; i <= Len1; ++i)
        {
                for (int32 j = 1; j <= Len2; ++j)
                {
                        const TCHAR Char1 = Str1[i - 1];
                        const TCHAR Char2 = Str2[j - 1];

                        if (Char1 == Char2)
                        {
                                // 字符相同，无需操作
                                DP.Add(FIntPoint(i, j), DP[FIntPoint(i - 1, j - 1)]);
                        }
                        else
                        {
                                // 取插入、删除、替换的最小值 +1
                                const int32 Insert = DP[FIntPoint(i, j - 1)] + 1;
                                const int32 Delete = DP[FIntPoint(i - 1, j)] + 1;
                                const int32 Replace = DP[FIntPoint(i - 1, j - 1)] + 1;

                                DP.Add(FIntPoint(i, j), FMath::Min(FMath::Min(Insert, Delete), Replace));
                        }
                }
        }

        const int32 Distance = DP[FIntPoint(Len1, Len2)];
        const int32 MaxLen = FMath::Max(Len1, Len2);
        float LevenshteinDistance = 1.0f - (static_cast<float>(Distance) / MaxLen);

        float result = f*0.7+ LevenshteinDistance*0.3;
        return result;
}

// 生成 N-gram 集合
TSet<FString> GenerateNGrams(const FString& InputString, int32 N)
{
	TSet<FString> NGrams;
	for (int32 i = 0; i <= InputString.Len() - N; ++i)
	{
		FString NGram = InputString.Mid(i, N);
		NGrams.Add(NGram);
	}
	return NGrams;
}




// 计算 Jaccard 相似度
float UAssetCheckToolBPLibrary::CalculateJaccardSimilarity(const FString& StringA, const FString& StringB)
{
	// 生成 N-gram 集合（这里使用 2-gram）
	TSet<FString> NGramsA = GenerateNGrams(StringA, 2);
	TSet<FString> NGramsB = GenerateNGrams(StringB, 2);

	// 计算交集
	TSet<FString> Intersection = NGramsA.Intersect(NGramsB);

	// 计算并集
	TSet<FString> Union = NGramsA.Union(NGramsB);

	// 避免除以零
	if (Union.Num() == 0)
	{
		return 0.0f;
	}

	// 计算 Jaccard 相似度
	return static_cast<float>(Intersection.Num()) / static_cast<float>(Union.Num());
}

float UAssetCheckToolBPLibrary::SmithWatermanAlignment(const FString& StringA, const FString& StringB)
{
	const int32 MatchScore = 2;      // 匹配得分
	const int32 MismatchScore = -1;  // 不匹配得分
	const int32 GapScore = -1;       // 插入/删除得分

	int32 LenA = StringA.Len();
	int32 LenB = StringB.Len();

	// 初始化得分矩阵
	TArray<TArray<int32>> ScoreMatrix;
	ScoreMatrix.SetNum(LenA + 1);
	for (int32 i = 0; i <= LenA; ++i)
	{
		ScoreMatrix[i].SetNum(LenB + 1);
	}

	// 填充得分矩阵
	int32 MaxScore = 0; // 记录最大得分
	for (int32 i = 1; i <= LenA; ++i)
	{
		for (int32 j = 1; j <= LenB; ++j)
		{
			int32 Match = ScoreMatrix[i - 1][j - 1] + (StringA[i - 1] == StringB[j - 1] ? MatchScore : MismatchScore);
			int32 Delete = ScoreMatrix[i - 1][j] + GapScore;
			int32 Insert = ScoreMatrix[i][j - 1] + GapScore;
			ScoreMatrix[i][j] = FMath::Max(0, FMath::Max(Match, FMath::Max(Delete, Insert)));

			// 更新最大得分
			if (ScoreMatrix[i][j] > MaxScore)
			{
				MaxScore = ScoreMatrix[i][j];
			}
		}
	}

	// 计算理论上的最大可能得分
	int32 MaxPossibleScore = MatchScore * FMath::Min(LenA, LenB);

	// 归一化得分到 [0, 1] 范围
	float NormalizedScore = (MaxPossibleScore > 0) ? static_cast<float>(MaxScore) / static_cast<float>(MaxPossibleScore) : 0.0f;

	return NormalizedScore; // 返回归一化后的得分
}


float UAssetCheckToolBPLibrary::PrefixWeightedSimilarity(const FString& StringA, const FString& StringB)
{
	int32 PrefixLength = 0;
	for (int32 i = 0; i < FMath::Min(StringA.Len(), StringB.Len()); ++i)
	{
		if (StringA[i] == StringB[i])
		{
			PrefixLength++;
		}
		else
		{
			break;
		}
	}

	float PrefixWeight = 0.9f; // 前缀权重
	float SuffixWeight = 0.1f; // 后缀权重

	float PrefixScore = static_cast<float>(PrefixLength) / FMath::Max(StringA.Len(), StringB.Len());
	float SuffixScore = CalculateJaccardSimilarity(StringA, StringB); // 使用 Jaccard 相似度计算后缀匹配

	return PrefixWeight * PrefixScore + SuffixWeight * SuffixScore;
}

float UAssetCheckToolBPLibrary::JaroWinklerDistance(const FString& StringA, const FString& StringB)
{
    const FString& A = StringA;
    const FString& B = StringB;

    // 如果两个字符串相同，直接返回 1.0
    if (A == B)
    {
        return 1.0f;
    }

    // 计算匹配窗口大小
    int32 MaxLength = FMath::Max(A.Len(), B.Len());
    int32 MatchWindow = (MaxLength / 2) - 1;

    // 初始化匹配标志数组
    TArray<bool> MatchesA;
    MatchesA.Init(false, A.Len());
    TArray<bool> MatchesB;
    MatchesB.Init(false, B.Len());

    // 计算匹配字符数
    int32 Matches = 0;
    for (int32 i = 0; i < A.Len(); ++i)
    {
        int32 Start = FMath::Max(0, i - MatchWindow);
        int32 End = FMath::Min(B.Len() - 1, i + MatchWindow);

        for (int32 j = Start; j <= End; ++j)
        {
            if (!MatchesB[j] && A[i] == B[j])
            {
                MatchesA[i] = true;
                MatchesB[j] = true;
                Matches++;
                break;
            }
        }
    }

    // 如果没有匹配字符，返回 0.0
    if (Matches == 0)
    {
        return 0.0f;
    }

    // 计算匹配字符的顺序一致性（transpositions）
    int32 Transpositions = 0;
    int32 k = 0;
    for (int32 i = 0; i < A.Len(); ++i)
    {
        if (MatchesA[i])
        {
            while (!MatchesB[k])
            {
                k++;
            }
            if (A[i] != B[k])
            {
                Transpositions++;
            }
            k++;
        }
    }

    // 计算 Jaro 距离
    float JaroDistance = (static_cast<float>(Matches) / A.Len() +
                          static_cast<float>(Matches) / B.Len() +
                          (static_cast<float>(Matches) - Transpositions / 2.0f) / Matches) / 3.0f;

    // 计算 Jaro-Winkler 距离
    float PrefixScale = 0.1f; // 前缀权重因子
    int32 PrefixLength = 0;
    for (int32 i = 0; i < FMath::Min(4, FMath::Min(A.Len(), B.Len())); ++i)
    {
        if (A[i] == B[i])
        {
            PrefixLength++;
        }
        else
        {
            break;
        }
    }

    float JaroWinklerDistance = JaroDistance + (PrefixLength * PrefixScale * (1.0f - JaroDistance));

    return JaroWinklerDistance;
}

float UAssetCheckToolBPLibrary::GetFuzzyMatchScore(const FString& InputString, const FString& TargetString)
{
	int32 Distance = ComputeLevenshteinDistance(InputString, TargetString);
	int32 MaxLength = FMath::Max(InputString.Len(), TargetString.Len());

	if (MaxLength == 0) return 1.0f; // 完全匹配
	float Similarity = 1.0f - (float)Distance / (float)MaxLength;

	return FMath::Clamp(Similarity, 0.0f, 1.0f); // 返回0到1之间的分值
}

// 生成N-gram集合
TSet<FString> UAssetCheckToolBPLibrary::GenerateNGrams(const FString& Input, int32 N)
{
	TSet<FString> NGrams;
	if (N <= 0 || Input.Len() < N) {
		return NGrams;
	}

	for (int32 i = 0; i <= Input.Len() - N; ++i) {
		NGrams.Add(Input.Mid(i, N));
	}
	return NGrams;
}



FString UAssetCheckToolBPLibrary::GetUniqueFilename(const FString& InputPath)
{
	// 分割路径和文件名	FString DirPart;
	FString FilePart;
	FString ExtensionPart;
	FString DirPart;
	FPaths::Split(InputPath, DirPart, FilePart,  ExtensionPart);


	int32 Counter = 1;

	while (true)
	{
		FString NewFilename = FString::Printf(TEXT("%s_%d.%s"), *FilePart, Counter, *ExtensionPart);
		FString NewPath = FPaths::Combine(DirPart, NewFilename);

		if (!FPaths::FileExists(NewPath))
		{
			return NewFilename;
		}

		Counter++;
	}
}

TArray<FString> UAssetCheckToolBPLibrary::FindFilePaths(const FString& DirectoryPath, bool bRecursive, const FString& FileExtension)
{
	TArray<FString> ResultFiles;
	FindDirectoryInternal(DirectoryPath, bRecursive, FileExtension, ResultFiles);
	return ResultFiles;
}

void UAssetCheckToolBPLibrary::FindDirectoryInternal(const FString& DirectoryPath, bool bRecursive, const FString& FileExtension, TArray<FString>& OutFiles)
{
	// 获取目录下所有文件和文件夹
	TArray<FString> FoundFiles;
	IFileManager::Get().FindFiles(FoundFiles, *DirectoryPath, *FileExtension);

	// 添加完整路径
	for (const FString& File : FoundFiles)
	{
		OutFiles.Add(FPaths::Combine(DirectoryPath, File));
	}

	if (bRecursive)
	{
		// 获取所有子目录
		TArray<FString> FoundDirectories;
		IFileManager::Get().FindFiles(FoundDirectories, *FPaths::Combine(DirectoryPath, TEXT("*")), false, true);

		// 递归扫描子目录
		for (const FString& Dir : FoundDirectories)
		{
			FindDirectoryInternal(FPaths::Combine(DirectoryPath, Dir), true, FileExtension, OutFiles);
		}
	}
}
/*
void UAssetCheckToolBPLibrary::RegexMatchAndReplace(const FString& InputPath, const FString& Pattern, FString& OutputPath)
{
	TArray<FString> InputSegments;
	TArray<FString> PatternSegments;
	FString PathPart, Filename, Extension;

	// 分割输入路径和正则表达式模式
	FPaths::Split(InputPath, PathPart, Filename, Extension);
	FString FullPath = FPaths::Combine(PathPart, Filename + (Extension.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(".%s"), *Extension)));
	FullPath.ParseIntoArray(InputSegments, TEXT("/"));
	Pattern.ParseIntoArray(PatternSegments, TEXT("/"));

	TArray<FString> OutputSegments;

	// 遍历 PatternSegments，逐个查找匹配项
	for (const FString& PatternSegment : PatternSegments)
	{
		bool bFoundMatch = false;

		// 使用正则表达式在 InputSegments 中查找匹配项
		FRegexPattern RegexPattern(PatternSegment);
		for (const FString& InputSegment : InputSegments)
		{
			FRegexMatcher Matcher(RegexPattern, InputSegment);
			if (Matcher.FindNext())
			{
				OutputSegments.Add(InputSegment); // 找到匹配项，添加到 OutputSegments
				bFoundMatch = true;
				break; // 找到匹配项后跳出循环
			}
		}

		// 如果未找到匹配项，将 PatternSegment 添加到 OutputSegments
		if (!bFoundMatch)
		{
			OutputSegments.Add(PatternSegment);
		}
	}

	// 构建 OutputPath
	OutputPath = FString::Join(OutputSegments, TEXT("/"));

	// 保留原始路径格式（绝对路径或相对路径）
	if (InputPath.StartsWith(TEXT("/")))
	{
		OutputPath = TEXT("/") + OutputPath;
	}
	if (InputPath.EndsWith("/"))
	{
		OutputPath = OutputPath + TEXT("/");
	}
}*/
// 计算两个FString的Jaccard相似度（基于字符集合）
float CalculateJaccardSimilarity(const FString& Str1, const FString& Str2)
{
    // 创建字符集合
    TSet<TCHAR> Set1;
    for (const TCHAR& Char : Str1)
    {
        Set1.Add(Char);
    }

    TSet<TCHAR> Set2;
    for (const TCHAR& Char : Str2)
    {
        Set2.Add(Char);
    }

    // 计算交集
    TSet<TCHAR> Intersection = Set1.Intersect(Set2);

    // 计算并集
    TSet<TCHAR> Union = Set1.Union(Set2);

    // 避免除以零
    if (Union.Num() == 0) return 0.0f;

    // 返回Jaccard相似度
    return (float)Intersection.Num() / Union.Num();
}

void UAssetCheckToolBPLibrary::RegexMatchAndReplace(
    const FString& InputPath,
    const FString& Pattern,
    FString& OutputPath,
    float math1Wight,
    float math2Wight,
    float math3Wight)
{
    // 步骤1：路径分割
    TArray<FString> InputSegments;
    TArray<FString> PatternSegments;
    FString PathPart, Filename, Extension;

    FPaths::Split(InputPath, PathPart, Filename, Extension);
    FString FullPath = FPaths::Combine(PathPart,
        Filename + (Extension.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(".%s"), *Extension)));
    FullPath.ParseIntoArray(InputSegments, TEXT("/"));
    Pattern.ParseIntoArray(PatternSegments, TEXT("/"));


    TArray<FString> OutputSegments;
    int32 CurrentSegmentIndex = 0;

    // 步骤2：逐级处理模式段
    for (const FString& PatternSeg : PatternSegments)
    {
		if (PatternSegments.Num() >= InputSegments.Num()) {
		    // 只检查下一个输入段（如果存在）
		    if (CurrentSegmentIndex < InputSegments.Num()) {
		        FRegexPattern RegexPattern(PatternSeg);
		        FRegexMatcher Matcher(RegexPattern, InputSegments[CurrentSegmentIndex]);

		        // 首先尝试大小写敏感匹配
		        if (Matcher.FindNext()) {
		            OutputSegments.Add(InputSegments[CurrentSegmentIndex]);
		            CurrentSegmentIndex++;
		        }
		        else {
		            // 如果大小写敏感匹配失败，尝试大小写不敏感匹配
		            FRegexPattern CaseInsensitivePattern(PatternSeg, ERegexPatternFlags::CaseInsensitive);
		            FRegexMatcher CaseInsensitiveMatcher(CaseInsensitivePattern, InputSegments[CurrentSegmentIndex]);

		            if (CaseInsensitiveMatcher.FindNext()) {
		                // 大小写不敏感匹配成功，仅修正字母字符的大小写
		                FString CorrectedString = InputSegments[CurrentSegmentIndex];

		                // 创建字母字符映射表 - 仅分析模式字符串中的字母字符
		                TMap<int32, TCHAR> LetterCharMap;
		                bool bInEscapeSequence = false;
		                bool bInCharacterClass = false;
		                int32 patternIndex = 0;
		                int32 literalIndex = 0;

		                while (patternIndex < PatternSeg.Len()) {
		                    TCHAR currentChar = PatternSeg[patternIndex];

		                    if (bInEscapeSequence) {
		                        // 转义字符之后的字母字符会被记录
		                        if ((currentChar >= 'A' && currentChar <= 'Z') ||
		                            (currentChar >= 'a' && currentChar <= 'z')) {
		                            LetterCharMap.Add(literalIndex, currentChar);
		                        }
		                        bInEscapeSequence = false;
		                    }
		                    else if (currentChar == '\\') {
		                        bInEscapeSequence = true;
		                    }
		                    else if (currentChar == '[') {
		                        bInCharacterClass = true;
		                    }
		                    else if (currentChar == ']' && bInCharacterClass) {
		                        bInCharacterClass = false;
		                    }
		                    else if (!bInCharacterClass &&
		                            (currentChar >= 'A' && currentChar <= 'Z') ||
		                            (currentChar >= 'a' && currentChar <= 'z')) {
		                        // 这是一个字母字符（非正则表达式元字符）
		                        LetterCharMap.Add(literalIndex, currentChar);
		                    }

		                    patternIndex++;
		                    if (!bInEscapeSequence && currentChar != '\\') {
		                        literalIndex++;
		                    }
		                }

		                // 仅修正字母字符的大小写
		                for (const auto& Entry : LetterCharMap) {
		                    if (Entry.Key < CorrectedString.Len()) {
		                        TCHAR inputChar = CorrectedString[Entry.Key];
		                        // 仅当目标字符是字母时才修正
		                        if ((inputChar >= 'A' && inputChar <= 'Z') ||
		                            (inputChar >= 'a' && inputChar <= 'z')) {
		                            CorrectedString[Entry.Key] = Entry.Value;
		                        }
		                    }
		                }

		                OutputSegments.Add(CorrectedString);
		                CurrentSegmentIndex++;
		            }
		            else {
		                // 两种匹配方式都失败，添加模式段
		                OutputSegments.Add(PatternSeg);
		            }
		        }
		    }
		    else {
		        OutputSegments.Add(PatternSeg);
		    }
		    continue;
		}

        bool bFoundMatch = false;
        const FString& CurrentInputSeg = InputSegments[CurrentSegmentIndex];

        // 步骤3：处理选项模式 (aaa|bbb|ccc)
        if (PatternSeg.StartsWith(TEXT("(")) && PatternSeg.EndsWith(TEXT(")")))
        {
            TArray<FString> Options;
            PatternSeg.Mid(1, PatternSeg.Len()-2).ParseIntoArray(Options, TEXT("|"));

            FString BestMatch;
            float MaxSimilarity = -1.0f; // 使用相似度最大值而非距离最小值

        	for (const FString& Option : Options)
        	{
        		FRegexPattern RegexPattern(Option);
        		FRegexMatcher Matcher(RegexPattern, CurrentInputSeg);

        		// 优先检查完全匹配
        		if (Matcher.FindNext())
        		{
        			// 找到直接匹配项，直接返回当前输入段
        			BestMatch = Option;  // 使用pattern
        			bFoundMatch = true;
        			OutputSegments.Add(BestMatch);
        			break; // 找到匹配立即跳出循环
        		}
        	}

        	// 如果没有找到直接匹配，则计算相似度
        	if (!bFoundMatch)
        	{
        		for (const FString& Option : Options)
        		{
        			// 计算Jaccard相似度

        			float similarity1 = PrefixWeightedSimilarity(CurrentInputSeg, Option);
        			float similarity2 = CalculateJaccardSimilarity(CurrentInputSeg, Option);
        			//float similarity2 = SmithWatermanAlignment(CurrentInputSeg, Option);
        			float similarity3 = CheckNumericMatch(CurrentInputSeg, Option);

        			float Similarity = similarity1 * math1Wight + similarity2 * math2Wight + similarity3 * math3Wight;
        			if (Similarity > MaxSimilarity)
        			{
        				MaxSimilarity = Similarity;
        				BestMatch = Option;
        			}
        		}
        		bFoundMatch = (MaxSimilarity >= 0);
        	}

            if (MaxSimilarity >= 0)
            {
                OutputSegments.Add(BestMatch);
                bFoundMatch = true;

            }
        }

    	// 步骤4：普通模式处理
		if (!bFoundMatch)
		{
		    // 尝试直接匹配（区分大小写）
			FString ModifiedPatternSeg = TEXT("^") + PatternSeg + TEXT("$");
		    FRegexPattern RegexPattern(ModifiedPatternSeg);
		    FRegexMatcher Matcher(RegexPattern, CurrentInputSeg);

		    if (Matcher.FindNext())
		    {
		        OutputSegments.Add(CurrentInputSeg);
		    }
		    else
		    {
		        // 尝试大小写不敏感的匹配
		        FRegexPattern CaseInsensitivePattern(FString::Printf(TEXT("(?i)%s"), *PatternSeg));
		        FRegexMatcher CaseInsensitiveMatcher(CaseInsensitivePattern, CurrentInputSeg);

		        if (CaseInsensitiveMatcher.FindNext())
		        {
		            // 大小写不敏感匹配成功，现在需要根据模式修正大小写
		            FString CorrectedString = CurrentInputSeg;

		            // 创建字面量字符映射表 - 分析模式字符串，找出非特殊字符
		            TMap<int32, TCHAR> LiteralCharMap;
		            bool bInEscapeSequence = false;
		            bool bInCharacterClass = false;
		            int32 patternIndex = 0;
		            int32 literalIndex = 0;

		            while (patternIndex < PatternSeg.Len())
		            {
		                TCHAR currentChar = PatternSeg[patternIndex];

		                if (bInEscapeSequence)
		                {
		                    // 转义字符之后的字符总是字面量，除非是特殊的正则表达式转义序列
		                    // 例如 \d, \w 等不是字面量，但 \. 是作为字面量的点
		                    if (currentChar != 'd' && currentChar != 'w' && currentChar != 's' &&
		                        currentChar != 'D' && currentChar != 'W' && currentChar != 'S' &&
		                        currentChar != 'b' && currentChar != 'B' && currentChar != 'n' &&
		                        currentChar != 't' && currentChar != 'r' && currentChar != 'f' &&
		                        currentChar != 'v')
		                    {
		                        LiteralCharMap.Add(literalIndex, currentChar);
		                    }
		                    bInEscapeSequence = false;
		                }
		                else if (currentChar == '\\')
		                {
		                    bInEscapeSequence = true;
		                }
		                else if (currentChar == '[')
		                {
		                    bInCharacterClass = true;
		                }
		                else if (currentChar == ']' && bInCharacterClass)
		                {
		                    bInCharacterClass = false;
		                }
		                else if (!bInCharacterClass &&
		                         currentChar != '.' && currentChar != '*' && currentChar != '+' &&
		                         currentChar != '?' && currentChar != '^' && currentChar != '$' &&
		                         currentChar != '(' && currentChar != ')' && currentChar != '{' &&
		                         currentChar != '}' && currentChar != '|')
		                {
		                    // 这是一个普通字符（非正则表达式元字符）
		                    LiteralCharMap.Add(literalIndex, currentChar);
		                }

		                patternIndex++;
		                if (!bInEscapeSequence && currentChar != '\\')
		                {
		                    literalIndex++;
		                }
		            }

		            // 将LiteralCharMap应用到原始字符串，只修正字母的大小写
		            for (const auto& Pair : LiteralCharMap)
		            {
		                int32 Index = Pair.Key;
		                TCHAR PatternChar = Pair.Value;

		                if (Index < CorrectedString.Len() && FChar::IsAlpha(PatternChar) && FChar::IsAlpha(CorrectedString[Index]))
		                {
		                    // 根据模式字符的大小写修正输入字符串
		                    if (FChar::IsUpper(PatternChar))
		                    {
		                        CorrectedString[Index] = FChar::ToUpper(CorrectedString[Index]);
		                    }
		                    else if (FChar::IsLower(PatternChar))
		                    {
		                        CorrectedString[Index] = FChar::ToLower(CorrectedString[Index]);
		                    }
		                }
		            }

		            //OutputSegments.Add(CorrectedString);

		        	std::string InputStr(TCHAR_TO_UTF8(*CorrectedString));
		        	std::string PatternStr(TCHAR_TO_UTF8(*PatternSeg));
		        	std::regex_constants::syntax_option_type CaseFlag = std::regex::ECMAScript;
		        	std::regex fullPattern(PatternStr, CaseFlag);
		        	if (std::regex_match(InputStr, fullPattern))
		        	{
		        		OutputSegments.Add(CorrectedString);
		        	}
			        else
			        {
			        	OutputSegments.Add(PatternSeg);
			        }
		        }
		        else
		        {
		            // 完全不匹配，使用模式字符串
		            OutputSegments.Add(PatternSeg);
		        }
		    }
		}
		CurrentSegmentIndex++;

    }

    // 步骤5：构建输出路径
    OutputPath = FString::Join(OutputSegments, TEXT("/"));

    // 保留路径格式
    if (InputPath.StartsWith(TEXT("/"))) OutputPath = TEXT("/") + OutputPath;
    if (InputPath.EndsWith(TEXT("/"))) OutputPath += TEXT("/");
}

bool UAssetCheckToolBPLibrary::RenameFolders(const FString& DestPath, const FString& SourcePath)
{
	return AssetViewUtils::RenameFolder(DestPath, SourcePath);
}


void UAssetCheckToolBPLibrary::SetSelectedAssetByPath(const TArray<FString> AssetPathArray)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> AssetsToSelect;

	for (const FString& AssetPath : AssetPathArray)
	{

		// 如果路径包含扩展名，则移除扩展名尝试获取包资产
		FString PackagePath = AssetPath;
		int32 DotIndex;
		if (PackagePath.FindLastChar('.', DotIndex))
		{
			PackagePath = PackagePath.Left(DotIndex);
		}


		TArray<FAssetData> AssetDatas;
		AssetRegistryModule.Get().GetAssetsByPackageName(FName(*PackagePath), AssetDatas, false);
		if (AssetDatas.Num() > 0)
		{
			AssetsToSelect.Append(AssetDatas);
		}
	}

	if (AssetsToSelect.Num() > 0)
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().SyncBrowserToAssets(AssetsToSelect, true, true);
	}
}

float UAssetCheckToolBPLibrary::GetLightMapResolution(UStaticMesh* StaticMesh)
{
	if (StaticMesh)
	{
		// Use the getter method to access the LightMapResolution
		return StaticMesh->GetLightMapResolution();
	}
	return 0.0f;
}


int32 UAssetCheckToolBPLibrary::GetVisibleUVSetCount(UStaticMesh* StaticMesh, int32 LODIndex, float Tolerance )
{
	if (!StaticMesh || !StaticMesh->GetRenderData())
	{
		UE_LOG(LogTemp, Warning, TEXT("StaticMesh is invalid or has no render data."));
		return -1; // 返回 -1 表示错误
	}

	// 获取静态网格的渲染数据
	const FStaticMeshRenderData* RenderData = StaticMesh->GetRenderData();

	// 检查 LODIndex 是否有效
	if (LODIndex < 0 || LODIndex >= RenderData->LODResources.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid LODIndex: %d"), LODIndex);
		return -1; // 返回 -1 表示错误
	}

	// 获取指定 LOD 的资源
	const FStaticMeshLODResources& LODResource = RenderData->LODResources[LODIndex];

	// 获取当前 LOD 的 UV 集数量
	int32 UVSetCount = LODResource.GetNumTexCoords();

	// 统计实际使用的 UV 集数量
	int32 VisibleUVSetCount = 0;

	// 遍历每个 UV 集
	for (int32 UVSetIndex = 0; UVSetIndex < UVSetCount; UVSetIndex++)
	{
		// 获取 UV 集的顶点数据
		const FStaticMeshVertexBuffer& VertexBuffer = LODResource.VertexBuffers.StaticMeshVertexBuffer;

		bool IsEmpty = true;

		// 获取第一个顶点的 UV 值
		FVector2f PreviousUV = VertexBuffer.GetVertexUV(0, UVSetIndex);

		// 遍历每个顶点，检查 UV 集是否为空
		for (uint32 VertIndex = 1; VertIndex < VertexBuffer.GetNumVertices(); ++VertIndex)
		{
			FVector2f CurrentUV = VertexBuffer.GetVertexUV(VertIndex, UVSetIndex);

			// 计算当前 UV 与上一个 UV 的差值
			float DeltaU = FMath::Abs(CurrentUV.X - PreviousUV.X);
			float DeltaV = FMath::Abs(CurrentUV.Y - PreviousUV.Y);

			// 如果差值大于容差，则认为 UV 集非空
			if (DeltaU > Tolerance || DeltaV > Tolerance)
			{
				IsEmpty = false;
				break;
			}

			// 更新上一个 UV 值
			PreviousUV = CurrentUV;
		}

		// 如果 UV 集非空，则计数
		if (!IsEmpty)
		{
			VisibleUVSetCount++;
		}
	}

	// 返回可见 UV 集数量
	return VisibleUVSetCount;
}

int32 UAssetCheckToolBPLibrary::GetVisibleUVSetCountForSkeletalMesh(USkeletalMesh* SkeletalMesh, int32 LODIndex, float Tolerance)
{
    if (!SkeletalMesh || !SkeletalMesh->GetResourceForRendering())
    {
        UE_LOG(LogTemp, Warning, TEXT("SkeletalMesh is invalid or has no render data."));
        return -1; // 返回 -1 表示错误
    }

    // 获取骨骼网格的渲染数据
    const FSkeletalMeshRenderData* RenderData = SkeletalMesh->GetResourceForRendering();

    // 检查 LODIndex 是否有效
    if (LODIndex < 0 || LODIndex >= RenderData->LODRenderData.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid LODIndex: %d"), LODIndex);
        return -1; // 返回 -1 表示错误
    }

    // 获取指定 LOD 的资源
    const FSkeletalMeshLODRenderData& LODResource = RenderData->LODRenderData[LODIndex];

    // 获取当前 LOD 的 UV 集数量
    int32 UVSetCount = LODResource.GetNumTexCoords();

    // 统计实际使用的 UV 集数量
    int32 VisibleUVSetCount = 0;

    // 遍历每个 UV 集
    for (int32 UVSetIndex = 0; UVSetIndex < UVSetCount; UVSetIndex++)
    {
        // 获取 UV 集的顶点数据
        const FStaticMeshVertexBuffer& VertexBuffer = LODResource.StaticVertexBuffers.StaticMeshVertexBuffer;

        bool IsEmpty = true;

        // 获取第一个顶点的 UV 值
        FVector2f PreviousUV = VertexBuffer.GetVertexUV(0, UVSetIndex);

        // 遍历每个顶点，检查 UV 集是否为空
        for (uint32 VertIndex = 1; VertIndex < VertexBuffer.GetNumVertices(); ++VertIndex)
        {
            FVector2f CurrentUV = VertexBuffer.GetVertexUV(VertIndex, UVSetIndex);

            // 计算当前 UV 与上一个 UV 的差值
            float DeltaU = FMath::Abs(CurrentUV.X - PreviousUV.X);
            float DeltaV = FMath::Abs(CurrentUV.Y - PreviousUV.Y);

            // 如果差值大于容差，则认为 UV 集非空
            if (DeltaU > Tolerance || DeltaV > Tolerance)
            {
                IsEmpty = false;
                break;
            }

            // 更新上一个 UV 值
            PreviousUV = CurrentUV;
        }

        // 如果 UV 集非空，则计数
        if (!IsEmpty)
        {
            VisibleUVSetCount++;
        }
    }

    // 返回可见 UV 集数量
    return VisibleUVSetCount;
}






int32 UAssetCheckToolBPLibrary::ReplaceTexturesInMaterial(UMaterialInterface* Material, const TArray<FString>& PathsToReplace, UTexture* DefaultTexture)
{
	if (!Material || !DefaultTexture)
	{
		return 0;
	}

	// 材质实例处理
	UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(Material);
	if (MaterialInstance)
	{
		return 0;
	}

	// 基础材质处理
	UMaterial* BaseMaterial = Cast<UMaterial>(Material);
	if (!BaseMaterial)
	{
		return 0;
	}

	int32 ReplacedCount = 0;

#if WITH_EDITOR
	// 获取编辑器专用的材质
	UMaterial* MaterialForEditing = BaseMaterial;

	// 遍历所有纹理表达式
	for (TObjectIterator<UMaterialExpressionTextureSample> It; It; ++It)
	{
		UMaterialExpressionTextureSample* TextureSample = *It;

		// 确保表达式属于当前材质
		if (TextureSample->GetOuter() != MaterialForEditing)
		{
			continue;
		}

		if (!TextureSample->Texture)
		{
			continue;
		}

		FString TexturePath = TextureSample->Texture->GetPathName();

		// 检查贴图路径是否在需要替换的列表中
		for (const FString& PathToReplace : PathsToReplace)
		{
			if (TexturePath.Contains(PathToReplace))
			{
				TextureSample->Texture = DefaultTexture;
				ReplacedCount++;
				break;
			}
		}
	}

	if (ReplacedCount > 0)
	{
		BaseMaterial->PostEditChange();
	}
#endif

	return ReplacedCount;
}



void UAssetCheckToolBPLibrary::JumpToMaterialNode(UMaterialInterface* Material, const FString& NodeName)
{
#if WITH_EDITOR
	if (!GIsEditor || !Material || NodeName.IsEmpty())
	{
		return;
	}

	// 使用标准的资产编辑器管理器打开资产
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem)
	{
		return;
	}

	// 打开材质编辑器
	AssetEditorSubsystem->OpenEditorForAsset(Material);

	// 延迟执行搜索，确保编辑器完全加载
	FTimerHandle TimerHandle;
	GEditor->GetTimerManager()->SetTimer(TimerHandle, [Material, NodeName]()
	{
		// 获取活动的TAB
		TSharedPtr<SDockTab> ActiveTab = FGlobalTabmanager::Get()->GetActiveTab();
		if (!ActiveTab.IsValid())
		{
			return;
		}

		// 模拟按下Ctrl+F进行搜索
		FSlateApplication::Get().ProcessKeyDownEvent(
			FKeyEvent(
				EKeys::F, // Key being pressed
				FModifierKeysState(
					false, // bInIsLeftShiftDown
					false, // bInIsRightShiftDown
					true,  // bInIsLeftControlDown
					false, // bInIsRightControlDown
					false, // bInIsLeftAltDown
					false, // bInIsRightAltDown
					false, // bInIsLeftCommandDown
					false, // bInIsRightCommandDown
					false  // bInAreCapsLocked
				),
				0,      // InUserIndex
				false,  // bInIsRepeat
				0,      // InCharacterCode
				0       // InKeyCode
			)
		);

		// 短暂延迟后输入搜索文本
		FTimerHandle InputTimerHandle;
		GEditor->GetTimerManager()->SetTimer(InputTimerHandle, [NodeName]()
		{
			// 模拟输入搜索文本
			for (int32 i = 0; i < NodeName.Len(); ++i)
			{
				FSlateApplication::Get().ProcessKeyCharEvent(
					FCharacterEvent(NodeName[i],
								   FModifierKeysState(),
								   0,
								   false));
			}
		}, 0.1f, false);

	}, 0.5f, false);
#endif
}

bool UAssetCheckToolBPLibrary::OpenMaterialEditorAtNode(UMaterialInterface* Material, const FString& NodeName)
{
#if WITH_EDITOR
    if (!GIsEditor || !Material || NodeName.IsEmpty())
    {
        return false;
    }

    // 使用标准的资产编辑器管理器打开资产
    UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
    if (!AssetEditorSubsystem)
    {
        return false;
    }

    // 先关闭已打开的编辑器实例，确保重新打开
    AssetEditorSubsystem->CloseAllEditorsForAsset(Material);

    // 打开材质编辑器
    //IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->OpenEditorForAsset(Material);
	AssetEditorSubsystem->OpenEditorForAsset(Material);
	IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(Material,true);
    if (!EditorInstance)
    {
        return false;
    }

    // 延长等待时间，确保编辑器UI完全加载
    FTimerHandle TimerHandle;
    GEditor->GetTimerManager()->SetTimer(TimerHandle, [Material, NodeName]()
    {
        // 1. 尝试先获取焦点到编辑器窗口
        TSharedPtr<SWindow> ActiveWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
        if (ActiveWindow.IsValid())
        {
            FSlateApplication::Get().SetKeyboardFocus(ActiveWindow, EFocusCause::SetDirectly);
        }

        // 2. 按下ESC键清除可能的状态
        FSlateApplication::Get().ProcessKeyDownEvent(
            FKeyEvent(
                EKeys::Escape,
                FModifierKeysState(),
                0,
                false,
                0,
                0
            )
        );
        FSlateApplication::Get().ProcessKeyUpEvent(
            FKeyEvent(
                EKeys::Escape,
                FModifierKeysState(),
                0,
                false,
                0,
                0
            )
        );

        // 3. 短暂延迟后执行搜索
        FTimerHandle SearchTimerHandle;
        GEditor->GetTimerManager()->SetTimer(SearchTimerHandle, [NodeName]()
        {
            // 模拟按下Ctrl+F进行搜索
            FModifierKeysState CtrlDown(
                false, // bInIsLeftShiftDown
                false, // bInIsRightShiftDown
                true,  // bInIsLeftControlDown
                false, // bInIsRightControlDown
                false, // bInIsLeftAltDown
                false, // bInIsRightAltDown
                false, // bInIsLeftCommandDown
                false, // bInIsRightCommandDown
                false  // bInAreCapsLocked
            );

            FKeyEvent KeyDownEvent(
                EKeys::F,
                CtrlDown,
                0,
                false,
                0,
                0
            );
            FSlateApplication::Get().ProcessKeyDownEvent(KeyDownEvent);

            // 释放键盘
            FKeyEvent KeyUpEvent(
                EKeys::F,
                CtrlDown,
                0,
                false,
                0,
                0
            );
            FSlateApplication::Get().ProcessKeyUpEvent(KeyUpEvent);

            // 4. 等待搜索框出现并输入文本
            FTimerHandle InputTimerHandle;
            GEditor->GetTimerManager()->SetTimer(InputTimerHandle, [NodeName]()
            {
                // 尝试直接查找和设置搜索框文本
                TSharedPtr<SWidget> SearchBoxWidget = FSlateApplication::Get().GetUserFocusedWidget(0);
                if (SearchBoxWidget.IsValid())
                {
                    // 清除现有内容
                    for (int32 i = 0; i < 30; ++i)  // 假设最多有30个字符
                    {
                        FSlateApplication::Get().ProcessKeyDownEvent(
                            FKeyEvent(
                                EKeys::BackSpace,
                                FModifierKeysState(),
                                0,
                                false,
                                0,
                                0
                            )
                        );
                    }

                    // 输入搜索文本
                    for (int32 i = 0; i < NodeName.Len(); ++i)
                    {
                        FSlateApplication::Get().ProcessKeyCharEvent(
                            FCharacterEvent(
                                NodeName[i],
                                FModifierKeysState(),
                                0,
                                false
                            )
                        );
                    }

                    // 5. 按回车键确认搜索
                    FTimerHandle EnterTimerHandle;
                    GEditor->GetTimerManager()->SetTimer(EnterTimerHandle, []()
                    {
                        FSlateApplication::Get().ProcessKeyDownEvent(
                            FKeyEvent(
                                EKeys::Enter,
                                FModifierKeysState(),
                                0,
                                false,
                                0,
                                0
                            )
                        );
                        FSlateApplication::Get().ProcessKeyUpEvent(
                            FKeyEvent(
                                EKeys::Enter,
                                FModifierKeysState(),
                                0,
                                false,
                                0,
                                0
                            )
                        );
                    }, 0.2f, false);
                }
                else
                {
                    // 备选方法：直接输入搜索文本
                    for (int32 i = 0; i < NodeName.Len(); ++i)
                    {
                        FSlateApplication::Get().ProcessKeyCharEvent(
                            FCharacterEvent(
                                NodeName[i],
                                FModifierKeysState(),
                                0,
                                false
                            )
                        );
                    }

                    // 按回车键确认搜索
                    FTimerHandle EnterTimerHandle;
                    GEditor->GetTimerManager()->SetTimer(EnterTimerHandle, []()
                    {
                        FSlateApplication::Get().ProcessKeyDownEvent(
                            FKeyEvent(
                                EKeys::Enter,
                                FModifierKeysState(),
                                0,
                                false,
                                0,
                                0
                            )
                        );
                        FSlateApplication::Get().ProcessKeyUpEvent(
                            FKeyEvent(
                                EKeys::Enter,
                                FModifierKeysState(),
                                0,
                                false,
                                0,
                                0
                            )
                        );
                    }, 0.2f, false);
                }
            }, 0.3f, false);
        }, 0.2f, false);
    }, 1.0f, false);  // 延长初始等待时间

    return true;
#else
    return false;
#endif
}

void UAssetCheckToolBPLibrary::ExecuteSearchInMaterialEditor(const FString& SearchText)
{
    // ===== 以下是原来的搜索实现 =====
    // 模拟按下Ctrl+F进行搜索
    FModifierKeysState CtrlDown(
        false, // bInIsLeftShiftDown
        false, // bInIsRightShiftDown
        true,  // bInIsLeftControlDown
        false, // bInIsRightControlDown
        false, // bInIsLeftAltDown
        false, // bInIsRightAltDown
        false, // bInIsLeftCommandDown
        false, // bInIsRightCommandDown
        false  // bInAreCapsLocked
    );

    FKeyEvent KeyDownEvent(
        EKeys::F,
        CtrlDown,
        0,
        false,
        0,
        0
    );
    FSlateApplication::Get().ProcessKeyDownEvent(KeyDownEvent);

    // 释放键盘
    FKeyEvent KeyUpEvent(
        EKeys::F,
        CtrlDown,
        0,
        false,
        0,
        0
    );
    FSlateApplication::Get().ProcessKeyUpEvent(KeyUpEvent);

    // 短暂延迟后输入搜索文本
    FTimerHandle InputTimerHandle;
    GEditor->GetTimerManager()->SetTimer(InputTimerHandle, [SearchText]()
    {
        // 清除现有文本(按下Ctrl+A全选)
        FModifierKeysState CtrlDown(
            false, // bInIsLeftShiftDown
            false, // bInIsRightShiftDown
            true,  // bInIsLeftControlDown
            false, // bInIsRightControlDown
            false, // bInIsLeftAltDown
            false, // bInIsRightAltDown
            false, // bInIsLeftCommandDown
            false, // bInIsRightCommandDown
            false  // bInAreCapsLocked
        );

        FKeyEvent SelectAllKeyEvent(
            EKeys::A,
            CtrlDown,
            0,
            false,
            0,
            0
        );
        FSlateApplication::Get().ProcessKeyDownEvent(SelectAllKeyEvent);
        FSlateApplication::Get().ProcessKeyUpEvent(SelectAllKeyEvent);

        // 等待一下确保全选完成
        FTimerHandle DeleteAndInputTimerHandle;
        GEditor->GetTimerManager()->SetTimer(DeleteAndInputTimerHandle, [SearchText]()
        {
            // 按删除键清空文本
            FSlateApplication::Get().ProcessKeyDownEvent(
                FKeyEvent(
                    EKeys::Delete,
                    FModifierKeysState(),
                    0,
                    false,
                    0,
                    0
                )
            );

            // 输入新的搜索文本
            for (int32 i = 0; i < SearchText.Len(); ++i)
            {
                FSlateApplication::Get().ProcessKeyCharEvent(
                    FCharacterEvent(
                        SearchText[i],
                        FModifierKeysState(),
                        0,
                        false
                    )
                );
            }

            // 按回车键确认搜索
            FTimerHandle EnterTimerHandle;
            GEditor->GetTimerManager()->SetTimer(EnterTimerHandle, []()
            {
                FSlateApplication::Get().ProcessKeyDownEvent(
                    FKeyEvent(
                        EKeys::Enter,
                        FModifierKeysState(),
                        0,
                        false,
                        0,
                        0
                    )
                );
                FSlateApplication::Get().ProcessKeyUpEvent(
                    FKeyEvent(
                        EKeys::Enter,
                        FModifierKeysState(),
                        0,
                        false,
                        0,
                        0
                    )
                );
            }, 0.2f, false);
        }, 0.1f, false);
    }, 0.3f, false);
}


UFoliageType_InstancedStaticMesh* UAssetCheckToolBPLibrary::CreateFoliageTypeForStaticMesh(UStaticMesh* StaticMesh, FString& OutPackagePath)
{
	if (!StaticMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("StaticMesh is null!"));
		return nullptr;
	}

	// 获取 StaticMesh 的路径和名称
	FString StaticMeshPath = StaticMesh->GetPathName();
	FString StaticMeshName = StaticMesh->GetName();

	// 创建 FoliageType 的路径和名称
	FString FoliageTypePath = FPaths::GetPath(StaticMeshPath) / (StaticMeshName + TEXT("_foliageType"));
	FString FoliageTypeName = StaticMeshName + TEXT("_foliageType");

	// 创建 FoliageType
	UPackage* Package = CreatePackage(*FoliageTypePath);
	UFoliageType_InstancedStaticMesh* FoliageType = NewObject<UFoliageType_InstancedStaticMesh>(Package, *FoliageTypeName, RF_Public | RF_Standalone);

	if (!FoliageType)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create FoliageType!"));
		return nullptr;
	}

	// 设置 FoliageType 的 StaticMesh
	FoliageType->Mesh = StaticMesh;

	// 标记为脏以便保存
	FoliageType->MarkPackageDirty();

	// 保存 FoliageType
	FString PackageFileName = FPackageName::LongPackageNameToFilename(FoliageTypePath, FPackageName::GetAssetPackageExtension());

	// Assuming Package, FoliageType, and PackageFileName are already defined appropriately
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
	bool bSaved = UPackage::SavePackage(Package, FoliageType, *PackageFileName, SaveArgs);
	//bool bSaved = UPackage::SavePackage(Package, FoliageType, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName);
	if (!bSaved)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save FoliageType!"));
		return nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("FoliageType created and saved successfully: %s"), *FoliageTypePath);

	// 将生成的 FoliageType 的路径作为输出
	OutPackagePath = FoliageTypePath;

	return FoliageType;
}





void UAssetCheckToolBPLibrary::SetMobility(UFoliageType* FoliageType,EComponentMobility::Type Mobility)
{
	if (FoliageType )
	{
		FoliageType->Mobility = Mobility;
	}

}

void UAssetCheckToolBPLibrary::SetLightmapType(UFoliageType* FoliageType, ELightmapType LightmapType)
{
	if (FoliageType )
	{
		FoliageType->LightmapType = LightmapType;
	}
}

void UAssetCheckToolBPLibrary::SetComponnentLightmapType(UPrimitiveComponent* Component, ELightmapType LightmapType)
{
	if (!IsValid(Component))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid component provided."));
		return;
	}

	// 处理 UInstancedStaticMeshComponent
	if (UInstancedStaticMeshComponent* ISMComponent = Cast<UInstancedStaticMeshComponent>(Component))
	{
		if (ISMComponent)
		{
			ISMComponent->SetLightmapType(LightmapType);
			ISMComponent->MarkRenderStateDirty(); // 标记组件需要更新渲染状态
			UE_LOG(LogTemp, Log, TEXT("Set LightmapType for InstancedStaticMeshComponent '%s'."), *ISMComponent->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("InstancedStaticMeshComponent '%s' has no valid StaticMesh."), *ISMComponent->GetName());
		}
	}
	// 处理 UStaticMeshComponent
	else if (UStaticMeshComponent* SMComponent = Cast<UStaticMeshComponent>(Component))
	{
		if (SMComponent)
		{
			SMComponent->SetLightmapType(LightmapType);
			SMComponent->MarkRenderStateDirty(); // 标记组件需要更新渲染状态
			UE_LOG(LogTemp, Log, TEXT("Set LightmapType for StaticMeshComponent '%s'."), *SMComponent->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("StaticMeshComponent '%s' has no valid StaticMesh."), *SMComponent->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Unsupported component type provided."));
	}
}

void  UAssetCheckToolBPLibrary::SetLightmapTypetest(UFoliageType_InstancedStaticMesh* FoliageTypeInput)
{
	if (FoliageTypeInput)
	{
		FoliageTypeInput->LightmapType = ELightmapType::Default;

	}
}

void UAssetCheckToolBPLibrary::ReParentMaterialInstances(const TArray<UMaterialInstanceConstant*>& MaterialInstances, UMaterialInterface* NewParentMaterial)
{
	if (!NewParentMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("NewParentMaterial is null!"));
		return;
	}

	FText Title = FText::FromString(TEXT("ReParent工具"));
	FText Message = FText::Format(
		FText::FromString(TEXT("将会把所有选择的子材质的Parent设置为\n{0}\n要继续吗?")),
		FText::FromName(NewParentMaterial->GetFName())
	);
	EAppMsgType::Type Type = EAppMsgType::YesNo;
	EAppReturnType::Type Response = FMessageDialog::Open(Type, Message, Title); // 正确传值

	if (Response == EAppReturnType::Yes)
	{
		TArray<UPackage*> PackagesToSave;

		for (UMaterialInstanceConstant* Instance : MaterialInstances)
		{
			if (Instance)
			{
				//FMaterialEditorUtilities::MaterialReParent(Instance, NewParentMaterial);
				Instance->MarkPackageDirty();
				PackagesToSave.Add(Instance->GetOutermost());
			}
		}

		FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true, false);
	}
}


bool UAssetCheckToolBPLibrary::TransferMaterialParameters(
    FString SourcePath,
    FString TargetPath,
    const TArray<FString>& ExcludeKeywords) // 新增的排除关键字参数
{
    TObjectPtr<UMaterialInstanceConstant> SourceMatInst = LoadMatInstByPath(SourcePath);
    TObjectPtr<UMaterialInstanceConstant> TargetMatInst = LoadMatInstByPath(TargetPath);

    if (!SourceMatInst || !TargetMatInst)
    {
        return false; // 如果源或目标材质实例加载失败，直接返回 false
    }

    bool bAnyParameterTransferred = false; // 用于标记是否有参数被传递

    // 检查属性名是否包含排除关键字
    auto ShouldExcludeParameter = [&ExcludeKeywords](const FName& ParamName) -> bool
    {
        FString ParamNameStr = ParamName.ToString();
        for (const FString& Keyword : ExcludeKeywords)
        {
            if (ParamNameStr.Contains(Keyword))
            {
                return true; // 如果属性名包含排除关键字，返回 true
            }
        }
        return false; // 否则返回 false
    };

    /* StaticSwitch */
    TArray<FMaterialParameterInfo> TargetMatSwitchParamInfos;
    TArray<FGuid> TargetSwitchGuid;
    TargetMatInst->GetAllStaticSwitchParameterInfo(TargetMatSwitchParamInfos, TargetSwitchGuid);
    TArray<FMaterialParameterInfo> SourceMatSwitchParamInfos;
    TArray<FGuid> SourceSwitchGuid;
    SourceMatInst->GetAllStaticSwitchParameterInfo(SourceMatSwitchParamInfos, SourceSwitchGuid);

    for (FMaterialParameterInfo S_Element : SourceMatSwitchParamInfos)
    {
        FName S_ParamName = S_Element.Name;
        if (ShouldExcludeParameter(S_ParamName)) // 检查是否排除
        {
            continue;
        }

        bool S_ParmValue = false;
        FGuid S_Guid = FGuid();
        SourceMatInst->GetStaticSwitchParameterValue(FHashedMaterialParameterInfo(S_ParamName), S_ParmValue, S_Guid);

        for (FMaterialParameterInfo T_Element : TargetMatSwitchParamInfos)
        {
            FName T_ParamName = T_Element.Name;
            if (ShouldExcludeParameter(T_ParamName)) // 检查是否排除
            {
                continue;
            }

            bool T_ParmValue = false;
            FGuid T_Guid = FGuid();
            TargetMatInst->GetStaticSwitchParameterValue(FHashedMaterialParameterInfo(T_ParamName), T_ParmValue, T_Guid);

            if (S_ParamName == T_ParamName && S_ParmValue != T_ParmValue)
            {
                TargetMatInst->SetStaticSwitchParameterValueEditorOnly(T_Element, S_ParmValue);
                bAnyParameterTransferred = true; // 标记有参数被传递
            }
        }
    }

    /* Scalar */
    TArray<FMaterialParameterInfo> TargetMatScalarParamInfos;
    TArray<FGuid> TargetScalarGuid;
    TargetMatInst->GetAllScalarParameterInfo(TargetMatScalarParamInfos, TargetScalarGuid);
    TArray<FMaterialParameterInfo> SourceMatScalarParamInfos;
    TArray<FGuid> SourceScalarGuid;
    SourceMatInst->GetAllScalarParameterInfo(SourceMatScalarParamInfos, SourceScalarGuid);

    for (FMaterialParameterInfo S_Element : SourceMatScalarParamInfos)
    {
        FName S_ParamName = S_Element.Name;
        if (ShouldExcludeParameter(S_ParamName)) // 检查是否排除
        {
            continue;
        }

        float S_ParmValue = 0;
        SourceMatInst->GetScalarParameterValue(FHashedMaterialParameterInfo(S_ParamName), S_ParmValue);

        for (FMaterialParameterInfo T_Element : TargetMatScalarParamInfos)
        {
            FName T_ParamName = T_Element.Name;
            if (ShouldExcludeParameter(T_ParamName)) // 检查是否排除
            {
                continue;
            }

            float T_ParmValue = 0;
            TargetMatInst->GetScalarParameterValue(FHashedMaterialParameterInfo(T_ParamName), T_ParmValue);

            if (S_ParamName == T_ParamName && S_ParmValue != T_ParmValue)
            {
                TargetMatInst->SetScalarParameterValueEditorOnly(T_Element, S_ParmValue);
                bAnyParameterTransferred = true; // 标记有参数被传递
            }
        }
    }

    /* Vector */
    TArray<FMaterialParameterInfo> TargetMatVectorParamInfos;
    TArray<FGuid> TargetVectorGuid;
    TargetMatInst->GetAllVectorParameterInfo(TargetMatVectorParamInfos, TargetVectorGuid);
    TArray<FMaterialParameterInfo> SourceMatVectorParamInfos;
    TArray<FGuid> SourceVectorGuid;
    SourceMatInst->GetAllVectorParameterInfo(SourceMatVectorParamInfos, SourceVectorGuid);

    for (FMaterialParameterInfo S_Element : SourceMatVectorParamInfos)
    {
        FName S_ParamName = S_Element.Name;
        if (ShouldExcludeParameter(S_ParamName)) // 检查是否排除
        {
            continue;
        }

        FLinearColor S_ParmValue = FLinearColor(0, 0, 0);
        SourceMatInst->GetVectorParameterValue(FHashedMaterialParameterInfo(S_ParamName), S_ParmValue);

        for (FMaterialParameterInfo T_Element : TargetMatVectorParamInfos)
        {
            FName T_ParamName = T_Element.Name;
            if (ShouldExcludeParameter(T_ParamName)) // 检查是否排除
            {
                continue;
            }

            FLinearColor T_ParmValue = FLinearColor(0, 0, 0);
            TargetMatInst->GetVectorParameterValue(FHashedMaterialParameterInfo(T_ParamName), T_ParmValue);

            if (S_ParamName == T_ParamName && S_ParmValue != T_ParmValue)
            {
                TargetMatInst->SetVectorParameterValueEditorOnly(T_Element, S_ParmValue);
                bAnyParameterTransferred = true; // 标记有参数被传递
            }
        }
    }

    /* Texture */
    TArray<FMaterialParameterInfo> TargetMatTextureParamInfos;
    TArray<FGuid> TargetTextureGuid;
    TargetMatInst->GetAllTextureParameterInfo(TargetMatTextureParamInfos, TargetTextureGuid);
    TArray<FMaterialParameterInfo> SourceMatTextureParamInfos;
    TArray<FGuid> SourceTextureGuid;
    SourceMatInst->GetAllTextureParameterInfo(SourceMatTextureParamInfos, SourceTextureGuid);

    for (FMaterialParameterInfo S_Element : SourceMatTextureParamInfos)
    {
        FName S_ParamName = S_Element.Name;
        if (ShouldExcludeParameter(S_ParamName)) // 检查是否排除
        {
            continue;
        }

        UTexture* S_ParmValue = nullptr;
        SourceMatInst->GetTextureParameterValue(FHashedMaterialParameterInfo(S_ParamName), S_ParmValue);

        for (FMaterialParameterInfo T_Element : TargetMatTextureParamInfos)
        {
            FName T_ParamName = T_Element.Name;
            if (ShouldExcludeParameter(T_ParamName)) // 检查是否排除
            {
                continue;
            }

            UTexture* T_ParmValue = nullptr;
            TargetMatInst->GetTextureParameterValue(FHashedMaterialParameterInfo(T_ParamName), T_ParmValue);

            if (S_ParamName == T_ParamName && S_ParmValue != T_ParmValue)
            {
                TargetMatInst->SetTextureParameterValueEditorOnly(T_Element, S_ParmValue);
                bAnyParameterTransferred = true; // 标记有参数被传递
            }
        }
    }

    TargetMatInst->PostEditChange();

    return bAnyParameterTransferred; // 返回是否有参数被传递
}

UMaterialInstanceConstant* UAssetCheckToolBPLibrary::LoadMatInstByPath(const FString& Path)
{
	// 创建 FSoftObjectPath 对象
	FSoftObjectPath SoftObjectPath(Path);

	// 使用 LoadObject 加载材质实例
	UMaterialInstanceConstant* LoadedMatInst = Cast<UMaterialInstanceConstant>(SoftObjectPath.TryLoad());

	if (LoadedMatInst)
	{
		return LoadedMatInst;
	}
	else
	{
		return nullptr;
	}
}


UClass* UAssetCheckToolBPLibrary::GetAssetClassByPath(const FString& AssetPath)
{
	if (AssetPath.IsEmpty())
	{
		return nullptr; // 如果 AssetPath 为空，直接返回 nullptr
	}

	// 创建 FSoftObjectPath 对象
	FSoftObjectPath SoftObjectPath(AssetPath);

	// 使用 TryLoad 加载资产
	UObject* LoadedAsset = SoftObjectPath.TryLoad();

	if (LoadedAsset)
	{
		// 返回资产的 UClass
		return LoadedAsset->GetClass();
	}

	return nullptr; // 如果加载失败，返回 nullptr
}

AInstancedFoliageActor* UAssetCheckToolBPLibrary::GetPartitionFoliageActor(AActor* StaticMeshActor)
{
	if (!StaticMeshActor)
	{
		return nullptr;
	}
	UWorld* World = StaticMeshActor->GetWorld();
	if(!World)
	{
		DebugUtil::MessageDialog(TEXT("Can not Get World"));
		return nullptr;
	}

	ULevel* Level =  StaticMeshActor->GetLevel();
	if(!Level)
	{
		DebugUtil::MessageDialog(TEXT("Can not Get Level"));
		return nullptr;
	}

	AInstancedFoliageActor* IFA = AInstancedFoliageActor::Get(World,true,Level,StaticMeshActor->GetActorLocation());
	if(!IFA)
	{
		DebugUtil::MessageDialog(TEXT("Can not Get or Create InstancedFoliageActor"));
		return nullptr;
	}

	return IFA;
}



AInstancedFoliageActor* UAssetCheckToolBPLibrary::GetLevelFoliageActor(FString LevelName)
{
	// 获取编辑器世界
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!IsValid(World))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get the editor world."));
		return nullptr;
	}

	// 将 LevelName 转换为 FName
	FName LevelFName = *LevelName;

	// 获取对应的 LevelStreaming
	ULevelStreaming* LevelStreaming = World->GetLevelStreamingForPackageName(LevelFName);
	if (!LevelStreaming || !LevelStreaming->IsLevelLoaded())
	{
		UE_LOG(LogTemp, Error, TEXT("Level '%s' is not loaded or does not exist."), *LevelName);
		return nullptr;
	}

	// 获取加载的子关卡
	ULevel* ActorLevel = LevelStreaming->GetLoadedLevel();
	if (!IsValid(ActorLevel))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get the loaded level for '%s'."), *LevelName);
		return nullptr;
	}

	// 查找或创建对应的 AInstancedFoliageActor
	AInstancedFoliageActor* FoliageActor = AInstancedFoliageActor::GetInstancedFoliageActorForLevel(ActorLevel);
	if (!IsValid(FoliageActor))
	{
		// 如果没有找到，创建一个新的 AInstancedFoliageActor
		FActorSpawnParameters SpawnParams;
		SpawnParams.OverrideLevel = ActorLevel; // 将新 Actor 附加到目标关卡
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FoliageActor = World->SpawnActor<AInstancedFoliageActor>(SpawnParams);
		if (IsValid(FoliageActor))
		{
			UE_LOG(LogTemp, Log, TEXT("Created a new foliage actor for level '%s'."), *LevelName);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create a foliage actor for level '%s'."), *LevelName);
			return nullptr;
		}
	}

	return FoliageActor;
}


AActor* UAssetCheckToolBPLibrary::GetActorByActorFullName(const FString& InName)
{
	if (InName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Component name is empty."));
		return nullptr;
	}
	//AActor* ActorFound = FindObject<AActor>(ANY_PACKAGE, *InName);
	AActor* ActorFound = FindObject<AActor>(nullptr, *InName);
	return ActorFound;
}

UActorComponent* UAssetCheckToolBPLibrary::GetComponentByComponentFullName(const FString& InName)
{
	if (InName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Component name is empty."));
		return nullptr;
	}

	// 查找组件
	UActorComponent* ComponentFound = FindObject<UActorComponent>(nullptr, *InName);

	return ComponentFound;


}


int32 UAssetCheckToolBPLibrary::FindMaterialWorldPositionExpression(const FString& MaterialPath, FString& OutDetails)
{
        // 创建 FSoftObjectPath 对象
        FSoftObjectPath SoftObjectPath(MaterialPath);
		OutDetails = TEXT("");

        // 使用 LoadObject 加载
        UMaterial* LoadedMaterial = Cast<UMaterial>(SoftObjectPath.TryLoad());

        if (!LoadedMaterial)
        {
                DebugUtil::MessageDialog(MaterialPath+TEXT("is not a material"));
                return 0;
        }

        FMaterialExpressionCollection& MaterialExpressionCollection = LoadedMaterial->GetExpressionCollection();
        TArray<TObjectPtr<UMaterialExpression>> MaterialExpressions = MaterialExpressionCollection.Expressions;
        TArray<TObjectPtr<UMaterialExpressionWorldPosition>> WorldPositionArray;

        //查找Material的节点，添加到数组中
        for(TObjectPtr<UMaterialExpression> Expression : MaterialExpressions)
        {
                TObjectPtr<UMaterialExpressionWorldPosition> WorldPositionExpression = Cast<UMaterialExpressionWorldPosition>(Expression);
                if(!WorldPositionExpression) continue;
				if(WorldPositionExpression->WorldPositionShaderOffset>1)	continue;
                WorldPositionArray.AddUnique(WorldPositionExpression);
        }
		if(WorldPositionArray.Num() > 0)
		{
			OutDetails += FString::Printf(TEXT("%s : %d;  "), *LoadedMaterial->GetName(), WorldPositionArray.Num());
		}
        //查找Material的Function的节点，添加到数组中
        TArray<UMaterialFunctionInterface*> DependentFunctions;
        LoadedMaterial->GetDependentFunctions(DependentFunctions);
        for(UMaterialFunctionInterface* DependentFunction : DependentFunctions)
        {
                TConstArrayView<TObjectPtr<UMaterialExpression>> Expressions = DependentFunction->GetExpressions();
                int32 Num = 0;
                for(TObjectPtr<UMaterialExpression> Expression : Expressions)
                {
                        TObjectPtr<UMaterialExpressionWorldPosition> TextureBaseExpression = Cast<UMaterialExpressionWorldPosition>(Expression);
                        if(!TextureBaseExpression) continue;
                		if(TextureBaseExpression->WorldPositionShaderOffset>1) continue;
                        WorldPositionArray.AddUnique(TextureBaseExpression);
                        Num++;
                }
				if(Num == 0) continue;
                OutDetails += FString::Printf(TEXT("%s : %d;  "), *DependentFunction->GetName(), Num);
        }

        return WorldPositionArray.Num();
}



TArray<FString> UAssetCheckToolBPLibrary::RecursiveGetAssetsDependencies(const TArray<FString>& PackageNames,FString& OutPackagePath)
{

	TSet<FName> AllDependencies;
	TSet<FString> OutExternalObjectsPaths;
	TSet<FName> ExcludedDependencies;
	OutPackagePath.Empty();

	auto ShouldExcludeFromDependenciesSearch = [](const FName& DependencyName) -> bool
	{
		// 示例：跳过引擎脚本包（/Script/...）
		return DependencyName.ToString().StartsWith(TEXT("/Script/"));
	};

	for(const FString& PackageName : PackageNames)
	{
		AllDependencies.Add(FName(PackageName));
	}

	for(const FString& PackageName : PackageNames)
	{
		UAssetCheckToolBPLibrary::RecursiveGetDependencies(
		FName(PackageName),
		AllDependencies,
		OutExternalObjectsPaths,
		ExcludedDependencies,
		ShouldExcludeFromDependenciesSearch
		);
	}

	TArray<FString> Result;
	for (const FName& Dependency : AllDependencies)
	{
		Result.Add(Dependency.ToString());
		OutPackagePath += Dependency.ToString() + "\n";
	}

	return Result;
}


void UAssetCheckToolBPLibrary::RecursiveGetDependencies(const FName& PackageName, TSet<FName>& AllDependencies,TSet<FString>& OutExternalObjectsPaths, TSet<FName>& ExcludedDependencies,const TFunction<bool(FName)>& ShouldExcludeFromDependenciesSearch)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FName> Dependencies;
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	AssetRegistry.GetDependencies(PackageName, Dependencies);

	for (TArray<FName>::TConstIterator DependsIt = Dependencies.CreateConstIterator(); DependsIt; ++DependsIt)
	{
		FString DependencyName = (*DependsIt).ToString();

		const bool bIsScriptPackage = DependencyName.StartsWith(TEXT("/Script"));

		// The asset registry can give some reference to some deleted assets. We don't want to migrate these.
		const bool bAssetExist = AssetRegistry.GetAssetPackageDataCopy(*DependsIt).IsSet();

		if (!bIsScriptPackage && bAssetExist)
		{
			uint32 DependsHash = GetTypeHash(*DependsIt);
			if (!AllDependencies.ContainsByHash(DependsHash, *DependsIt) && !ExcludedDependencies.ContainsByHash(DependsHash, *DependsIt))
			{
				// Early stop the dependency search
				if (ShouldExcludeFromDependenciesSearch(*DependsIt))
				{
					ExcludedDependencies.AddByHash(DependsHash, *DependsIt);
					continue;
				}

				AllDependencies.AddByHash(DependsHash, *DependsIt);

				RecursiveGetDependencies(*DependsIt, AllDependencies, OutExternalObjectsPaths, ExcludedDependencies, ShouldExcludeFromDependenciesSearch);
			}
		}
	}

	// Handle Specific External Objects use case (only used for the Migrate path for now)
	// todo: revisit how to handle those in a more generic way. Should the FExternalActorAssetDependencyGatherer handle the external objects reference also?
	TArray<FAssetData> Assets;

	// The migration only work on the saved version of the assets so no need to scan the for the in memory only assets. This also greatly improve the performance of the migration when a lot assets are loaded in the editor.
	const bool bOnlyIncludeOnDiskAssets = true;
	if (AssetRegistryModule.Get().GetAssetsByPackageName(PackageName, Assets, bOnlyIncludeOnDiskAssets))
	{
		for (const FAssetData& AssetData : Assets)
		{
			if (AssetData.GetClass() && AssetData.GetClass()->IsChildOf<UWorld>())
			{
				TArray<FString> ExternalObjectsPaths = ULevel::GetExternalObjectsPaths(PackageName.ToString());
				for (const FString& ExternalObjectsPath : ExternalObjectsPaths)
				{
					if (!ExternalObjectsPath.IsEmpty() && !OutExternalObjectsPaths.Contains(ExternalObjectsPath))
					{
						OutExternalObjectsPaths.Add(ExternalObjectsPath);
						AssetRegistryModule.Get().ScanPathsSynchronous({ ExternalObjectsPath }, /*bForceRescan*/true, /*bIgnoreDenyListScanFilters*/true);

						TArray<FAssetData> ExternalObjectAssets;
						AssetRegistryModule.Get().GetAssetsByPath(FName(*ExternalObjectsPath), ExternalObjectAssets, /*bRecursive*/true, bOnlyIncludeOnDiskAssets);

						for (const FAssetData& ExternalObjectAsset : ExternalObjectAssets)
						{
							// We don't expose the early dependency search exit to the external objects/actors since to the users their are same the outer package that own these objects
							AllDependencies.Add(ExternalObjectAsset.PackageName);
							RecursiveGetDependencies(ExternalObjectAsset.PackageName, AllDependencies, OutExternalObjectsPaths, ExcludedDependencies, ShouldExcludeFromDependenciesSearch);
						}
					}
				}
			}
		}
	}
}


TArray<FString> UAssetCheckToolBPLibrary::RecursiveGetAssetDependencies(const FString& PackageName)
{
	TSet<FName> AllDependencies;
	TSet<FString> OutExternalObjectsPaths;
	TSet<FName> ExcludedDependencies;

	auto ShouldExcludeFromDependenciesSearch = [](const FName& DependencyName) -> bool
	{
		// 示例：跳过引擎脚本包（/Script/...）
		return DependencyName.ToString().StartsWith(TEXT("/Script/"));
	};

	AllDependencies.Add(FName(PackageName));

	RecursiveGetDependencies(
			FName(PackageName),
			AllDependencies,
			OutExternalObjectsPaths,
			ExcludedDependencies,
			ShouldExcludeFromDependenciesSearch
			);

	TArray<FString> Result;
	for (const FName& Dependency : AllDependencies)
	{
		Result.Add(Dependency.ToString());
	}

	return Result;
}

// 获取对象的直接父类
UClass* UAssetCheckToolBPLibrary::GetParentClass(UObject* Object)
{
	if (!IsValid(Object))
	{
		return nullptr; // 安全校验
	}

	UClass* Class = Object->GetClass(); // 获取对象类型
	return Class ? Class->GetSuperClass() : nullptr; // 返回父类
}

// 获取完整的继承链（可选扩展）
TArray<UClass*> UAssetCheckToolBPLibrary::GetParentClassChain(UObject* Object)
{
	TArray<UClass*> ClassChain;

	if (IsValid(Object))
	{
		for (UClass* Class = Object->GetClass(); Class; Class = Class->GetSuperClass())
		{
			ClassChain.Add(Class);
		}
	}

	return ClassChain;
}

void UAssetCheckToolBPLibrary::CancelMaterialInstanceOverride(UMaterialInstanceConstant* MaterialInst,const FString& ParameterName)
{
        if(!MaterialInst) return;

        for(FScalarParameterValue ScalarParameterValue : MaterialInst->ScalarParameterValues)
        {
                if(ParameterName==ScalarParameterValue.ParameterInfo.Name.ToString())
                {
                        MaterialInst->ScalarParameterValues.Remove(ScalarParameterValue);
                }
        }

        for(FVectorParameterValue VectorParameterValue : MaterialInst->VectorParameterValues)
        {
                if(ParameterName==VectorParameterValue.ParameterInfo.Name.ToString())
                {
                        MaterialInst->VectorParameterValues.Remove(VectorParameterValue);
                }
        }

        for(FTextureParameterValue TextureParameterValue : MaterialInst->TextureParameterValues)
        {
                if(ParameterName==TextureParameterValue.ParameterInfo.Name.ToString())
                {
                        MaterialInst->TextureParameterValues.Remove(TextureParameterValue);
                }
        }

        MaterialInst->PostEditChange();
}

TArray<FString> UAssetCheckToolBPLibrary::SplitStringByDelimiter(const FString& FileContent, const FString& Delimiter )
{
	TArray<FString> PathArray;
	FileContent.ParseIntoArray(PathArray, *Delimiter, true); // 使用传入的分隔符
	// 可选：若分隔符是换行符，移除行末的\r（兼容Windows换行符\r\n）
	if (Delimiter.Equals(TEXT("\n"))) {
		for (FString& Line : PathArray) {
			Line.TrimEndInline();
		}
	}
	return PathArray;
}

TArray<FString> UAssetCheckToolBPLibrary::RecursiveGetAssetDependenciesWithClass(
    const FString& PackageName,
    const TArray<UClass*>& FilterClasses,
    const TArray<FString>& ExcludeKeywords)  // 新增排除关键词参数[1](@ref)[9](@ref)
{
    TSet<FName> AllDependencies;
    TSet<FString> OutExternalObjectsPaths;
    TSet<FName> ExcludedDependencies;

    // 优化：将排除关键词转为TSet提高查询效率[1](@ref)
    TSet<FString> ExcludedKeywordsSet(ExcludeKeywords);

    // 修改排除逻辑：增加关键词检查[1](@ref)[9](@ref)
    auto ShouldExcludeFromDependenciesSearch = [&ExcludedKeywordsSet](const FName& DependencyName) -> bool
    {
        FString DependencyStr = DependencyName.ToString();

        // 1. 默认排除引擎脚本
        if (DependencyStr.StartsWith(TEXT("/Script/")))
            return true;

        // 2. 检查排除关键词[1](@ref)
        for (const FString& Keyword : ExcludedKeywordsSet)
        {
            if (DependencyStr.Contains(Keyword))
                return true;
        }
        return false;
    };

    AllDependencies.Add(FName(PackageName));
    RecursiveGetDependencies(
        FName(PackageName),
        AllDependencies,
        OutExternalObjectsPaths,
        ExcludedDependencies,
        ShouldExcludeFromDependenciesSearch  // 使用新的排除逻辑
    );

    TArray<FString> Result;
    for (const FName& Dependency : AllDependencies)
    {
        FString DependencyStr = Dependency.ToString();

        // 最终过滤：确保未被排除[1](@ref)
        if (ShouldExcludeFromDependenciesSearch(Dependency))
            continue;

        // 类过滤逻辑保持不变
        if (FilterClasses.IsEmpty())
        {
            Result.Add(DependencyStr);
            continue;
        }

        if (UObject* Object = FSoftObjectPath(DependencyStr).TryLoad())
        {
            for (UClass* FilterClass : FilterClasses)
            {
                if (Object->IsA(FilterClass))
                {
                    Result.Add(DependencyStr);
                    break;
                }
            }
        }
    }
    return Result;
}




void UAssetCheckToolBPLibrary::InsertSkeletalMeshLODs(USkeletalMesh* SkeletalMesh, USkeletalMesh* LOD0)
{
	FSkeletalMeshModel* ImportedModel = SkeletalMesh->GetImportedModel();
	int32 LODNum = ImportedModel->LODModels.Num();

	DebugUtil::Print(FString::FromInt(LODNum));

	for(int i = LODNum; i>0 ;i--)
	{
		SetCustomLOD(SkeletalMesh,SkeletalMesh,i,i-1,TEXT(""),true);
	}
	LODNum = ImportedModel->LODModels.Num();

	SetCustomLOD(SkeletalMesh,LOD0,LODNum,0,TEXT(""),false);
	LODNum = ImportedModel->LODModels.Num();
	DebugUtil::Print(FString::FromInt(LODNum));

	// 获取最后一个LOD模型的索引
	int32 LastIndex = LODNum - 1;

	SetCustomLOD(SkeletalMesh,SkeletalMesh,0,LastIndex,TEXT(""),true);

	LODNum = ImportedModel->LODModels.Num();
	LastIndex = LODNum - 1;

	//移除最后一级LOD
	FSkeletalMeshUpdateContext UpdateContext;
	UpdateContext.SkeletalMesh = SkeletalMesh;

	FLODUtilities::RemoveLOD(UpdateContext, LastIndex);

	/*if (SkeletalMesh->GetLODSettings())
	{
		SkeletalMesh->GetLODSettings()->SetLODSettingsToMesh(SkeletalMesh);
	}*/


	auto GetMaterialIndexLambda = [](USkeletalMesh* SkeletalMesh, int32 LODIndex, int32 SectionIndex) -> int32
	{
		const FSkeletalMeshLODInfo* LODInfoPtr = SkeletalMesh->GetLODInfo(LODIndex);
		if (LODInfoPtr && LODInfoPtr->LODMaterialMap.IsValidIndex(SectionIndex) && LODInfoPtr->LODMaterialMap[SectionIndex] != INDEX_NONE)
		{
			return LODInfoPtr->LODMaterialMap[SectionIndex];
		}
		return SkeletalMesh->GetImportedModel()->LODModels[LODIndex].Sections[SectionIndex].MaterialIndex;
	};
	TMap<FString, UMaterialInterface*> LOD0Map;

	const FSkeletalMeshLODModel& LODModel = LOD0->GetImportedModel()->LODModels[0];

	for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); SectionIndex++)
	{
		const FSkelMeshSection& Section = LODModel.Sections[SectionIndex];
		int32 VertexCount = Section.GetNumVertices();

		FString String = FString::FromInt(VertexCount) + Section.SoftVertices[1].Position.ToString();

		int32 MaterialIndex = GetMaterialIndexLambda(LOD0,0,SectionIndex);
		UMaterialInterface* MaterialInterface = LOD0->GetMaterials()[MaterialIndex].MaterialInterface;


		LOD0Map.Add(String, MaterialInterface);
	}

	TArray<FSkelMeshSection> SKMSections = ImportedModel->LODModels[0].Sections;
	TArray<int32>& SKMMaterialMap = SkeletalMesh->GetLODInfo(0)->LODMaterialMap;

	for (int i = 0;i < SKMSections.Num(); i++)
	{
		int32 VertNum = SKMSections[i].GetNumVertices();
		FString String = FString::FromInt(VertNum) + SKMSections[i].SoftVertices[1].Position.ToString();

		UMaterialInterface* Material = LOD0Map.FindRef(String);

		FName SlotName = Material->GetFName();

		FSkeletalMaterial SkeletalMaterial = FSkeletalMaterial(Material,SlotName);
		SkeletalMesh->GetMaterials().Emplace(SkeletalMaterial);

		if(SKMMaterialMap.IsValidIndex(i))
		{
			SKMMaterialMap[i] = SkeletalMesh->GetMaterials().Num() - 1;
		}
		else
		{
			SKMMaterialMap.SetNum(i + 1);
			SKMMaterialMap[i] = SkeletalMesh->GetMaterials().Num() - 1;
		}
	}

	//删除空的材质插槽
	TArray<FSkeletalMaterial>& Materials = SkeletalMesh->GetMaterials();
	for (int32 i = Materials.Num() - 1; i >= 0; --i)
	{
		if (Materials[i].MaterialInterface == nullptr) // 检查是否为空
		{
			USkeletalMesh* SkeletalMeshPtr = SkeletalMesh;
			SkeletalMeshPtr->GetMaterials().RemoveAt(i);
			FSkeletalMeshModel* Model = SkeletalMeshPtr->GetImportedModel();



			//When we delete a material slot we need to fix all MaterialIndex after the deleted index
			TArray<int32>& LODMaterialMap = SkeletalMeshPtr->GetLODInfo(0)->LODMaterialMap;
			for (int32 SectionIndex = 0; SectionIndex < Model->LODModels[0].Sections.Num(); ++SectionIndex)
			{
				int32 SectionMaterialIndex = Model->LODModels[0].Sections[SectionIndex].MaterialIndex;
				if (LODMaterialMap.IsValidIndex(SectionIndex) && LODMaterialMap[SectionIndex] != INDEX_NONE)
				{
					SectionMaterialIndex = LODMaterialMap[SectionIndex];
				}
				if (SectionMaterialIndex > i)
				{
					SectionMaterialIndex--;
				}
				if (SectionMaterialIndex != Model->LODModels[0].Sections[SectionIndex].MaterialIndex)
				{
					while(!LODMaterialMap.IsValidIndex(SectionIndex))
					{
						LODMaterialMap.Add(INDEX_NONE);
					}
					LODMaterialMap[SectionIndex] = SectionMaterialIndex;
				}
			}
		}

	}

	SkeletalMesh->PostEditChange();
}


bool UAssetCheckToolBPLibrary::SetCustomLOD(USkeletalMesh* DestinationSkeletalMesh, USkeletalMesh* SourceSkeletalMesh, const int32 LodIndex, const int32 SrcLodIndex,const FString& SourceDataFilename,const bool Save)
{
if(!DestinationSkeletalMesh || !SourceSkeletalMesh)
	{
		return false;
	}

	FScopedSkeletalMeshPostEditChange ScopePostEditChange(DestinationSkeletalMesh);
	//Lock the skeletal mesh
	struct FScopedLockProperties
	{
	public:
		FScopedLockProperties(USkeletalMesh* SkeletalMesh)
		{
			check(SkeletalMesh);
			Lock = SkeletalMesh->LockPropertiesUntil();
		}

		~FScopedLockProperties()
		{
			Lock->Trigger();
			Lock = nullptr;
		}
	private:
		FEvent* Lock = nullptr;
	};
	FScopedLockProperties ScopedLock(DestinationSkeletalMesh);
	FSkinnedAssetAsyncBuildScope AsyncBuildScope(DestinationSkeletalMesh);

	// Get a list of all the clothing assets affecting this LOD so we can re-apply later
	TArray<ClothingAssetUtils::FClothingAssetMeshBinding> ClothingBindings;
	TArray<UClothingAssetBase*> ClothingAssetsInUse;
	TArray<int32> ClothingAssetSectionIndices;
	TArray<int32> ClothingAssetInternalLodIndices;

	//SAve custom imported morph targets
	TMap<FString, TArray<FLODUtilities::FMorphTargetLodBackupData>> BackupImportedMorphTargetData;

	TArray<FSkinWeightProfileInfo> ExistingSkinWeightProfileInfos;
	TArray<FSkeletalMeshImportData> ExistingAlternateImportDataPerLOD;

	FSkeletalMeshModel* const SourceImportedResource = SourceSkeletalMesh->GetImportedModel();
	FSkeletalMeshModel* const DestImportedResource = DestinationSkeletalMesh->GetImportedModel();

	if (!SourceImportedResource || !DestImportedResource)
	{
		return false;
	}

	if (DestImportedResource->LODModels.IsValidIndex(LodIndex))
	{
		FLODUtilities::UnbindClothingAndBackup(DestinationSkeletalMesh, ClothingBindings, LodIndex);

		//Backup the lod custom imported morph
		FLODUtilities::BackupCustomImportedMorphTargetData(DestinationSkeletalMesh, BackupImportedMorphTargetData);

		int32 ExistingLodCount = DestinationSkeletalMesh->GetLODNum();

		//Extract all LOD Skin weight profiles data, we will re-apply them at the end
		ExistingSkinWeightProfileInfos = DestinationSkeletalMesh->GetSkinWeightProfiles();
		DestinationSkeletalMesh->GetSkinWeightProfiles().Reset();
		for (int32 AllLodIndex = 0; AllLodIndex < ExistingLodCount; ++AllLodIndex)
		{
			FSkeletalMeshLODModel& BuildLODModel = DestinationSkeletalMesh->GetImportedModel()->LODModels[AllLodIndex];
			BuildLODModel.SkinWeightProfiles.Reset();

			//Store the LOD alternate skinning profile data
			FSkeletalMeshImportData& SkeletalMeshImportData = ExistingAlternateImportDataPerLOD.AddDefaulted_GetRef();
			if (DestinationSkeletalMesh->HasMeshDescription(AllLodIndex))
			{
				PRAGMA_DISABLE_DEPRECATION_WARNINGS
				DestinationSkeletalMesh->LoadLODImportedData(AllLodIndex, SkeletalMeshImportData);
				PRAGMA_ENABLE_DEPRECATION_WARNINGS
			}
		}
	}

	//Lambda to call to re-apply the clothing
	auto ReapplyClothing = [&DestinationSkeletalMesh, &ClothingBindings, &SourceImportedResource, &LodIndex]()
	{
		if (SourceImportedResource->LODModels.IsValidIndex(LodIndex))
		{
			// Re-apply our clothing assets
			FLODUtilities::RestoreClothingFromBackup(DestinationSkeletalMesh, ClothingBindings, LodIndex);
		}
	};

	auto ReapplyCustomImportedMorphTarget = [&DestinationSkeletalMesh, &BackupImportedMorphTargetData, &LodIndex]()
	{
		if (FMeshDescription* MeshDescription = DestinationSkeletalMesh->GetMeshDescription(LodIndex))
		{
			if (FLODUtilities::RestoreCustomImportedMorphTargetData(DestinationSkeletalMesh, LodIndex, *MeshDescription, BackupImportedMorphTargetData))
			{
				USkeletalMesh::FCommitMeshDescriptionParams CommitParams;
				CommitParams.bForceUpdate = false;
				CommitParams.bMarkPackageDirty = false;
				DestinationSkeletalMesh->CommitMeshDescription(LodIndex, CommitParams);
			}
		}
	};

	auto ReapplyAlternateSkinning = [&DestinationSkeletalMesh, &ExistingSkinWeightProfileInfos, &ExistingAlternateImportDataPerLOD, &LodIndex, SrcLodIndex]()
	{
		if (ExistingSkinWeightProfileInfos.Num() > 0)
		{
			TArray<FSkinWeightProfileInfo>& SkinProfiles = DestinationSkeletalMesh->GetSkinWeightProfiles();
			SkinProfiles = ExistingSkinWeightProfileInfos;
			for (const FSkinWeightProfileInfo& ProfileInfo : SkinProfiles)
			{
				const int32 LodCount = DestinationSkeletalMesh->GetLODNum();
				for(int32 AllLodIndex = 0; AllLodIndex < LodCount; ++AllLodIndex)
				{
					if (!ExistingAlternateImportDataPerLOD.IsValidIndex(AllLodIndex))
					{
						continue;
					}
					if (!DestinationSkeletalMesh->HasMeshDescription(AllLodIndex))
					{
						continue;
					}
					const FSkeletalMeshLODInfo* LodInfo = DestinationSkeletalMesh->GetLODInfo(AllLodIndex);
					if (!LodInfo)
					{
						continue;
					}

					const FSkeletalMeshImportData& ExistingImportDataSrc = ExistingAlternateImportDataPerLOD[AllLodIndex];

					const FString ProfileNameStr = ProfileInfo.Name.ToString();

					FSkeletalMeshImportData ImportDataDest;
					PRAGMA_DISABLE_DEPRECATION_WARNINGS
					DestinationSkeletalMesh->LoadLODImportedData(AllLodIndex, ImportDataDest);
					PRAGMA_ENABLE_DEPRECATION_WARNINGS

					int32 PointNumberDest = ImportDataDest.Points.Num();
					int32 VertexNumberDest = ImportDataDest.Points.Num();

					if (ExistingImportDataSrc.Points.Num() != PointNumberDest)
					{
						UE_LOG(LogTemp, Error, TEXT("Alternate skinning mesh vertex number is different from the mesh LOD, we cannot apply the existing alternate skinning [%s] on the re-import skeletal mesh LOD %d [%s]"), *ProfileNameStr, LodIndex, *DestinationSkeletalMesh->GetName());
						continue;
					}

					//Replace the data into the destination bulk data and save it
					int32 ProfileIndex = SrcLodIndex;
					if (ImportDataDest.AlternateInfluenceProfileNames.Find(ProfileNameStr, ProfileIndex))
					{
						ImportDataDest.AlternateInfluenceProfileNames.RemoveAt(ProfileIndex);
						ImportDataDest.AlternateInfluences.RemoveAt(ProfileIndex);
					}
					int32 SrcProfileIndex = SrcLodIndex;
					if (ExistingImportDataSrc.AlternateInfluenceProfileNames.Find(ProfileNameStr, SrcProfileIndex))
					{
						ImportDataDest.AlternateInfluenceProfileNames.Add(ProfileNameStr);
						ImportDataDest.AlternateInfluences.Add(ExistingImportDataSrc.AlternateInfluences[SrcProfileIndex]);
					}

					//Resave the bulk data with the new or refreshed data
					PRAGMA_DISABLE_DEPRECATION_WARNINGS
					DestinationSkeletalMesh->SaveLODImportedData(AllLodIndex, ImportDataDest);
					PRAGMA_ENABLE_DEPRECATION_WARNINGS
				}
			}
		}
	};

	// Now we copy the base FSkeletalMeshLODModel from the imported skeletal mesh as the new LOD in the selected mesh.
	if (SourceImportedResource->LODModels.Num() == 0)
	{
		return false;
	}

	// Names of root bones must match.
	// If the names of root bones don't match, the LOD Mesh does not share skeleton with base Mesh.
	if (SourceSkeletalMesh->GetRefSkeleton().GetBoneName(0) != DestinationSkeletalMesh->GetRefSkeleton().GetBoneName(0))
	{
		UE_LOG(LogTemp, Error, TEXT("SkeletalMesh [%s] FLODUtilities::SetCustomLOD: Root bone in LOD is '%s' instead of '%s'.\nImport failed..")
		, *DestinationSkeletalMesh->GetName()
		, *SourceSkeletalMesh->GetRefSkeleton().GetBoneName(0).ToString()
		, *DestinationSkeletalMesh->GetRefSkeleton().GetBoneName(0).ToString());
		return false;
	}
	// We do some checking here that for every bone in the mesh we just imported, it's in our base ref skeleton, and the parent is the same.
	for (int32 i = 0; i < SourceSkeletalMesh->GetRefSkeleton().GetRawBoneNum(); i++)
	{
		int32 LODBoneIndex = i;
		FName LODBoneName = SourceSkeletalMesh->GetRefSkeleton().GetBoneName(LODBoneIndex);
		int32 BaseBoneIndex = DestinationSkeletalMesh->GetRefSkeleton().FindBoneIndex(LODBoneName);
		if (BaseBoneIndex == INDEX_NONE)
		{
			UE_LOG(LogTemp, Error, TEXT("FLODUtilities::SetCustomLOD: Bone '%s' not found in destination SkeletalMesh '%s'.\nImport failed.")
				, *LODBoneName.ToString()
				, *DestinationSkeletalMesh->GetName());
			return false;
		}

		if (i > 0)
		{
			int32 LODParentIndex = SourceSkeletalMesh->GetRefSkeleton().GetParentIndex(LODBoneIndex);
			FName LODParentName = SourceSkeletalMesh->GetRefSkeleton().GetBoneName(LODParentIndex);

			int32 BaseParentIndex = DestinationSkeletalMesh->GetRefSkeleton().GetParentIndex(BaseBoneIndex);
			FName BaseParentName = DestinationSkeletalMesh->GetRefSkeleton().GetBoneName(BaseParentIndex);

			if (LODParentName != BaseParentName)
			{
				UE_LOG(LogTemp, Error, TEXT("SkeletalMesh [%s] FLODUtilities::SetCustomLOD: Bone '%s' in LOD has parent '%s' instead of '%s'")
					, *DestinationSkeletalMesh->GetName()
					, *LODBoneName.ToString()
					, *LODParentName.ToString()
					, *BaseParentName.ToString());
				return false;
			}
		}
	}

	FSkeletalMeshLODModel& NewLODModel = SourceImportedResource->LODModels[SrcLodIndex];

	// If this LOD is not the base LOD, we check all bones we have sockets on are present in it.
	if (LodIndex > 0)
	{
		const TArray<USkeletalMeshSocket*>& Sockets = DestinationSkeletalMesh->GetMeshOnlySocketList();

		for (int32 i = 0; i < Sockets.Num(); i++)
		{
			// Find bone index the socket is attached to.
			USkeletalMeshSocket* Socket = Sockets[i];
			int32 SocketBoneIndex = SourceSkeletalMesh->GetRefSkeleton().FindBoneIndex(Socket->BoneName);

			// If this LOD does not contain the socket bone, abort import.
			if (SocketBoneIndex == INDEX_NONE)
			{
				UE_LOG(LogTemp, Error, TEXT("FLODUtilities::SetCustomLOD: This LOD is missing bone '%s' used by socket '%s'.\nAborting import.")
					, *Socket->BoneName.ToString()
					, *Socket->SocketName.ToString());
				return false;
			}
		}
	}

	{
		//The imported LOD is always in LOD 0 of the SourceSkeletalMesh
		const int32 SourceLODIndex = SrcLodIndex;
		if (SourceSkeletalMesh->HasMeshDescription(SourceLODIndex))
		{
			// Fix up the imported data bone indexes
			FSkeletalMeshImportData LODImportData;
			PRAGMA_DISABLE_DEPRECATION_WARNINGS
			SourceSkeletalMesh->LoadLODImportedData(SourceLODIndex, LODImportData);
			PRAGMA_ENABLE_DEPRECATION_WARNINGS
			const int32 LODImportDataBoneNumber = LODImportData.RefBonesBinary.Num();
			//We want to create a remap array so we can fix all influence easily
			TArray<int32> ImportDataBoneRemap;
			ImportDataBoneRemap.AddZeroed(LODImportDataBoneNumber);
			//We generate a new RefBonesBinary array to replace the existing one
			TArray<SkeletalMeshImportData::FBone> RemapedRefBonesBinary;
			RemapedRefBonesBinary.AddZeroed(DestinationSkeletalMesh->GetRefSkeleton().GetNum());
			for (int32 ImportBoneIndex = 0; ImportBoneIndex < LODImportDataBoneNumber; ++ImportBoneIndex)
			{
				SkeletalMeshImportData::FBone& ImportedBone = LODImportData.RefBonesBinary[ImportBoneIndex];
				int32 LODBoneIndex = ImportBoneIndex;
				FName LODBoneName = FName(*FSkeletalMeshImportData::FixupBoneName(ImportedBone.Name));
				int32 BaseBoneIndex = DestinationSkeletalMesh->GetRefSkeleton().FindBoneIndex(LODBoneName);
				ImportDataBoneRemap[ImportBoneIndex] = BaseBoneIndex;
				if (BaseBoneIndex != INDEX_NONE)
				{
					RemapedRefBonesBinary[BaseBoneIndex] = ImportedBone;
					if (RemapedRefBonesBinary[BaseBoneIndex].ParentIndex != INDEX_NONE)
					{
						RemapedRefBonesBinary[BaseBoneIndex].ParentIndex = ImportDataBoneRemap[RemapedRefBonesBinary[BaseBoneIndex].ParentIndex];
					}
				}
			}
			//Copy the new RefBonesBinary over the existing one
			LODImportData.RefBonesBinary = RemapedRefBonesBinary;

			//Fix the influences
			bool bNeedShrinking = false;
			const int32 InfluenceNumber = LODImportData.Influences.Num();
			for (int32 InfluenceIndex = InfluenceNumber - 1; InfluenceIndex >= 0; --InfluenceIndex)
			{
				SkeletalMeshImportData::FRawBoneInfluence& Influence = LODImportData.Influences[InfluenceIndex];
				Influence.BoneIndex = ImportDataBoneRemap[Influence.BoneIndex];
				if (Influence.BoneIndex == INDEX_NONE)
				{
					const int32 DeleteCount = 1;
					LODImportData.Influences.RemoveAt(InfluenceIndex, DeleteCount, EAllowShrinking::No);
					bNeedShrinking = true;
				}
			}
			//Shrink the array if we have deleted at least one entry
			if (bNeedShrinking)
			{
				LODImportData.Influences.Shrink();
			}
			//Save the fix up remap bone index
			PRAGMA_DISABLE_DEPRECATION_WARNINGS
			if(Save)SourceSkeletalMesh->SaveLODImportedData(SourceLODIndex, LODImportData);
			PRAGMA_ENABLE_DEPRECATION_WARNINGS
		}

		// Fix up the ActiveBoneIndices array.
		for (int32 ActiveIndex = 0; ActiveIndex < NewLODModel.ActiveBoneIndices.Num(); ActiveIndex++)
		{
			int32 LODBoneIndex = NewLODModel.ActiveBoneIndices[ActiveIndex];
			FName LODBoneName = SourceSkeletalMesh->GetRefSkeleton().GetBoneName(LODBoneIndex);
			int32 BaseBoneIndex = DestinationSkeletalMesh->GetRefSkeleton().FindBoneIndex(LODBoneName);
			checkSlow(BaseBoneIndex != INDEX_NONE);
			NewLODModel.ActiveBoneIndices[ActiveIndex] = static_cast<FBoneIndexType>(BaseBoneIndex);
		}

		// Fix up the chunk BoneMaps.
		for (int32 SectionIndex = 0; SectionIndex < NewLODModel.Sections.Num(); SectionIndex++)
		{
			FSkelMeshSection& Section = NewLODModel.Sections[SectionIndex];
			for (int32 BoneMapIndex = 0; BoneMapIndex < Section.BoneMap.Num(); BoneMapIndex++)
			{
				int32 LODBoneIndex = Section.BoneMap[BoneMapIndex];
				FName LODBoneName = SourceSkeletalMesh->GetRefSkeleton().GetBoneName(LODBoneIndex);
				int32 BaseBoneIndex = DestinationSkeletalMesh->GetRefSkeleton().FindBoneIndex(LODBoneName);
				checkSlow(BaseBoneIndex != INDEX_NONE);
				Section.BoneMap[BoneMapIndex] = static_cast<FBoneIndexType>(BaseBoneIndex);
			}
		}

		// Create the RequiredBones array in the LODModel from the ref skeleton.
		for (int32 RequiredBoneIndex = 0; RequiredBoneIndex < NewLODModel.RequiredBones.Num(); RequiredBoneIndex++)
		{
			FName LODBoneName = SourceSkeletalMesh->GetRefSkeleton().GetBoneName(NewLODModel.RequiredBones[RequiredBoneIndex]);
			int32 BaseBoneIndex = DestinationSkeletalMesh->GetRefSkeleton().FindBoneIndex(LODBoneName);
			if (BaseBoneIndex != INDEX_NONE)
			{
				NewLODModel.RequiredBones[RequiredBoneIndex] = static_cast<FBoneIndexType>(BaseBoneIndex);
			}
			else
			{
				NewLODModel.RequiredBones.RemoveAt(RequiredBoneIndex--);
			}
		}

		// Also sort the RequiredBones array to be strictly increasing.
		NewLODModel.RequiredBones.Sort();
		DestinationSkeletalMesh->GetRefSkeleton().EnsureParentsExistAndSort(NewLODModel.ActiveBoneIndices);
	}

	// To be extra-nice, we apply the difference between the root transform of the meshes to the verts.
	FMatrix44f LODToBaseTransform = FMatrix44f(SourceSkeletalMesh->GetRefPoseMatrix(0).InverseFast() * DestinationSkeletalMesh->GetRefPoseMatrix(0));

	for (int32 SectionIndex = 0; SectionIndex < NewLODModel.Sections.Num(); SectionIndex++)
	{
		FSkelMeshSection& Section = NewLODModel.Sections[SectionIndex];

		// Fix up soft verts.
		for (int32 i = 0; i < Section.SoftVertices.Num(); i++)
		{
			Section.SoftVertices[i].Position = LODToBaseTransform.TransformPosition(Section.SoftVertices[i].Position);
			Section.SoftVertices[i].TangentX = LODToBaseTransform.TransformVector(Section.SoftVertices[i].TangentX);
			Section.SoftVertices[i].TangentY = LODToBaseTransform.TransformVector(Section.SoftVertices[i].TangentY);
			Section.SoftVertices[i].TangentZ = LODToBaseTransform.TransformVector(Section.SoftVertices[i].TangentZ);
		}
	}

	TArray<FName> ExistingOriginalPerSectionMaterialImportName;
	bool bIsReImport = false;
	//Restore the LOD section data in case this LOD was reimport and some material match
	if (DestImportedResource->LODModels.IsValidIndex(LodIndex) && SourceImportedResource->LODModels.IsValidIndex(SrcLodIndex))
	{
		if (DestinationSkeletalMesh->HasMeshDescription(LodIndex))
		{
			FSkeletalMeshImportData DestinationLODImportData;
			PRAGMA_DISABLE_DEPRECATION_WARNINGS
			DestinationSkeletalMesh->LoadLODImportedData(LodIndex, DestinationLODImportData);
			PRAGMA_ENABLE_DEPRECATION_WARNINGS
			for (int32 SectionIndex = 0; SectionIndex < DestinationLODImportData.Materials.Num(); ++SectionIndex)
			{
				ExistingOriginalPerSectionMaterialImportName.Add(FName(*DestinationLODImportData.Materials[SectionIndex].MaterialImportName));
			}
		}
		bIsReImport = true;
	}

	// If we want to add this as a new LOD to this mesh - add to LODModels/LODInfo array.
	if (LodIndex == DestImportedResource->LODModels.Num())
	{
		DestImportedResource->LODModels.Add(new FSkeletalMeshLODModel());

		// Add element to LODInfo array.
		DestinationSkeletalMesh->AddLODInfo();
		check(DestinationSkeletalMesh->GetLODNum() == DestImportedResource->LODModels.Num());
	}

	const int32 SourceLODIndex = SrcLodIndex;
	if (SourceSkeletalMesh->HasMeshDescription(SourceLODIndex))
	{
		// Fix up the imported data bone indexes
		FSkeletalMeshImportData SourceLODImportData;
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		SourceSkeletalMesh->LoadLODImportedData(SourceLODIndex, SourceLODImportData);
		PRAGMA_ENABLE_DEPRECATION_WARNINGS
		FLODUtilities::FSkeletalMeshMatchImportedMaterialsParameters Parameters;
		Parameters.bIsReImport = bIsReImport;
		Parameters.LodIndex = LodIndex;
		Parameters.SkeletalMesh = DestinationSkeletalMesh;
		Parameters.ImportedMaterials = &SourceLODImportData.Materials;
		Parameters.ExistingOriginalPerSectionMaterialImportName = &ExistingOriginalPerSectionMaterialImportName;
		Parameters.CustomImportedLODModel = &NewLODModel;
		FLODUtilities::MatchImportedMaterials(Parameters);
	}

	// Set up LODMaterialMap to number of materials in new mesh.
	FSkeletalMeshLODInfo& LODInfo = *(DestinationSkeletalMesh->GetLODInfo(LodIndex));

	//Copy the build settings
	if (SourceSkeletalMesh->GetLODInfo(SrcLodIndex))
	{
		const FSkeletalMeshLODInfo& ImportedLODInfo = *(SourceSkeletalMesh->GetLODInfo(SrcLodIndex));
		LODInfo.BuildSettings = ImportedLODInfo.BuildSettings;
	}

	// Release all resources before replacing the model
	DestinationSkeletalMesh->PreEditChange(NULL);

	// Assign new FSkeletalMeshLODModel to desired slot in selected skeletal mesh.
	FSkeletalMeshLODModel::CopyStructure(&(DestImportedResource->LODModels[LodIndex]), &NewLODModel);

	//Copy the import data into the base skeletalmesh for the imported LOD
	FMeshDescription SourceMeshDescription;
	if (SourceSkeletalMesh->CloneMeshDescription(SrcLodIndex, SourceMeshDescription))
	{
		DestinationSkeletalMesh->ModifyMeshDescription(LodIndex);
		DestinationSkeletalMesh->CreateMeshDescription(LodIndex, MoveTemp(SourceMeshDescription));
		DestinationSkeletalMesh->CommitMeshDescription(LodIndex);
	}


	// If this LOD had been generated previously by automatic mesh reduction, clear that flag.
	LODInfo.bHasBeenSimplified = false;
	if (DestinationSkeletalMesh->GetLODSettings() == nullptr || !DestinationSkeletalMesh->GetLODSettings()->HasValidSettings() || DestinationSkeletalMesh->GetLODSettings()->GetNumberOfSettings() <= LodIndex)
	{
		//Make sure any custom LOD have correct settings (no reduce)
		LODInfo.ReductionSettings.NumOfTrianglesPercentage = 1.0f;
		LODInfo.ReductionSettings.MaxNumOfTriangles = MAX_uint32;
		LODInfo.ReductionSettings.MaxNumOfTrianglesPercentage = MAX_uint32;
		LODInfo.ReductionSettings.NumOfVertPercentage = 1.0f;
		LODInfo.ReductionSettings.MaxNumOfVerts = MAX_uint32;
		LODInfo.ReductionSettings.MaxNumOfVertsPercentage = MAX_uint32;
		LODInfo.ReductionSettings.MaxDeviationPercentage = 0.0f;
	}

	// Set LOD source filename
	DestinationSkeletalMesh->GetLODInfo(LodIndex)->SourceImportFilename = UAssetImportData::SanitizeImportFilename(SourceDataFilename, nullptr);
	DestinationSkeletalMesh->GetLODInfo(LodIndex)->bImportWithBaseMesh = false;

	ReapplyClothing();

	ReapplyCustomImportedMorphTarget();

	ReapplyAlternateSkinning();

	// Notification of success
	FNotificationInfo NotificationInfo(FText::GetEmpty());
	NotificationInfo.Text = FText::Format(NSLOCTEXT("UnrealEd", "LODImportSuccessful", "Mesh for LOD {0} imported successfully!"), FText::AsNumber(LodIndex));
	NotificationInfo.ExpireDuration = 5.0f;
	FSlateNotificationManager::Get().AddNotification(NotificationInfo);

	return true;

}

void UAssetCheckToolBPLibrary::ReplaceSKMReferences(UObject* Source, UObject* Dest)
{

	const UClass* FirstPendingDelete = Dest->GetClass();
	const UClass* AssetDataClass = Source->GetClass();
	bool CanReplace = ObjectTools::AreClassesInterchangeable( FirstPendingDelete, AssetDataClass );

	// Filter out blueprints of different types
	if (FirstPendingDelete->IsChildOf(UBlueprint::StaticClass()) && AssetDataClass->IsChildOf(UBlueprint::StaticClass()))
	{
		CanReplace = false;
	}

	if(!CanReplace)return;

	// Find which object the user has elected to be the "object to consolidate to"
	UObject* ObjectToConsolidateTo = Source;
	check( ObjectToConsolidateTo );

	TArray<UObject*> FinalConsolidationObjects;
	FinalConsolidationObjects.Add(Dest);

	// Perform the object consolidation
	bool bShowDeleteConfirmation = false;
	ObjectTools::FConsolidationResults ConsResults = ObjectTools::ConsolidateObjects( ObjectToConsolidateTo, FinalConsolidationObjects, bShowDeleteConfirmation );

	//UEditorLoadingAndSavingUtils::SavePackages(ObjectPtrDecay(ConsResults.DirtiedPackages), false);
	// If the consolidation went off successfully with no failed objects, prompt the user to checkout/save the packages dirtied by the operation
	if ( ConsResults.DirtiedPackages.Num() > 0 && ConsResults.FailedConsolidationObjs.Num() == 0)
	{
		FEditorFileUtils::FPromptForCheckoutAndSaveParams SaveParams;
		SaveParams.bCheckDirty = false;
		SaveParams.bPromptToSave = false;
		SaveParams.bIsExplicitSave = true;

		const UEditorLoadingSavingSettings* Settings = GetDefault<UEditorLoadingSavingSettings>();
		bool AutomaticallyCheckoutOnAssetModification = Settings->GetAutomaticallyCheckoutOnAssetModification();
		UEditorLoadingSavingSettings* Settings2 = const_cast<UEditorLoadingSavingSettings*>(Settings);
		Settings2->SetAutomaticallyCheckoutOnAssetModificationOverride(true);
		FEditorFileUtils::PromptForCheckoutAndSave( ObjectPtrDecay(ConsResults.DirtiedPackages), SaveParams);

		Settings2->SetAutomaticallyCheckoutOnAssetModificationOverride(AutomaticallyCheckoutOnAssetModification);
	}
}

void UAssetCheckToolBPLibrary::FixUpRedirectorInAssetsFolder(TArray<UObject*> Assets)
{

        TArray<FString> SelectedPaths;
        //DebugUtil::Print( FPaths::GetPath(Asset->GetPathName()));
        for(UObject* Asset : Assets)
        {
                SelectedPaths.Add(FPaths::GetPath(Asset->GetPathName()));
        }

        TArray<FString> SelectedPackages;
        //SelectedPackages.Add(Asset->GetPackage()->GetName());


        FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

        // Form a filter from the paths
        FARFilter Filter;
        Filter.bRecursivePaths = true;

        Filter.PackagePaths.Reserve(SelectedPaths.Num());
        for (const FString& Path : SelectedPaths)
        {
                Filter.PackagePaths.Emplace(*Path);
        }

        if (!SelectedPaths.IsEmpty())
        {
                Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());
        }

        // Query for a list of assets in the selected paths
        TArray<FAssetData> AssetList;
        AssetRegistryModule.Get().GetAssets(Filter, AssetList);

        Filter.Clear();

        Filter.PackageNames.Reserve(SelectedPackages.Num());
        for (const FString& PackageName : SelectedPackages)
        {
                Filter.PackageNames.Emplace(*PackageName);
        }

        if (!SelectedPackages.IsEmpty())
        {
                Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());
        }

        AssetRegistryModule.Get().GetAssets(Filter, AssetList);

        if (AssetList.Num() == 0)
        {
                return;
        }

        FScopedSlowTask SlowTask(3, LOCTEXT("FixupRedirectorsSlowTask", "Fixing up redirectors"));
        SlowTask.MakeDialog(true);

        SlowTask.EnterProgressFrame(1, LOCTEXT("FixupRedirectors_LoadAssets", "Loading Assets..."));
        TArray<UObject*> Objects;
        AssetViewUtils::FLoadAssetsSettings Settings{
                .bFollowRedirectors = false,
                .bAllowCancel = true,
        };
        AssetViewUtils::ELoadAssetsResult Result = AssetViewUtils::LoadAssetsIfNeeded(AssetList, Objects, Settings);
        if (Result != AssetViewUtils::ELoadAssetsResult::Cancelled && !SlowTask.ShouldCancel())
        {
                TArray<UObjectRedirector*> Redirectors;
                for (UObject* Object : Objects)
                {
                        if (UObjectRedirector* Redirector = Cast<UObjectRedirector>(Object))
                        {
                                Redirectors.Add(Redirector);
                        }
                }

                SlowTask.EnterProgressFrame(1, LOCTEXT("FixupRedirectors_FixupReferencers", "Fixing up referencers..."));
                // Load the asset tools module
                FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
                AssetToolsModule.Get().FixupReferencers(Redirectors, true, ERedirectFixupMode::DeleteFixedUpRedirectors);
        }
}


void UAssetCheckToolBPLibrary::ReloadAsset(UObject* Object)
{

	TArray<UPackage*> PackagesToReload;
	TArray<FAssetData> SelectedAssets;
	SelectedAssets.Add(FAssetData(Object));
	for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
	{
		const FAssetData& AssetData = *AssetIt;

		if (AssetData.AssetClassPath == UObjectRedirector::StaticClass()->GetClassPathName())
		{
			// Don't operate on Redirectors
			continue;
		}

		if (AssetData.AssetClassPath == UUserDefinedStruct::StaticClass()->GetClassPathName())
		{
			FNotificationInfo Notification(LOCTEXT("CannotReloadUserStruct", "User created structures cannot be safely reloaded."));
			Notification.ExpireDuration = 3.0f;
			FSlateNotificationManager::Get().AddNotification(Notification);
			continue;
		}

		if (AssetData.AssetClassPath == UUserDefinedEnum::StaticClass()->GetClassPathName())
		{
			FNotificationInfo Notification(LOCTEXT("CannotReloadUserEnum", "User created enumerations cannot be safely reloaded."));
			Notification.ExpireDuration = 3.0f;
			FSlateNotificationManager::Get().AddNotification(Notification);
			continue;
		}

		PackagesToReload.AddUnique(AssetData.GetPackage());
	}

	if (PackagesToReload.Num() > 0)
	{
		FText ErrorText = FText();
		UPackageTools::ReloadPackages(PackagesToReload,ErrorText,EReloadPackagesInteractionMode::AssumePositive);
	}
}

bool UAssetCheckToolBPLibrary::IsNaniteEnabled(UStaticMesh* StaticMesh)
{
	if (StaticMesh)
	{
		return StaticMesh->IsNaniteEnabled();
	}
	return false;
}

void UAssetCheckToolBPLibrary::GetSameSourceActors(AActor* SourceActor, TArray<AActor*>& SameSourceActors, bool bIncludeSelf)
{
	SameSourceActors.Empty();

	if (!IsValid(SourceActor))
	{
		return;
	}

#if WITH_EDITOR
	// 使用UE编辑器内置的选择同类型Actor功能
	if (GEditor)
	{
		// 获取编辑器选择管理器
		USelection* ActorSelection = GEditor->GetSelectedActors();

		// 临时清空选择
		GEditor->SelectNone(false, true);

		// 选择源Actor
		GEditor->SelectActor(SourceActor, true, false);

		// 执行选择同类型Actor的命令（相当于Shift+E）
		GEditor->Exec(SourceActor->GetWorld(), TEXT("ACTOR SELECT MATCHINGSTATICMESH"));

		// 获取选中的Actor
		for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
		{
			if (AActor* Actor = Cast<AActor>(*It))
			{
				// 根据bIncludeSelf参数决定是否包含源Actor本身
				if (bIncludeSelf || Actor != SourceActor)
				{
					SameSourceActors.Add(Actor);
				}
			}
		}

		// 恢复原始选择状态
		GEditor->SelectNone(false, true);
	}
#endif
}


void UAssetCheckToolBPLibrary::FixAllGroupActors()
{
	AGroupActor::FixupGroupActor();
}

TArray<AGroupActor*> UAssetCheckToolBPLibrary::FindRelatedGroupActors(const TArray<AActor*>& SelectedActors)
{
	TArray<AGroupActor*> RelatedGroupActors;

	if (UWorld* EditorWorld = GEditor->GetEditorWorldContext().World())
	{
		if (SelectedActors.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("No actors provided in SelectedActors."));
			return RelatedGroupActors;
		}

		// 1. 找出所有 GroupActor
		TArray<AGroupActor*> AllGroupActors;
		for (TActorIterator<AGroupActor> It(EditorWorld); It; ++It)
		{
			AllGroupActors.Add(*It);
		}

		if (AllGroupActors.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("No GroupActors found in the world."));
			return RelatedGroupActors;
		}

		// 2. 找出哪些 GroupActor 包含选中的 Actor
		for (AGroupActor* GroupActor : AllGroupActors)
		{
			if (!GroupActor || GroupActor->GroupActors.Num() == 0)
				continue;

			bool bContainsSelectedActor = false;
			for (TObjectPtr<AActor> MemberActor : GroupActor->GroupActors)
			{
				if (MemberActor && SelectedActors.Contains(MemberActor))
				{
					bContainsSelectedActor = true;
					break;
				}
			}

			if (bContainsSelectedActor)
			{
				RelatedGroupActors.Add(GroupActor);
			}
		}
	}

	return RelatedGroupActors;
}

void UAssetCheckToolBPLibrary::FixGroupActors(const TArray<AGroupActor*>& GroupActors)
{
	if (UWorld* EditorWorld = GEditor->GetEditorWorldContext().World())
	{
		for (AGroupActor* GroupActor : GroupActors)
		{
			if (GroupActor->GroupActors.IsEmpty())
			{
				FScopedRefreshAllBrowsers LevelRefreshAllBrowsers;
				GWorld->EditorDestroyActor( GroupActor, false );
				GroupActor->Destroy(); // 删除 GroupActor
				LevelRefreshAllBrowsers.Request();
			}
			if (GroupActor && GroupActor->GroupActors.ContainsByPredicate([](const TObjectPtr<AActor> Actor) { return Actor == nullptr; }))
			{
				GroupActor->Modify(); // 标记为已修改（用于撤销/重做）
				GroupActor->GroupActors.RemoveAll([](const TObjectPtr<AActor> Actor) { return Actor == nullptr; });
				GroupActor->GroupActors.Shrink(); // 释放多余内存

				if (GroupActor->GroupActors.IsEmpty())
				{
					FScopedRefreshAllBrowsers LevelRefreshAllBrowsers;
					GWorld->EditorDestroyActor( GroupActor, false );
					GroupActor->Destroy(); // 删除 GroupActor
					LevelRefreshAllBrowsers.Request();
				}
			}
		}
	}
}

void UAssetCheckToolBPLibrary::RemoveActorsFromGroupsAndDestroy(const TArray<AActor*>& ActorsToRemove)
{
	if (!GEditor || !GEditor->GetEditorWorldContext().World())
	{
		UE_LOG(LogTemp, Warning, TEXT("No editor world available"));
		return;
	}

	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();

	for (AActor* Actor : ActorsToRemove)
	{
		if (!Actor)
		{
			continue;
		}

		// 1. 获取父GroupActor
		AGroupActor* ParentGroup = Cast<AGroupActor>(Actor->GroupActor);

		// 2. 如果存在父GroupActor，则从父GroupActor中移除该Actor
		if (ParentGroup)
		{
			ParentGroup->Modify(); // 标记为已修改以便撤销/重做
			ParentGroup->Remove(*Actor); // 从Group中移除Actor
		}

		// 3. 销毁Actor
		Actor->Destroy();
	}
}


bool UAssetCheckToolBPLibrary::IsGroupActorEmpty(AGroupActor* GroupActor)
{
	if (UWorld* EditorWorld = GEditor->GetEditorWorldContext().World())
	{		FScopedRefreshAllBrowsers LevelRefreshAllBrowsers;
			LevelRefreshAllBrowsers.Request();
			if (GroupActor->GroupActors.IsEmpty())
			{
				GroupActor->Destroy(); // 删除 GroupActor}
				return true;
			}
			else
			{
				return false;
			}
	}
	return false;
}

bool UAssetCheckToolBPLibrary::CheckFXCollision(UObject* Object)
{
        if( Object->IsA(UStaticMesh::StaticClass()) )
        {
                UStaticMesh* StaticMesh = Cast<UStaticMesh>(Object);
                if(!StaticMesh)return false;

                UBodySetup* BodySetup = StaticMesh->GetBodySetup();
                if (!BodySetup)return false;

                int32 Count = BodySetup->AggGeom.BoxElems.Num();
                Count += BodySetup->AggGeom.SphereElems.Num();
                Count += BodySetup->AggGeom.SphylElems.Num();
                Count += BodySetup->AggGeom.ConvexElems.Num();
                return Count > 0 ;
        }
        else if(Object->IsA(USkeletalMesh::StaticClass()))
        {
                USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(Object);
                if(!SkeletalMesh)return false;
                UPhysicsAsset* PhysicsAsset = SkeletalMesh->GetPhysicsAsset();
                if(!PhysicsAsset)return false;

                return PhysicsAsset->SkeletalBodySetups.Num()>0;
        }
        else if(Object->IsA(UPaperSprite::StaticClass()))
        {
                UPaperSprite* PaperSprite = Cast<UPaperSprite>(Object);
                if(!PaperSprite)return false;

                // 获取 UClass 对象
                UClass* SpriteClass = PaperSprite->GetClass();

                // 获取 CollisionGeometry 属性
                FProperty* CollisionGeometryProperty = SpriteClass->FindPropertyByName(TEXT("CollisionGeometry"));
                if (!CollisionGeometryProperty)return false;

                // 使用反射获取 CollisionGeometry
                void* CollisionGeometryPtr = CollisionGeometryProperty->ContainerPtrToValuePtr<void>(PaperSprite);

                // 确保获取到了 CollisionGeometry
                if (!CollisionGeometryPtr)return false;
                FSpriteGeometryCollection* CollisionGeometry = reinterpret_cast<FSpriteGeometryCollection*>(CollisionGeometryPtr);

                return  CollisionGeometry->Shapes.Num() > 0;
        }

        return false;
}

void UAssetCheckToolBPLibrary::ClearFXCollision(UObject* Object)
{

        if( Object->IsA(UStaticMesh::StaticClass()) )
        {
                UStaticMesh* StaticMesh = Cast<UStaticMesh>(Object);
                if(!StaticMesh)return;

                // Close the mesh editor to prevent crashing. Reopen it after the mesh has been built.
                UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
                bool bStaticMeshIsEdited = false;
                if (AssetEditorSubsystem->FindEditorForAsset(StaticMesh, false))
                {
                        AssetEditorSubsystem->CloseAllEditorsForAsset(StaticMesh);
                        bStaticMeshIsEdited = true;
                }

                StaticMesh->GetBodySetup()->Modify();

                // Remove simple collisions
                StaticMesh->GetBodySetup()->RemoveSimpleCollision();

                // refresh collision change back to static mesh components
                //RefreshCollisionChange(*StaticMesh);

                // Request re-building of mesh with new collision shapes
                StaticMesh->PostEditChange();

                // Reopen MeshEditor on this mesh if the MeshEditor was previously opened in it
                if (bStaticMeshIsEdited)
                {
                        AssetEditorSubsystem->OpenEditorForAsset(StaticMesh);
                }
        }
        else if(Object->IsA(USkeletalMesh::StaticClass()))
        {
                USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(Object);
                if(!SkeletalMesh)return ;
                UPhysicsAsset* PhysicsAsset = SkeletalMesh->GetPhysicsAsset();
                if(!PhysicsAsset)return ;


                PhysicsAsset->ConstraintSetup.Empty();
                PhysicsAsset->SkeletalBodySetups.Empty();

                PhysicsAsset->UpdateBodySetupIndexMap();
                PhysicsAsset->UpdateBoundsBodiesArray();

                PhysicsAsset->PostEditChange();

                PhysicsAsset->MarkPackageDirty();
        }
        else if(Object->IsA(UPaperSprite::StaticClass()))
        {
                UPaperSprite* PaperSprite = Cast<UPaperSprite>(Object);
                if(!PaperSprite)return ;

                // 获取 UClass 对象
                UClass* SpriteClass = PaperSprite->GetClass();

                // 获取 CollisionGeometry 属性
                FProperty* CollisionGeometryProperty = SpriteClass->FindPropertyByName(TEXT("CollisionGeometry"));
                if (!CollisionGeometryProperty)return ;

                // 使用反射获取 CollisionGeometry
                void* CollisionGeometryPtr = CollisionGeometryProperty->ContainerPtrToValuePtr<void>(PaperSprite);

                // 确保获取到了 CollisionGeometry
                if (!CollisionGeometryPtr)return ;
                FSpriteGeometryCollection* CollisionGeometry = reinterpret_cast<FSpriteGeometryCollection*>(CollisionGeometryPtr);

                CollisionGeometry->Shapes.Empty();
                PaperSprite->MarkPackageDirty();
        }
}



#undef LOCTEXT_NAMESPACE