// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetAction/QuickMaterialCreateWidget.h"

#include "AssetToolsModule.h"
#include "DebugUtil.h"
#include "EditorAssetLibrary.h"
#include "EditorUtilityLibrary.h"
#include "Factories/MaterialFactoryNew.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialinstanceConstant.h"

void UQuickMaterialCreateWidget::CreateMaterialFromSelectedTextures()
{
	
	if(bCustomMaterialName)
	{
		if (MaterialName.IsEmpty() || MaterialName.Equals(TEXT("M_")))
		{
			DebugUtil::MessageDialog(TEXT("Please Enter a valid Name"));
		}
	}

	TArray<FAssetData> SelectedAssetsData= UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<UTexture2D*> TexturesArray;
	FString TexturesPath;
	
	/* Process Selected Data */
	if(!ProcessSelectedData(SelectedAssetsData,TexturesArray,TexturesPath))
	{
		//把MaterialName设置为默认值
		MaterialName = TEXT("M_");
		return;
	}
	if(TexturesArray.IsEmpty())
	{
		DebugUtil::MessageDialog(TEXT("TexturesArray is empty"));
		return;
	}
	if (!CheckName(TexturesPath,MaterialName)){MaterialName = TEXT("M_");return;}

	UMaterial* CreatedMaterial = CreateMaterialAsset(MaterialName,TexturesPath);
	if(!CreatedMaterial)
	{
		DebugUtil::MessageDialog(TEXT("Failed to create material"));
		return;
	}
	uint32 PinsCount = 0;
	//为每一个选中的纹理决定如何连接到材质上
	for(UTexture2D* TextureInArray:TexturesArray)
	{
		CreateMaterialNodes(CreatedMaterial,TextureInArray,PinsCount);
	}
	
	if(PinsCount > 0)
	{
		DebugUtil::ShowNotify(FString::FromInt(PinsCount) + TEXT(" Textures is Connected"));
	}

	//创建材质实例
	if(bCreateMaterialInstance)
	{
		CreateMaterialInstance(CreatedMaterial,MaterialName,TexturesPath);
	}
	
	MaterialName = TEXT("M_");
}

bool UQuickMaterialCreateWidget::ProcessSelectedData(const TArray<FAssetData>& SelectedAssetsData,
	TArray<UTexture2D*>& OutTexturesArray, FString& OutTexturesPath)
{
	//TODO: 检查选中的资产是否在同一文件夹
	
	//如果没选任何资产，就退出
	if(SelectedAssetsData.Num() == 0)
	{
		DebugUtil::MessageDialog(TEXT("No Texture Selected"));
		return false;
	}
	
	//我们想只为材质设置一次名字
	bool bMaterialNameSet = false;
	
	for(const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		UObject* SelectedAsset = SelectedAssetData.GetAsset();
		//如果选中非法资产，就退出
		UTexture2D* SelectedTexture = Cast<UTexture2D>(SelectedAsset);
		if(!SelectedTexture)
		{
			DebugUtil::MessageDialog(SelectedAsset->GetName() + TEXT(" is not a texture"));
			return false;
		}

		OutTexturesArray.Add(SelectedTexture);

		//只为材质的路径设置一次
		if (OutTexturesPath.IsEmpty())
		{
			OutTexturesPath = SelectedAssetData.PackagePath.ToString();
		}
		//自动生成材质名字
		if(!bCustomMaterialName && !bMaterialNameSet)
		{
			MaterialName = SelectedAsset->GetName();
			/* 材质名称移除纹理的后缀 */
			for(const FString& BaseColorName:BaseColorNameArray)
			{
				if(SelectedAsset->GetName().EndsWith(BaseColorName))
				{
					MaterialName.RemoveFromEnd(BaseColorName);
				}
			}
			for(const FString& NormalName:NormalNameArray)
			{
				if(SelectedAsset->GetName().EndsWith(NormalName))
				{
					MaterialName.RemoveFromEnd(NormalName);
				}
			}
			for(const FString& RoughnessName:RoughnessNameArray)
			{
				if(SelectedAsset->GetName().EndsWith(RoughnessName))
				{
					MaterialName.RemoveFromEnd(RoughnessName);
				}
			}
			for(const FString& AOName:AONameArray)
			{
				if(SelectedAsset->GetName().EndsWith(AOName))
				{
					MaterialName.RemoveFromEnd(AOName);
				}
			}
			for(const FString& PackedName:PackedNameArray)
			{
				if(SelectedAsset->GetName().EndsWith(PackedName))
				{
					MaterialName.RemoveFromEnd(PackedName);
				}
			}
			MaterialName.RemoveFromStart(TEXT("T_"));
			MaterialName.InsertAt(0,TEXT("M_"));
			bMaterialNameSet = true;
		}
	}
	return true;
}

