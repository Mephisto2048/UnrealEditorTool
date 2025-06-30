// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "CollisionCheckWidget.generated.h"

/**
 * 
 */
UCLASS()
class MFSTMANAGER_API UCollisionCheckWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable)
	bool CheckFXCollision(UObject* Object);

	UFUNCTION(BlueprintCallable)
	void ClearFXCollision(UObject* Object);
};
