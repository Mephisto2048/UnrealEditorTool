// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetAction/MeshLODWidget.h"

#include "AssetDeleteModel.h"
#include "AssetSelection.h"
#include "DebugUtil.h"
#include "LODUtilities.h"
#include "EditorFramework/AssetImportData.h"
#include "Engine/SkeletalMeshLODSettings.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/SkinnedAssetCommon.h"
#include "Rendering/SkeletalMeshModel.h"
#include "ClothingAsset.h"
#include "FileHelpers.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/SkinnedAssetAsyncCompileUtils.h"
#include "C:\Program Files\Epic Games\UE_5.5\Engine\Source\Editor\UnrealEd\Private\AutoReimport\AutoReimportUtilities.h"
#include <AssetToolsModule.h>

#include "AssetViewUtils.h"
#include "Animation/DebugSkelMeshComponent.h"
#include "Kismet2/BlueprintEditorUtils.h"
#define LOCTEXT_NAMESPACE "PersonaMeshDetails"
void UMeshLODWidget::FillEmptyMaterialSlots(USkeletalMesh* SkeletalMesh)
{
	TArray<FSkeletalMaterial>& MaterialSlots = SkeletalMesh->GetMaterials();
	
	UMaterialInterface* Material = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial")));
	
	for(FSkeletalMaterial& MaterialSlot : MaterialSlots)
	{
		if(MaterialSlot.MaterialInterface == nullptr) MaterialSlot.MaterialInterface = Material;
	}
	SkeletalMesh->PostEditChange();
}

FString UMeshLODWidget::InsertSkeletalMeshLODs(USkeletalMesh* SkeletalMesh, USkeletalMesh* LOD0)
{
	FSkeletalMeshModel* ImportedModel = SkeletalMesh->GetImportedModel();
	int32 LODNum = ImportedModel->LODModels.Num();

	DebugUtil::Print(FString::FromInt(LODNum));
	
	for(int i = LODNum; i>0 ;i--)
	{
		SetCustomLOD(SkeletalMesh,SkeletalMesh,i,i-1,TEXT(""));
	}
	LODNum = ImportedModel->LODModels.Num();
	
	FLODUtilities::SetCustomLOD(SkeletalMesh,LOD0,LODNum,TEXT(""));
	LODNum = ImportedModel->LODModels.Num();
	DebugUtil::Print(FString::FromInt(LODNum));
	
	// 获取最后一个LOD模型的索引
	int32 LastIndex = LODNum - 1;
    
	SetCustomLOD(SkeletalMesh,SkeletalMesh,0,LastIndex,TEXT(""));
	
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
	
	//构造顶点数与材质的映射
	auto GetMaterialIndexLambda = [](USkeletalMesh* SkeletalMesh, int32 LODIndex, int32 SectionIndex) -> int32
	{
		const FSkeletalMeshLODInfo* LODInfoPtr = SkeletalMesh->GetLODInfo(LODIndex);
		if (LODInfoPtr && LODInfoPtr->LODMaterialMap.IsValidIndex(SectionIndex) && LODInfoPtr->LODMaterialMap[SectionIndex] != INDEX_NONE)
		{
			return LODInfoPtr->LODMaterialMap[SectionIndex];
		}
		return SkeletalMesh->GetImportedModel()->LODModels[LODIndex].Sections[SectionIndex].MaterialIndex;
	};
	TMap<FVector3f, UMaterialInterface*> LOD0Map;
        
	const FSkeletalMeshLODModel& LODModel = LOD0->GetImportedModel()->LODModels[0];
        
	for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); SectionIndex++)
	{
		const FSkelMeshSection& Section = LODModel.Sections[SectionIndex];
		int32 VertexCount = Section.GetNumVertices();

		//FString String = FString::FromInt(VertexCount) + Section.SoftVertices[1].Position.ToString();
		FVector3f VertPosition = Section.SoftVertices[1].Position;
		
		int32 MaterialIndex = GetMaterialIndexLambda(LOD0,0,SectionIndex);
		UMaterialInterface* MaterialInterface = LOD0->GetMaterials()[MaterialIndex].MaterialInterface;
		
		if(!MaterialInterface)
		{
			UMaterialInterface* DefaultMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial")));
			MaterialInterface = DefaultMaterial;
		}
			
		LOD0Map.Add(VertPosition, MaterialInterface);
	}

	//迁移材质
	TArray<FSkelMeshSection> SKMSections = ImportedModel->LODModels[0].Sections;
	TArray<int32>& SKMMaterialMap = SkeletalMesh->GetLODInfo(0)->LODMaterialMap;
	
	for (int i = 0;i < SKMSections.Num(); i++)
	{
		//int32 VertNum = SKMSections[i].GetNumVertices();
		//FString String = FString::FromInt(VertNum) + SKMSections[i].SoftVertices[1].Position.ToString();
		FVector3f VertPosition = SKMSections[i].SoftVertices[1].Position;
		UMaterialInterface* Material = LOD0Map.FindRef(VertPosition);
		
		if(!Material)
		{
			for (const TPair<FVector3f, UMaterialInterface*>& Pair : LOD0Map)
			{
				// 计算差异
				FVector3f Difference = Pair.Key - VertPosition;
				float Length = Difference.Length();
				// 找到最小差异的键
				if (Length < 0.005f)
				{
				
					Material = Pair.Value;
				}
			}
		}
		
		
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
	return TEXT("");
}

