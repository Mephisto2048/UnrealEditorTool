// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "MeshConsoleCheckWidget.generated.h"

/**
 *
 */
UCLASS()
class ASSETCHECKTOOL_API UMeshConsoleCheckWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void MeshConsoleCheck_InBP(const FString& PackagePath,bool pathmode);

	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void MeshAutoCheck_InBP();

	UFUNCTION(BlueprintCallable)
	bool MeshConsoleCheck(const FString& PackagePath,const int32 Num = 1000);

	UFUNCTION(BlueprintCallable)
	void MeshConsoleCheckLog(bool flag);

	UFUNCTION(BlueprintCallable,BlueprintPure)
	bool IsCommandlet();
//private:
	template<typename T>
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LoadAssetByPackagePath"), Category = "AssetCheckTool")
	auto LoadAssetByPackagePath(const FString& PackagePath) -> T*;
};