bool UQuickMaterialCreateWidget::CheckName(const FString& PackagePath, const FString& CheckedName)
{
	//得到文件夹中所有资产的路径的数组
	TArray<FString>ListPaths =  UEditorAssetLibrary::ListAssets(PackagePath);

	for(const FString& ListPath:ListPaths)
	{
		//由路径名得到资产名称
		const FString AssetName = FPaths::GetBaseFilename(ListPath);
		//如果与要检查的名称一样就返回false
		if(AssetName.Equals(CheckedName))
		{
			DebugUtil::MessageDialog(CheckedName + TEXT(" is already used"));
			return false;
		}
	}
	
	return true;
}

UMaterial* UQuickMaterialCreateWidget::CreateMaterialAsset(const FString& Name, const FString& Path)
{
	//加载创建材质所需的模块
	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();
	
	//创建资产
	UObject* CreatedObject =
		AssetToolsModule.Get().CreateAsset(Name,Path,UMaterial::StaticClass(),MaterialFactory);
	
	return Cast<UMaterial>(CreatedObject);
}

UMaterialInstanceConstant* UQuickMaterialCreateWidget::CreateMaterialInstance(UMaterial* Material,FString& Name, const FString& Path)
{
	Name.RemoveFromStart(TEXT("M_"));
	Name.InsertAt(0,TEXT("MI_"));
	//加载创建材质所需的模块
	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	UMaterialInstanceConstantFactoryNew* MaterialFactory = NewObject<UMaterialInstanceConstantFactoryNew>();
	
	//创建资产
	UObject* CreatedObject =
		AssetToolsModule.Get().CreateAsset(Name,Path,UMaterialInstanceConstant::StaticClass(),MaterialFactory);
	
	//指定母材质
	if(UMaterialInstanceConstant* CreatedMI = Cast<UMaterialInstanceConstant>(CreatedObject))
	{
		CreatedMI->SetParentEditorOnly(Material);
		//为啥要加这两句
		CreatedMI->PostEditChange();
		Material->PostEditChange();

		return CreatedMI;
	}
	return nullptr;
}

void UQuickMaterialCreateWidget::CreateMaterialNodes(UMaterial* Material,
                                                     UTexture2D* Texture, uint32& PinsCount)
{
	//在参数Material中创建TextureSample节点
	UMaterialExpressionTextureSample* TextureSampleNode =
		NewObject<UMaterialExpressionTextureSample>(Material);

	if(!TextureSampleNode) return;
	/* BaseColor */
	if(!Material->HasBaseColorConnected())    //basecolor插槽没有连接则继续
	{
		if(ConnectBaseColorPin(TextureSampleNode,Texture,Material))
		{
			PinsCount++;
			return;
		}
	}
	/* Normal */
	if(!Material->HasNormalConnected())    
	{
		if(ConnectNormalPin(TextureSampleNode,Texture,Material))
		{
			PinsCount++;
			return;
		}
	}
	/* Roughness */
	//TODO:
	if(!Material->HasRoughnessConnected())    
	{
		if(ConnectRoughnessPin(TextureSampleNode,Texture,Material))
		{
			PinsCount++;
			return;
		}
	}
	/* ao */
	//TODO:
	if(!Material->HasAmbientOcclusionConnected())    
	{
		if(ConnectAOPin(TextureSampleNode,Texture,Material))
		{
			PinsCount++;
			return;
		}
	}
	/* AR */
	//在最后检查AR贴图
	if(!Material->HasRoughnessConnected() && !Material->HasAmbientOcclusionConnected())    
	{
		if(ConnectARPin(TextureSampleNode,Texture,Material))
		{
			PinsCount++;
			return;
		}
	}
	
}

