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
	void InsertSkeletalMeshLODs(USkeletalMesh* SkeletalMesh,USkeletalMesh* LOD0);

	bool SetCustomLOD(USkeletalMesh* DestinationSkeletalMesh, USkeletalMesh* SourceSkeletalMesh, const int32 LodIndex, const int32 SrcLodIndex,const FString& SourceDataFilename);
};
