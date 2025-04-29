// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetAction/FindMaterialExpressionWidget.h"

#include "DebugUtil.h"
#include "EditorUtilityLibrary.h"
#include "Materials/MaterialExpressionWorldPosition.h"

void UFindMaterialExpressionWidget::FindWorldPositionExpression()
{
	WorldPositionExpressionNum = 0;
	TArray<FAssetData> SelectedAssetsData= UEditorUtilityLibrary::GetSelectedAssetData();

	//如果没选任何资产，就退出
	if(SelectedAssetsData.Num() == 0)
	{
		DebugUtil::MessageDialog(TEXT("No Material Selected"));
		return ;
	}

	UObject* SelectedAsset = SelectedAssetsData[0].GetAsset();

	UMaterial* SelectedMaterial = Cast<UMaterial>(SelectedAsset);

	if(!SelectedMaterial)
	{
		DebugUtil::MessageDialog(SelectedAsset->GetName() + TEXT(" is not a material"));
		return ;
	}

	FMaterialExpressionCollection& MaterialExpressionCollection = SelectedMaterial->GetExpressionCollection();
	TArray<TObjectPtr<UMaterialExpression>> MaterialExpressions = MaterialExpressionCollection.Expressions;

	//查找Material的TextureBase节点，添加到数组中
	for(TObjectPtr<UMaterialExpression> Expression : MaterialExpressions)
	{
		TObjectPtr<UMaterialExpressionWorldPosition> WorldPositionExpression = Cast<UMaterialExpressionWorldPosition>(Expression);
		if(!WorldPositionExpression) continue;
		
		WorldPositionExpressionArray.AddUnique(WorldPositionExpression);
	}

	//查找Material的Function的TextureSample节点，添加到数组中
	TArray<UMaterialFunctionInterface*> DependentFunctions;
	SelectedMaterial->GetDependentFunctions(DependentFunctions);
	for(UMaterialFunctionInterface* DependentFunction : DependentFunctions)
	{
		TConstArrayView<TObjectPtr<UMaterialExpression>> Expressions = DependentFunction->GetExpressions();
	
		for(TObjectPtr<UMaterialExpression> Expression : Expressions)
		{
			TObjectPtr<UMaterialExpressionWorldPosition> TextureBaseExpression = Cast<UMaterialExpressionWorldPosition>(Expression);
			if(!TextureBaseExpression) continue;

			WorldPositionExpressionArray.AddUnique(TextureBaseExpression);
	
		}
	}

	WorldPositionExpressionNum = WorldPositionExpressionArray.Num();
}

int32 UFindMaterialExpressionWidget::FindMaterialWorldPositionExpression(const FString& MaterialPath, FString& OutDetails)
{
	// 创建 FSoftObjectPath 对象
	FSoftObjectPath SoftObjectPath(MaterialPath);

	// 使用 LoadObject 加载
	UMaterial* LoadedMaterial = Cast<UMaterial>(SoftObjectPath.TryLoad());

	if (!LoadedMaterial)
	{
		DebugUtil::MessageDialog(MaterialPath+TEXT("is not a material"));
		return 0;
	}
	
	FMaterialExpressionCollection& MaterialExpressionCollection = LoadedMaterial->GetExpressionCollection();
	TArray<TObjectPtr<UMaterialExpression>> MaterialExpressions = MaterialExpressionCollection.Expressions;
	TArray<TObjectPtr<UMaterialExpressionWorldPosition>> WorldPositionArray;
	
	//查找Material的节点，添加到数组中
	for(TObjectPtr<UMaterialExpression> Expression : MaterialExpressions)
	{
		TObjectPtr<UMaterialExpressionWorldPosition> WorldPositionExpression = Cast<UMaterialExpressionWorldPosition>(Expression);
		if(!WorldPositionExpression) continue;
		
		WorldPositionArray.AddUnique(WorldPositionExpression);
	}
	
	OutDetails += FString::Printf(TEXT("%s : %d\n"), *LoadedMaterial->GetName(), WorldPositionArray.Num());
	
	//查找Material的Function的节点，添加到数组中
	TArray<UMaterialFunctionInterface*> DependentFunctions;
	LoadedMaterial->GetDependentFunctions(DependentFunctions);
	for(UMaterialFunctionInterface* DependentFunction : DependentFunctions)
	{
		TConstArrayView<TObjectPtr<UMaterialExpression>> Expressions = DependentFunction->GetExpressions();
		int32 Num = 0;
		for(TObjectPtr<UMaterialExpression> Expression : Expressions)
		{
			TObjectPtr<UMaterialExpressionWorldPosition> TextureBaseExpression = Cast<UMaterialExpressionWorldPosition>(Expression);
			if(!TextureBaseExpression) continue;

			WorldPositionArray.AddUnique(TextureBaseExpression);
			Num++;
		}
		
		OutDetails += FString::Printf(TEXT("%s : %d\n"), *DependentFunction->GetName(), Num);
	}

	return WorldPositionArray.Num();
}
