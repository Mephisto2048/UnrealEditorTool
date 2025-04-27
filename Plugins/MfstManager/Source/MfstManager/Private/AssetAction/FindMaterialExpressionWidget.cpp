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
		TObjectPtr<UMaterialExpressionWorldPosition> TextureBaseExpression = Cast<UMaterialExpressionWorldPosition>(Expression);
		if(!TextureBaseExpression) continue;
		
		WorldPositionExpressionArray.AddUnique(TextureBaseExpression);
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