void UMeshLODWidget::SetCustomLOD(USkeletalMesh* SkeletalMesh, USkeletalMesh* LOD0)
{
	SetCustomLOD(SkeletalMesh,LOD0,1,0,TEXT(""));
}

void UMeshLODWidget::AddMaterialSlot(USkeletalMesh* SkeletalMesh)
{
	SkeletalMesh->GetMaterials().Add(FSkeletalMaterial());
	SkeletalMesh->PostEditChange();
}



void UMeshLODWidget::SetMaterialSlot(USkeletalMesh* SkeletalMesh, USkeletalMesh* LOD0)
{
	FSkeletalMeshModel* LOD0Model = LOD0->GetImportedModel();
	TArray<FSkelMeshSection> LOD0Sections = LOD0Model->LODModels[0].Sections;
	TArray<int32>& LOD0MaterialMap = LOD0->GetLODInfo(0)->LODMaterialMap;
	
	TArray<int32>& SKMMaterialMap = SkeletalMesh->GetLODInfo(0)->LODMaterialMap;
	
	for(const FSkelMeshSection& Section : LOD0Sections)
	{ 
		int32 MaterialIndex = LOD0MaterialMap[Section.BaseIndex];
		
		UMaterialInterface* MaterialInterface = LOD0->GetMaterials()[MaterialIndex].MaterialInterface;
		
		FName SlotName = LOD0->GetMaterials()[MaterialIndex].MaterialSlotName;

		FSkeletalMaterial SkeletalMaterial = FSkeletalMaterial(MaterialInterface,SlotName);
		SkeletalMesh->GetMaterials().Add(SkeletalMaterial);

		SKMMaterialMap[Section.BaseIndex] = SkeletalMesh->GetMaterials().Num()-1;
	}
	SkeletalMesh->PostEditChange();
}



int32 UMeshLODWidget::GetMaterialIndex(USkeletalMesh* SkeletalMesh, int32 LODIndex, int32 SectionIndex)
{
	const FSkeletalMeshLODInfo* LODInfoPtr = SkeletalMesh->GetLODInfo(LODIndex);
	if (LODInfoPtr && LODInfoPtr->LODMaterialMap.IsValidIndex(SectionIndex) && LODInfoPtr->LODMaterialMap[SectionIndex] != INDEX_NONE)
	{
		return LODInfoPtr->LODMaterialMap[SectionIndex];
	}
	return SkeletalMesh->GetImportedModel()->LODModels[LODIndex].Sections[SectionIndex].MaterialIndex;
}

