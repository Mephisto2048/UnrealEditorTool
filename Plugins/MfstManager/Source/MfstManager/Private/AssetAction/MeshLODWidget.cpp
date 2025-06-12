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
	
	//FSkeletalMeshLODInfo* LODInfo = SkeletalMesh->GetLODInfo(0);
	for(int i = LODNum; i>0 ;i--)
	{
		SetCustomLOD(SkeletalMesh,SkeletalMesh,i,i-1,TEXT(""));
	}
	//SetCustomLOD(SkeletalMesh,SkeletalMesh,3,2,TEXT(""));
	FLODUtilities::SetCustomLOD(SkeletalMesh,LOD0,0,TEXT(""));
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
	if (DestImportedResource->LODModels.IsValidIndex(LodIndex) && SourceImportedResource->LODModels.IsValidIndex(SrcLodIndex))//m
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