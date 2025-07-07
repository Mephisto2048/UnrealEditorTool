// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "FixupRedirectorsWidget.generated.h"

/**
 * 
 */
UCLASS()
class MFSTMANAGER_API UFixupRedirectorsWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

	void BatchFixupAsset(TArray<UObject*> Assets);
	
	UFUNCTION(BlueprintCallable)
	void BatchFixupRedirectors(TArray<FString> SelectedPaths,TArray<FString> SelectedPackages);
	
	UFUNCTION(BlueprintCallable)
	void GetSelectedPathPackage(TArray<FString>& SelectedPaths,TArray<FString>& SelectedPackages);
};