void UMeshLODWidget::PrintSkeletalMeshInfo(USkeletalMesh* SkeletalMesh)
{
	FSkeletalMeshModel* const ImportedResource = SkeletalMesh->GetImportedModel();

	FSkeletalMeshLODModel* LODModel = &ImportedResource->LODModels[0];
	
	const int32 SectionNum = LODModel->Sections.Num();

	FString OutString;
	for(int i = 0;i<SectionNum;i++)
	{
		int32 VertNum = LODModel->Sections[i].GetNumVertices();
		OutString += FString::Printf(TEXT("Section %d :\n"), i);
		OutString += FString::Printf(TEXT("VertNum : %d\n"), VertNum);
	}
	DebugUtil::Print(OutString);
	
}

void UMeshLODWidget::SortSkeletalMeshLOD(USkeletalMesh* SkeletalMesh)
{
	FSkeletalMeshModel* ImportedModel = SkeletalMesh->GetImportedModel();
	TIndirectArray<FSkeletalMeshLODModel>& LODModelsArray = ImportedModel->LODModels;

	// 获取最后一个LOD模型的索引
	int32 LastIndex = LODModelsArray.Num() - 1;
    
	SetCustomLOD(SkeletalMesh,SkeletalMesh,0,LastIndex,TEXT(""));

}

void UMeshLODWidget::RefreshSectionMaterials(USkeletalMesh* SkeletalMesh, USkeletalMesh* LOD0)
{
	TFunction<TMap<int32, UMaterialInterface*>(USkeletalMesh*)> CreateVertexMaterialMap = [this](USkeletalMesh* Mesh) -> TMap<int32, UMaterialInterface*>
	{
		TMap<int32, UMaterialInterface*> Map;
        
		if (!Mesh || !Mesh->GetImportedModel())
			return Map;
        
		const int32 LODIndex = 0;
		if (!Mesh->GetImportedModel()->LODModels.IsValidIndex(LODIndex))
			return Map;
        
		const FSkeletalMeshLODModel& LODModel = Mesh->GetImportedModel()->LODModels[LODIndex];
        
		for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); SectionIndex++)
		{
			const FSkelMeshSection& Section = LODModel.Sections[SectionIndex];
			int32 VertexCount = Section.GetNumVertices();

			int32 MaterialIndex = GetMaterialIndex(Mesh,0,SectionIndex);
			UMaterialInterface* MaterialInterface = Mesh->GetMaterials()[MaterialIndex].MaterialInterface;
            
			// 注意：如果多个 Section 有相同的顶点数，后出现的会覆盖前一个
			Map.Add(VertexCount, MaterialInterface);
		}
        
		return Map;
	};
	TMap<int32, UMaterialInterface*> LOD0Map = CreateVertexMaterialMap(LOD0);

	FSkeletalMeshModel* ImportedModel = SkeletalMesh->GetImportedModel();
	TArray<FSkelMeshSection> SKMSections = ImportedModel->LODModels[0].Sections;
	TArray<int32>& SKMMaterialMap = SkeletalMesh->GetLODInfo(0)->LODMaterialMap;
	
	for (int i = 0;i < SKMSections.Num(); i++)
	{
		int32 VertNum = SKMSections[i].GetNumVertices();
		UMaterialInterface* Material = LOD0Map.FindRef(VertNum);

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

	SkeletalMesh->PostEditChange();
}

FString UMeshLODWidget::GetSectionVertPosition(USkeletalMesh* SkeletalMesh, int32 LODIndex, int32 SectionIndex)
{
	FVector3f pos =  SkeletalMesh->GetImportedModel()->LODModels[LODIndex].Sections[SectionIndex].SoftVertices[1].Position;
	return pos.ToString();
}

