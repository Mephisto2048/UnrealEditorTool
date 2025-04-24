// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "MeshToFoliageWidget.generated.h"

class UEditorActorSubsystem;
/**
 * 
 */
UCLASS()
class MFSTMANAGER_API UMeshToFoliageWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
public:
	/* 批量选择 */
	UFUNCTION(BlueprintCallable,Category = "ActorBatchSelection")
	void SelectActorsWithName();

	UFUNCTION(BlueprintCallable,Category = "ActorBatchSelection")
	void ConvertMeshActorsToFoliage();
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "ActorBatchSelection")
	TEnumAsByte<ESearchCase::Type> SearchCase = ESearchCase::CaseSensitive;

	
private:
	UPROPERTY()
	UEditorActorSubsystem* EditorActorSubsystem;

	bool GetEditorActorSubSystem();
};
