// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetAction/MeshLODWidget.h"

#include "DebugUtil.h"
#include "LODUtilities.h"
#include "EditorFramework/AssetImportData.h"
#include "Engine/SkeletalMeshLODSettings.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/SkinnedAssetCommon.h"
#include "Rendering/SkeletalMeshModel.h"
#include "ClothingAsset.h"

void UMeshLODWidget::InsertSkeletalMeshLODs(USkeletalMesh* SkeletalMesh, USkeletalMesh* LOD0)
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

	//迁移材质
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

bool UMeshLODWidget::SetCustomLOD(USkeletalMesh* DestinationSkeletalMesh, USkeletalMesh* SourceSkeletalMesh, const int32 LodIndex, const int32 SrcLodIndex,const FString& SourceDataFilename)
{
	if(!DestinationSkeletalMesh || !SourceSkeletalMesh)
	{
		return false;
	}

	FScopedSkeletalMeshPostEditChange ScopePostEditChange(DestinationSkeletalMesh);

	//If the imported LOD already exist, we will need to reimport all the skin weight profiles
	bool bMustReimportAlternateSkinWeightProfile = false;

	// Get a list of all the clothing assets affecting this LOD so we can re-apply later
	TArray<ClothingAssetUtils::FClothingAssetMeshBinding> ClothingBindings;
	TArray<UClothingAssetBase*> ClothingAssetsInUse;
	TArray<int32> ClothingAssetSectionIndices;
	TArray<int32> ClothingAssetInternalLodIndices;

	FSkeletalMeshModel* const SourceImportedResource = SourceSkeletalMesh->GetImportedModel();
	FSkeletalMeshModel* const DestImportedResource = DestinationSkeletalMesh->GetImportedModel();

	if (!SourceImportedResource || !DestImportedResource)
	{
		return false;
	}

	if (SourceImportedResource->LODModels.IsValidIndex(LodIndex))
	{
		bMustReimportAlternateSkinWeightProfile = true;
		FLODUtilities::UnbindClothingAndBackup(DestinationSkeletalMesh, ClothingBindings, LodIndex);
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

	FScopedSkeletalMeshPostEditChange ScopedPostEditChange(DestinationSkeletalMesh);

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
		if (!SourceSkeletalMesh->IsLODImportedDataEmpty(SourceLODIndex))
		{
			// Fix up the imported data bone indexes
			FSkeletalMeshImportData LODImportData;
			//SourceSkeletalMesh->GetMeshDescription()
			SourceSkeletalMesh->LoadLODImportedData(SourceLODIndex, LODImportData);
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
					const bool AllowShrink = false;
					LODImportData.Influences.RemoveAt(InfluenceIndex, DeleteCount, AllowShrink);
					bNeedShrinking = true;
				}
			}
			//Shrink the array if we have deleted at least one entry
			if (bNeedShrinking)
			{
				LODImportData.Influences.Shrink();
			}
			//Save the fix up remap bone index
			SourceSkeletalMesh->SaveLODImportedData(SourceLODIndex, LODImportData);
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
		if (!DestinationSkeletalMesh->IsLODImportedDataEmpty(LodIndex))
		{
			FSkeletalMeshImportData DestinationLODImportData;
			DestinationSkeletalMesh->LoadLODImportedData(LodIndex, DestinationLODImportData);
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
	
	// 处理LOD分段的材质导入
	const int32 SourceLODIndex = SrcLodIndex;
	if (!SourceSkeletalMesh->IsLODImportedDataEmpty(SourceLODIndex))
	{
		// Fix up the imported data bone indexes
		FSkeletalMeshImportData SourceLODImportData;
		SourceSkeletalMesh->LoadLODImportedData(SourceLODIndex, SourceLODImportData);
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
	USkeletalMesh::CopyImportedData(SrcLodIndex, SourceSkeletalMesh, LodIndex, DestinationSkeletalMesh);

	//PrintSkeletalMeshInfo(DestinationSkeletalMesh);
	// If this LOD had been generated previously by automatic mesh reduction, clear that flag.
	LODInfo.bHasBeenSimplified = false;
	if (DestinationSkeletalMesh->GetLODSettings() == nullptr || !DestinationSkeletalMesh->GetLODSettings()->HasValidSettings() || DestinationSkeletalMesh->GetLODSettings()->GetNumberOfSettings() <= LodIndex)
	{
		//Make sure any custom LOD have correct settings (no reduce)
		LODInfo.ReductionSettings.NumOfTrianglesPercentage = 1.0f;
		LODInfo.ReductionSettings.NumOfVertPercentage = 1.0f;
		LODInfo.ReductionSettings.MaxDeviationPercentage = 0.0f;
	}

	// Set LOD source filename
	DestinationSkeletalMesh->GetLODInfo(LodIndex)->SourceImportFilename = UAssetImportData::SanitizeImportFilename(SourceDataFilename, nullptr);
	DestinationSkeletalMesh->GetLODInfo(LodIndex)->bImportWithBaseMesh = false;

	ReapplyClothing();

	//Must be the last step because it cleanup the fbx importer to import the alternate skinning FBX
	if (bMustReimportAlternateSkinWeightProfile)
	{
		//TODO port skin weights utilities outside of UnrealEd module
		//FSkinWeightsUtilities::ReimportAlternateSkinWeight(DestinationSkeletalMesh, LodIndex);
	}
	
	// Notification of success
	FNotificationInfo NotificationInfo(FText::GetEmpty());
	NotificationInfo.Text = FText::Format(NSLOCTEXT("UnrealEd", "LODImportSuccessful", "Mesh for LOD {0} imported successfully!"), FText::AsNumber(LodIndex));
	NotificationInfo.ExpireDuration = 5.0f;
	FSlateNotificationManager::Get().AddNotification(NotificationInfo);
	
	return true;
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