void UMeshLODWidget::RemoveEmptyMaterialSlot(USkeletalMesh* SkeletalMesh)
{
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

void UMeshLODWidget::RemoveSkeletalMeshLOD(USkeletalMesh* SkeletalMesh,int32 LODIndex)
{
	USkeletalMesh* SkelMesh = SkeletalMesh;
	check(SkelMesh);
	check(SkelMesh->IsValidLODIndex(LODIndex));
	TArray<FSkelMeshSection>& Sections = SkeletalMesh->GetImportedModel()->LODModels[0].Sections;
	
	if (LODIndex >= 0)
	{
		FText RemoveLODText = FText::Format(LOCTEXT("OnPersonaRemoveLOD", "Persona editor: Remove LOD {0}"), LODIndex);
		FScopedTransaction Transaction(TEXT(""), RemoveLODText, SkelMesh);
		SkelMesh->Modify();

		FScopedSuspendAlternateSkinWeightPreview ScopedSuspendAlternateSkinnWeightPreview(SkelMesh);
		//PostEditChange scope
		{
			FScopedSkeletalMeshPostEditChange ScopedPostEditChange(SkelMesh);
				
			FSkeletalMeshUpdateContext UpdateContext;
			UpdateContext.SkeletalMesh = SkelMesh;
			//UpdateContext.AssociatedComponents.Push(GetPersonaToolkit()->GetPreviewMeshComponent());

			FLODUtilities::RemoveLOD(UpdateContext, LODIndex);

			if (SkelMesh->GetLODSettings())
			{
				SkelMesh->GetLODSettings()->SetLODSettingsToMesh(SkelMesh);
			}
		}
	}

	
	
}

void UMeshLODWidget::RemoveUnusedMaterialSlot(USkeletalMesh* SkeletalMesh)
{
	TWeakObjectPtr<USkeletalMesh> SkeletalMeshPtr = SkeletalMesh;
	//int32 TotalMaterialNum = SkeletalMesh->GetMaterials().Num();
	auto IsMaterialUsed = [this](USkeletalMesh* SkeletalMesh, int32 MaterialIndex) -> bool
	{
		if (SkeletalMesh == nullptr)
		{
			return false; // 如果SkeletalMesh无效，直接返回false
		}

		const int32 MaterialCount = SkeletalMesh->GetMaterials().Num();
		if (MaterialIndex < 0 || MaterialIndex >= MaterialCount)
		{
			return false; // 如果MaterialIndex不合法，返回false
		}

		// 获取模型
		FSkeletalMeshModel* ImportedResource = SkeletalMesh->GetImportedModel();
		if (!ImportedResource)
		{
			return false; // 如果无法获取到模型，返回false
		}

		// 遍历LOD和Section
		for (int32 LODIndex = 0; LODIndex < ImportedResource->LODModels.Num(); ++LODIndex)
		{
			FSkeletalMeshLODInfo& Info = *(SkeletalMesh->GetLODInfo(LODIndex));
        
			for (int32 SectionIndex = 0; SectionIndex < ImportedResource->LODModels[LODIndex].Sections.Num(); ++SectionIndex)
			{
				if (GetMaterialIndex(SkeletalMesh,LODIndex, SectionIndex) == MaterialIndex)
				{
					return true; // 找到材质被使用，返回true
				}
			}
		}

		return false; // 如果没有找到该材质被使用，返回false
	};
	for(int i = SkeletalMesh->GetMaterials().Num()-1;i>0;i--)
	{
		int32 MaterialIndex = i;
		
		if(IsMaterialUsed(SkeletalMesh,MaterialIndex)) continue;
		
		SkeletalMeshPtr->Modify();
		{
			FScopedSkeletalMeshPostEditChange ScopedPostEditChange(SkeletalMeshPtr.Get());
			//When we delete a material slot we must invalidate the DDC because material index is not part of the DDC key by design
			SkeletalMeshPtr->InvalidateDeriveDataCacheGUID();

			SkeletalMeshPtr->GetMaterials().RemoveAt(MaterialIndex);
			FSkeletalMeshModel* Model = SkeletalMeshPtr->GetImportedModel();

			int32 NumLODInfos = SkeletalMeshPtr->GetLODNum();

			//When we delete a material slot we need to fix all MaterialIndex after the deleted index
			for (int32 LODInfoIdx = 0; LODInfoIdx < NumLODInfos; LODInfoIdx++)
			{
				TArray<FSkelMeshSection>& Sections = Model->LODModels[LODInfoIdx].Sections;
				TArray<int32>& LODMaterialMap = SkeletalMeshPtr->GetLODInfo(LODInfoIdx)->LODMaterialMap;
				for (int32 SectionIndex = 0; SectionIndex < Sections.Num(); ++SectionIndex)
				{
					int32 SectionMaterialIndex = Sections[SectionIndex].MaterialIndex;
					if (LODMaterialMap.IsValidIndex(SectionIndex) && LODMaterialMap[SectionIndex] != INDEX_NONE)
					{
						SectionMaterialIndex = LODMaterialMap[SectionIndex];
					}
					if (SectionMaterialIndex > MaterialIndex)
					{
						SectionMaterialIndex--;
						//Patch the lod material map
						while (!LODMaterialMap.IsValidIndex(SectionIndex))
						{
							LODMaterialMap.Add(INDEX_NONE);
						}
						LODMaterialMap[SectionIndex] = SectionMaterialIndex;
						Sections[SectionIndex].MaterialIndex--;
					}
				}
			}
		}
		
	}
	
	SkeletalMesh->PostEditChange();
}

void UMeshLODWidget::ReplaceSKMReferences(UObject* Source, UObject* Dest)
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
	
	// If the consolidation went off successfully with no failed objects, prompt the user to checkout/save the packages dirtied by the operation
	if ( ConsResults.DirtiedPackages.Num() > 0 && ConsResults.FailedConsolidationObjs.Num() == 0)
	{
		FEditorFileUtils::FPromptForCheckoutAndSaveParams SaveParams;
		SaveParams.bCheckDirty = false;
		SaveParams.bPromptToSave = false;
		SaveParams.bIsExplicitSave = true;

		FEditorFileUtils::PromptForCheckoutAndSave( ObjectPtrDecay(ConsResults.DirtiedPackages), SaveParams);
	}
	
	//UEditorLoadingAndSavingUtils::SavePackages(ObjectPtrDecay(ConsResults.DirtiedPackages), false);
}

void UMeshLODWidget::FixUpRedirectorInAssetsFolder(TArray<UObject*> Assets)
{
	#define LOCTEXT_NAMESPACE "FixUpRedirectorInAssetFolder"
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
		AssetToolsModule.Get().FixupReferencers(Redirectors, false, ERedirectFixupMode::DeleteFixedUpRedirectors);
	}
}

void UMeshLODWidget::FixUpRedirector(TArray<UObject*> InRedirectors)
{
	TArray<UObjectRedirector*> Redirectors;
	for (UObject* Object : InRedirectors)
	{
		if (UObjectRedirector* Redirector = Cast<UObjectRedirector>(Object))
		{
			Redirectors.Add(Redirector);
		}
	}
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsModule.Get().FixupReferencers(Redirectors, true, ERedirectFixupMode::PromptForDeletingRedirectors);

	//ObjectTools::DeleteObjects(InRedirectors, false);
}


bool UMeshLODWidget::SetCustomLOD(USkeletalMesh* DestinationSkeletalMesh, USkeletalMesh* SourceSkeletalMesh, const int32 LodIndex, const int32 SrcLodIndex,const FString& SourceDataFilename)
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
			SourceSkeletalMesh->SaveLODImportedData(SourceLODIndex, LODImportData);
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
