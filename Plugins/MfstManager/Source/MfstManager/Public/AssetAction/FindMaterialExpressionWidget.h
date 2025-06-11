// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "FindMaterialExpressionWidget.generated.h"

class UMaterialInstanceConstant;
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

	UFUNCTION(BlueprintCallable,Category = "Find Material Expression")
	int32 FindMaterialWorldPositionExpression(const FString& MaterialPath, FString& OutDetails);

	UFUNCTION(BlueprintCallable,Category = "Get Material Property")
	FString GetMaterialProperty(const FString& PropertyName);

	UFUNCTION(BlueprintCallable,Category = "Get Material Property")
	void CancelMaterialInstanceOverride(const FString& ParameterName);

	UFUNCTION(BlueprintCallable,Category = "Get Material Property")
	void CancelMaterialInstanceOverride_(UMaterialInstanceConstant* MaterialInst,const FString& ParameterName);

	/* StringMatch */
	UFUNCTION(BlueprintCallable,Category = "Get Material Property")
	int32 LevenshteinDistance(const FString& Str1,const FString& Str2);

	UFUNCTION(BlueprintCallable,Category = "Get Material Property")
	float CalculateStringSimilarity(const FString& Str1, const FString& Str2);

	UFUNCTION(BlueprintCallable,Category = "Get Material Property")
	float CalculateJaroWinkler(const FString& String1, const FString& String2, float PrefixScale = 0.1f, int32 MaxPrefixLength = 4);

	UFUNCTION(BlueprintCallable,Category = "Get Material Property")
	float CheckNumericMatch(const FString& Str1, const FString& Str2);
	
	
	UPROPERTY(EditAnywhere,Category = "Get Material Property")
	UMaterialInstanceConstant* CurrentMaterial = nullptr;
	
	UPROPERTY(VisibleAnywhere,Category = "Find Material Expression")
	int32  WorldPositionExpressionNum = 0;
private:
	UPROPERTY()
	TArray<TObjectPtr<UMaterialExpressionWorldPosition>> WorldPositionExpressionArray;
};