bool UQuickMaterialCreateWidget::ConnectBaseColorPin(
	UMaterialExpressionTextureSample* TextureSampleNode,UTexture2D* Texture, UMaterial* Material)
{
	for(const FString& BaseColorName:BaseColorNameArray)
	{
		if(Texture->GetName().EndsWith(BaseColorName))
		{
			//设置节点采样的纹理
			TextureSampleNode->Texture = Texture;
			//将该节点正式添加到材质中
			Material->GetExpressionCollection().AddExpression(TextureSampleNode);
			//TextureSampleNode节点与BaseColor插槽连接
			Material->GetExpressionInputForProperty(MP_BaseColor)
				->Connect(0,TextureSampleNode);
			//通知引擎Material被修改并且需要更新,对材质进行刷新
			Material->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreateWidget::ConnectNormalPin(UMaterialExpressionTextureSample* TextureSampleNode,
	UTexture2D* Texture, UMaterial* Material)
{
	for(const FString& NormalName:NormalNameArray)
	{
		if(Texture->GetName().EndsWith(NormalName))
		{
			//纹理设置
			Texture->CompressionSettings = TC_Normalmap;
			Texture->PostEditChange();
			
			TextureSampleNode->Texture = Texture;
			TextureSampleNode->SamplerType = SAMPLERTYPE_Normal;
			
			Material->GetExpressionCollection().AddExpression(TextureSampleNode);
			Material->GetExpressionInputForProperty(MP_Normal)
				->Connect(0,TextureSampleNode);
			Material->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 300;
			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreateWidget::ConnectRoughnessPin(UMaterialExpressionTextureSample* TextureSampleNode,
	UTexture2D* Texture, UMaterial* Material)
{
	for(const FString& RoughnessName:RoughnessNameArray)
	{
		if(Texture->GetName().EndsWith(RoughnessName))
		{
			//纹理设置
			Texture->CompressionSettings = TC_Default;
			Texture->SRGB = false;
			Texture->PostEditChange();
			
			TextureSampleNode->Texture = Texture;
			TextureSampleNode->SamplerType = SAMPLERTYPE_LinearColor;
			
			Material->GetExpressionCollection().AddExpression(TextureSampleNode);
			
			Material->GetExpressionInputForProperty(MP_Roughness)
				->Connect(1,TextureSampleNode);
			
			Material->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 300;
			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreateWidget::ConnectAOPin(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* Texture,
	UMaterial* Material)
{
	for(const FString& AOName:AONameArray)
	{
		if(Texture->GetName().EndsWith(AOName))
		{
			//纹理设置
			Texture->CompressionSettings = TC_Default;
			Texture->SRGB = false;
			Texture->PostEditChange();
			
			TextureSampleNode->Texture = Texture;
			TextureSampleNode->SamplerType = SAMPLERTYPE_LinearColor;
			
			Material->GetExpressionCollection().AddExpression(TextureSampleNode);
			
			Material->GetExpressionInputForProperty(MP_AmbientOcclusion)
				->Connect(1,TextureSampleNode);
			
			Material->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 1200;
			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreateWidget::ConnectARPin(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* Texture,
                                              UMaterial* Material)
{
	for(const FString& PackedName:PackedNameArray)
	{
		if(Texture->GetName().EndsWith(PackedName))
		{
			//纹理设置
			Texture->CompressionSettings = TC_Default;
			Texture->SRGB = false;
			Texture->PostEditChange();
			
			TextureSampleNode->Texture = Texture;
			TextureSampleNode->SamplerType = SAMPLERTYPE_LinearColor;
			
			Material->GetExpressionCollection().AddExpression(TextureSampleNode);
			
			Material->GetExpressionInputForProperty(MP_AmbientOcclusion)
				->Connect(1,TextureSampleNode);
			Material->GetExpressionInputForProperty(MP_Roughness)
				->Connect(2,TextureSampleNode);
			
			Material->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 600;
			return true;
		}
	}
	return false;
}
