// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "FindMaterialExpressionWidget.generated.h"

class UMaterialExpressionWorldPosition;
/**
 * 
 */
UCLASS()
class MFSTMANAGER_API UFindMaterialExpressionWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable,Category = "Find Material Expression")
	void FindWorldPositionExpression();

	UPROPERTY(VisibleAnywhere,Category = "Find Material Expression")
	int32  WorldPositionExpressionNum = 0;
private:
	UPROPERTY()
	TArray<TObjectPtr<UMaterialExpressionWorldPosition>> WorldPositionExpressionArray;
};
