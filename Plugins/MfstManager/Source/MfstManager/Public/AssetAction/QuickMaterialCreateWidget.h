// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickMaterialCreateWidget.generated.h"

class UMaterialInstanceConstant;
class UMaterialExpressionTextureSample;
/**
 * 
 */
UCLASS()
class MFSTMANAGER_API UQuickMaterialCreateWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
public:
	/* Core */
	UFUNCTION(BlueprintCallable,Category = "Quick Create Material")
	void CreateMaterialFromSelectedTextures();

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Quick Create Material Menu")
	bool bCreateMaterialInstance = false;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Quick Create Material Menu")
	bool bCustomMaterialName = false;
	//当bCustomMaterialName为真时才能编辑
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Quick Create Material Menu",meta = (EditCondition = bCustomMaterialName))
	FString MaterialName = TEXT("M_");

	/* 支持的纹理后缀
	
	 */
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Supported Texture Names")
	TArray<FString> BaseColorNameArray = {
		TEXT("_basecolor"),
	};
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Supported Texture Names")
	TArray<FString> NormalNameArray = {
		TEXT("_normal"),
		TEXT("_norm"),
	};
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Supported Texture Names")
	TArray<FString> RoughnessNameArray = {
		TEXT("_rough"),
		TEXT("_roughness"),
	};
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Supported Texture Names")
	TArray<FString> AONameArray = {
		TEXT("_ao"),
		TEXT("_ambientocclusion"),
		TEXT("_AmbientOcclusion"),
		TEXT("_A"),
	};
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Supported Texture Names")
	TArray<FString> PackedNameArray = {
		TEXT("_AR"),
		TEXT("_OR"),
	};
private:
	bool ProcessSelectedData(const TArray<FAssetData>& SelectedData,TArray<UTexture2D*>& OutTexturesArray,FString& OutTexturesPath);
	bool CheckName(const FString& PackagePath,const FString& MaterialName);
	UMaterial* CreateMaterialAsset(const FString& Name,const FString& Path);
	UMaterialInstanceConstant* CreateMaterialInstance(UMaterial* Material,FString& Name,const FString& Path);
	void CreateMaterialNodes(UMaterial* Material,UTexture2D* Texture,uint32& PinsCount);
	/* Pin */
	bool ConnectBaseColorPin(UMaterialExpressionTextureSample* TextureSampleNode,UTexture2D* Texture,UMaterial* Material);
	bool ConnectNormalPin(UMaterialExpressionTextureSample* TextureSampleNode,UTexture2D* Texture,UMaterial* Material);
	bool ConnectRoughnessPin(UMaterialExpressionTextureSample* TextureSampleNode,UTexture2D* Texture,UMaterial* Material);
	bool ConnectAOPin(UMaterialExpressionTextureSample* TextureSampleNode,UTexture2D* Texture,UMaterial* Material);
	bool ConnectARPin(UMaterialExpressionTextureSample* TextureSampleNode,UTexture2D* Texture,UMaterial* Material);
};
