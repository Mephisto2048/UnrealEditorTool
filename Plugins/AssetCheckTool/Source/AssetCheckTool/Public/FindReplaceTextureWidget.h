// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "FindReplaceTextureWidget.generated.h"

class UMaterialExpressionTextureBase;
class UTextureThumbnailRenderer;
class UMaterialExpressionTextureSample;
/**
 *
 */
UCLASS()
class ASSETCHECKTOOL_API  UFindReplaceTextureWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
public:
	/* Find */
	UFUNCTION(BlueprintCallable,Category = "Find Textures")
	void FindTextures();

	UFUNCTION(BlueprintCallable,Category = "Find Textures")
	void FindTexturesInBatches(TArray<FString> AssetPaths);
	
	UFUNCTION(BlueprintCallable,Category = "Find Textures")
	void OpenMaterialEditor();

	UFUNCTION(BlueprintCallable,Category = "Find Textures")
	TArray<FString> GetSelectedPathsInContentBrowser();

	UFUNCTION(BlueprintCallable,Category = "Find Textures")
	void GetFilteredTextures();

	UFUNCTION(BlueprintPure,Category = "Find Textures")
	void GetTargetTexturePath(UMaterialExpressionTextureBase* Expression,FString& Outpath);
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly/*,Category = "Find Textures"*/)
	FString PathArray = TEXT("");
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Find Textures")
	FString MaterialName = TEXT("M_");
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly/*,Category = "Find Textures"*/)
	FString TextureList = TEXT("T_");

	/* Replace */
	UFUNCTION(BlueprintCallable,Category = "Replace Textures")
	void ReplaceTextures();

	UFUNCTION(BlueprintCallable,Category = "Replace Textures")
	void ResetTargetTextures();

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Replace Textures")
	TObjectPtr<UTexture2D> TargetColorTexture;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Replace Textures")
	TObjectPtr<UTexture2D> TargetLinearColorTexture;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Replace Textures")
	TObjectPtr<UTexture2D> TargetNormalTexture;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Replace Textures")
	TObjectPtr<UTexture2D> TargetGrayscaleTexture;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Replace Textures")
	TObjectPtr<UTexture2D> TargetLinearGrayscaleTexture;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Replace Textures")
	TObjectPtr<UTexture2D> TargetAlphaTexture;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Replace Textures")
	TObjectPtr<UTexture2D> TargetMasksTexture;

protected:
	UPROPERTY(BlueprintReadOnly)
	TArray<TObjectPtr<UMaterialExpressionTextureBase>> TextureBaseExpressionArray;
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> ExpressionNameArray;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMaterial> CurrentMaterial;
	
	UPROPERTY(BlueprintReadOnly)
	TArray<TObjectPtr<UMaterial>> CurrentMaterials;

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> CurrentMaterialNames;
private:
	bool ProcessSelectedMaterials(const TArray<FAssetData>& SelectedData,TArray<UMaterial*>& OutMaterialsArray,FString& MaterialsPath);
	bool ProcessTargetTextures();

	void FindMaterialEditorTabs();

	void NativeConstruct() override;
	TObjectPtr<UTexture2D> LoadTextureByPath(const FString& TexturePath);
	TObjectPtr<UMaterial> LoadMaterialByPath(const FString& MaterialPath);

	int32 MaterialNum = 0;
	FString MaterialPath = TEXT("");
};
