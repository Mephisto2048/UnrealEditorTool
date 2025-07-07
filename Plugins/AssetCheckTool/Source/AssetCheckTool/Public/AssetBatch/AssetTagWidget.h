// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "AssetTagWidget.generated.h"

/**
 * 
 */
UCLASS()
class ASSETCHECKTOOL_API UAssetTagWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void CheckSelectedAssetsTag_InBP();
};
