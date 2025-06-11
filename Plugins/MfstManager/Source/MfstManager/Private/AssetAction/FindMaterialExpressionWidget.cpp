// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetAction/FindMaterialExpressionWidget.h"

#include "DebugUtil.h"
#include "EditorUtilityLibrary.h"
#include "MaterialGraph/MaterialGraph.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialinstanceConstant.h"

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

	if(SelectedMaterial->MaterialGraph)
	{
		TArray<UEdGraphNode*> UnusedNodes;
		SelectedMaterial -> MaterialGraph -> GetUnusedExpressions(UnusedNodes);
		WorldPositionExpressionNum = UnusedNodes.Num();
	}
}

int32 UFindMaterialExpressionWidget::FindMaterialWorldPositionExpression(const FString& MaterialPath, FString& OutDetails)
{
	OutDetails.Empty();
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

	if(LoadedMaterial ->MaterialGraph)
	{
		TArray<UEdGraphNode*> UnusedNodes;
		LoadedMaterial -> MaterialGraph -> GetUnusedExpressions(UnusedNodes);
		if(UnusedNodes.Num()>0)
		{
			for(UEdGraphNode* Node : UnusedNodes)
			{
				FString Name = Node->GetName();
				OutDetails += FString::Printf(TEXT("%s\n"), *Name);
			}
			OutDetails += "\n";
		}
	}
	
	
	
	TArray<TObjectPtr<UMaterialExpressionWorldPosition>> WorldPositionArray;
	
	//查找Material的节点，添加到数组中
	for(TObjectPtr<UMaterialExpression> Expression : MaterialExpressions)
	{
		if(!(Expression->GraphNode))DebugUtil::ShowNotify(TEXT("can not find GraphNode"));
		if(! (Expression->HasConnectedOutputs())) continue;
		
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


FString UFindMaterialExpressionWidget::GetMaterialProperty(const FString& PropertyName)
{
	if(!CurrentMaterial) return TEXT("");
	//FVariant<bool,int32> PropertyValue;
	//PropertyValue = CurrentMaterial->TwoSided;

	UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(CurrentMaterial);
	UMaterialInterface* ParentMaterial = MaterialInstance->Parent;
	UMaterial* ParentAsMaterial = Cast<UMaterial>(ParentMaterial);
	
	// 尝试获取 UMaterial 的 UProperty
	FProperty* Property = FindFProperty<FProperty>(ParentAsMaterial->GetClass(), *PropertyName);
	if (!Property)
	{
		UE_LOG(LogTemp, Warning, TEXT("Property '%s' not found in material!"), *PropertyName);
		return TEXT("");
	}

	/*FBoolProperty* BoolProp = CastField<FBoolProperty>(Property);
	bool Value = false;
	BoolProp->GetValue_InContainer(CurrentMaterial, &Value);
	return Value ? TEXT("true") : TEXT("false");*/

	FUInt16Property* UIntProp = CastField<FUInt16Property>(Property);
	FUInt16Property::TCppType Value = 0;
	UIntProp->GetValue_InContainer(CurrentMaterial, &Value);
	bool bIsTwoSided = (Value & 1) != 0; // 提取最低位
	return bIsTwoSided ? TEXT("true") : TEXT("false");
}

void UFindMaterialExpressionWidget::CancelMaterialInstanceOverride(const FString& ParameterName)
{
	if(!CurrentMaterial) return;
	
	for(FScalarParameterValue ScalarParameterValue : CurrentMaterial->ScalarParameterValues)
	{
		if(ParameterName==ScalarParameterValue.ParameterInfo.Name.ToString())
		{
			CurrentMaterial->ScalarParameterValues.Remove(ScalarParameterValue);
		}
	}

	for(FVectorParameterValue VectorParameterValue : CurrentMaterial->VectorParameterValues)
	{
		if(ParameterName==VectorParameterValue.ParameterInfo.Name.ToString())
		{
			CurrentMaterial->VectorParameterValues.Remove(VectorParameterValue);
		}
	}

	for(FTextureParameterValue TextureParameterValue : CurrentMaterial->TextureParameterValues)
	{
		if(ParameterName==TextureParameterValue.ParameterInfo.Name.ToString())
		{
			CurrentMaterial->TextureParameterValues.Remove(TextureParameterValue);
		}
	}

	CurrentMaterial->PostEditChange();
}

void UFindMaterialExpressionWidget::CancelMaterialInstanceOverride_(UMaterialInstanceConstant* MaterialInst,const FString& ParameterName)
{
	if(!MaterialInst) return;
	
	for(FScalarParameterValue ScalarParameterValue : MaterialInst->ScalarParameterValues)
	{
		if(ParameterName==ScalarParameterValue.ParameterInfo.Name.ToString())
		{
			MaterialInst->ScalarParameterValues.Remove(ScalarParameterValue);
		}
	}

	for(FVectorParameterValue VectorParameterValue : MaterialInst->VectorParameterValues)
	{
		if(ParameterName==VectorParameterValue.ParameterInfo.Name.ToString())
		{
			MaterialInst->VectorParameterValues.Remove(VectorParameterValue);
		}
	}

	for(FTextureParameterValue TextureParameterValue : MaterialInst->TextureParameterValues)
	{
		if(ParameterName==TextureParameterValue.ParameterInfo.Name.ToString())
		{
			MaterialInst->TextureParameterValues.Remove(TextureParameterValue);
		}
	}

	MaterialInst->PostEditChange();
}

int32  UFindMaterialExpressionWidget::LevenshteinDistance(const FString& Str1, const FString& Str2)
{
	const int32 Len1 = Str1.Len();
	const int32 Len2 = Str2.Len();

	// 动态规划表（二维数组）
	TMap<FIntPoint, int32> DP; // 使用 FIntPoint (i, j) 作为键

	// 初始化边界条件
	for (int32 i = 0; i <= Len1; ++i)
	{
		DP.Add(FIntPoint(i, 0), i);
	}
	for (int32 j = 0; j <= Len2; ++j)
	{
		DP.Add(FIntPoint(0, j), j);
	}

	// 填充 DP 表
	for (int32 i = 1; i <= Len1; ++i)
	{
		for (int32 j = 1; j <= Len2; ++j)
		{
			const TCHAR Char1 = Str1[i - 1];
			const TCHAR Char2 = Str2[j - 1];

			if (Char1 == Char2)
			{
				// 字符相同，无需操作
				DP.Add(FIntPoint(i, j), DP[FIntPoint(i - 1, j - 1)]);
			}
			else
			{
				// 取插入、删除、替换的最小值 +1
				const int32 Insert = DP[FIntPoint(i, j - 1)] + 1;
				const int32 Delete = DP[FIntPoint(i - 1, j)] + 1;
				const int32 Replace = DP[FIntPoint(i - 1, j - 1)] + 1;

				DP.Add(FIntPoint(i, j), FMath::Min(FMath::Min(Insert, Delete), Replace));
			}
		}
	}

	return DP[FIntPoint(Len1, Len2)];
}

float UFindMaterialExpressionWidget::CalculateStringSimilarity(const FString& Str1, const FString& Str2)
{
	const int32 MaxLen = FMath::Max(Str1.Len(), Str2.Len());
	if (MaxLen == 0) // 如果两个字符串都为空
	{
		return 1.0f; // 视为完全匹配
	}

	const int32 Distance = LevenshteinDistance(Str1, Str2);
	return 1.0f - (static_cast<float>(Distance) / MaxLen);
}

float UFindMaterialExpressionWidget::CalculateJaroWinkler(const FString& String1, const FString& String2,float PrefixScale, int32 MaxPrefixLength)
{
	 // 如果两个字符串完全相同
    if (String1.Equals(String2, ESearchCase::IgnoreCase))
    {
        return 1.0f;
    }

    // 转换为小写以忽略大小写
    FString LowerStr1 = String1.ToLower();
    FString LowerStr2 = String2.ToLower();

    int32 Len1 = LowerStr1.Len();
    int32 Len2 = LowerStr2.Len();

    // 如果其中一个字符串为空
    if (Len1 == 0 || Len2 == 0)
    {
        return 0.0f;
    }

    // 计算匹配窗口大小
    int32 MatchDistance = FMath::Max(Len1, Len2) / 2 - 1;
    if (MatchDistance < 0)
    {
        MatchDistance = 0;
    }

    // 记录匹配的字符
	TArray<bool> Matched1;
	Matched1.Init(false, Len1);
	TArray<bool> Matched2;
	Matched2.Init(false, Len2);

    int32 Matches = 0;

    // 找到匹配的字符
    for (int32 i = 0; i < Len1; ++i)
    {
        int32 Start = FMath::Max(0, i - MatchDistance);
        int32 End = FMath::Min(Len2 - 1, i + MatchDistance);

        for (int32 j = Start; j <= End; ++j)
        {
            if (!Matched2[j] && LowerStr1[i] == LowerStr2[j])
            {
                Matched1[i] = true;
                Matched2[j] = true;
                Matches++;
                break;
            }
        }
    }

    // 如果没有匹配的字符
    if (Matches == 0)
    {
        return 0.0f;
    }

    // 计算 transpositions
    int32 Transpositions = 0;
    int32 k = 0;

    for (int32 i = 0; i < Len1; ++i)
    {
        if (Matched1[i])
        {
            while (!Matched2[k])
            {
                k++;
            }
            if (LowerStr1[i] != LowerStr2[k])
            {
                Transpositions++;
            }
            k++;
        }
    }

    Transpositions /= 2;

    // 计算 Jaro 距离
    float Jaro = ((float)Matches / Len1 +
                  (float)Matches / Len2 +
                  ((float)(Matches - Transpositions) / Matches)) / 3.0f;

    // 计算公共前缀
    int32 CommonPrefix = 0;
    while (CommonPrefix < Len1 && CommonPrefix < Len2 && CommonPrefix < MaxPrefixLength &&
           LowerStr1[CommonPrefix] == LowerStr2[CommonPrefix])
    {
        CommonPrefix++;
    }

    // 计算 Jaro-Winkler 距离
    float JaroWinkler = Jaro + (CommonPrefix * PrefixScale * (1.0f - Jaro));

	// 长度不匹配惩罚
	float LengthRatio = FMath::Min(Len1, Len2) / (float)FMath::Max(Len1, Len2);
	float LengthPenalty = 1.0f - LengthRatio;
	JaroWinkler = JaroWinkler * (1.0f - LengthPenalty);
	
    // 确保结果在 [0, 1] 范围内
    JaroWinkler = FMath::Clamp(JaroWinkler, 0.0f, 1.0f);

    return JaroWinkler;
}

float UFindMaterialExpressionWidget::CheckNumericMatch(const FString& Str1, const FString& Str2)
{
	// 提取字符串中的数字部分
	auto ExtractNumbers = [](const FString& Str) -> FString {
		FString Result;
		for (TCHAR Char : Str)
		{
			if (FChar::IsDigit(Char))
			{
				Result += Char;
			}
		}
		return Result;
	};

	FString Numbers1 = ExtractNumbers(Str1);
	FString Numbers2 = ExtractNumbers(Str2);
	if(Numbers1 == Numbers2) return 1;
	if(Numbers1.IsEmpty() || Numbers2.IsEmpty()) return 0;

	int32 Int1 = FCString::Atoi(*Numbers1);
	int32 Int2 = FCString::Atoi(*Numbers2);
	float f = Int1==Int2?1:(1- FMath::Clamp(abs(Int1-Int2)/10.0,0,1))*0.5;
	
	int32 Len1 = Numbers1.Len();
	int32 Len2 = Numbers2.Len();

	// 动态规划表（二维数组）
	TMap<FIntPoint, int32> DP; // 使用 FIntPoint (i, j) 作为键

	// 初始化边界条件
	for (int32 i = 0; i <= Len1; ++i)
	{
		DP.Add(FIntPoint(i, 0), i);
	}
	for (int32 j = 0; j <= Len2; ++j)
	{
		DP.Add(FIntPoint(0, j), j);
	}

	// 填充 DP 表
	for (int32 i = 1; i <= Len1; ++i)
	{
		for (int32 j = 1; j <= Len2; ++j)
		{
			const TCHAR Char1 = Str1[i - 1];
			const TCHAR Char2 = Str2[j - 1];

			if (Char1 == Char2)
			{
				// 字符相同，无需操作
				DP.Add(FIntPoint(i, j), DP[FIntPoint(i - 1, j - 1)]);
			}
			else
			{
				// 取插入、删除、替换的最小值 +1
				const int32 Insert = DP[FIntPoint(i, j - 1)] + 1;
				const int32 Delete = DP[FIntPoint(i - 1, j)] + 1;
				const int32 Replace = DP[FIntPoint(i - 1, j - 1)] + 1;

				DP.Add(FIntPoint(i, j), FMath::Min(FMath::Min(Insert, Delete), Replace));
			}
		}
	}

	const int32 Distance = DP[FIntPoint(Len1, Len2)];
	const int32 MaxLen = FMath::Max(Len1, Len2);
	float LevenshteinDistance = 1.0f - (static_cast<float>(Distance) / MaxLen);
	
	float result = f*0.7+ LevenshteinDistance*0.3;
	return result;
}
