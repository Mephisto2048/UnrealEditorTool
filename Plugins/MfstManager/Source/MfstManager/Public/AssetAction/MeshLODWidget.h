// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "MeshLODWidget.generated.h"

/**
 * 
 */
UCLASS()
class MFSTMANAGER_API UMeshLODWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void FillEmptyMaterialSlots(USkeletalMesh* SkeletalMesh);
	
	UFUNCTION(BlueprintCallable)
	FString InsertSkeletalMeshLODs(USkeletalMesh* SkeletalMesh,USkeletalMesh* LOD0);
	
	UFUNCTION(BlueprintCallable)
	void SetCustomLOD(USkeletalMesh* SkeletalMesh,USkeletalMesh* LOD0);
	
	UFUNCTION(BlueprintCallable)
	void AddMaterialSlot(USkeletalMesh* SkeletalMesh);
	
	UFUNCTION(BlueprintCallable)
	void SetMaterialSlot(USkeletalMesh* SkeletalMesh,USkeletalMesh* LOD0);
	
	UFUNCTION(BlueprintCallable)
	int32 GetMaterialIndex(USkeletalMesh* SkeletalMesh, int32 LODIndex, int32 SectionIndex);

	UFUNCTION(BlueprintCallable)
	void PrintSkeletalMeshInfo(USkeletalMesh* SkeletalMesh);
	
	UFUNCTION(BlueprintCallable)
	void SortSkeletalMeshLOD(USkeletalMesh* SkeletalMesh);

	UFUNCTION(BlueprintCallable)
	void RefreshSectionMaterials(USkeletalMesh* SkeletalMesh, USkeletalMesh* LOD0);

	UFUNCTION(BlueprintCallable)
	FString GetSectionVertPosition(USkeletalMesh* SkeletalMesh, int32 LODIndex, int32 SectionIndex);

	UFUNCTION(BlueprintCallable)
	void RemoveEmptyMaterialSlot(USkeletalMesh* SkeletalMesh);

	/* Delete */
	UFUNCTION(BlueprintCallable)
	void ReplaceSKMReferences(UObject* Source, UObject* Dest);

	UFUNCTION(BlueprintCallable)
	void FixUpRedirectorInAssetsFolder(TArray<UObject*> Assets);

	UFUNCTION(BlueprintCallable)
	void FixUpRedirector(TArray<UObject*> InRedirectors);
private:
	bool SetCustomLOD(USkeletalMesh* DestinationSkeletalMesh, USkeletalMesh* SourceSkeletalMesh, const int32 LodIndex, const int32 SrcLodIndex,const FString& SourceDataFilename);
	
	
};
