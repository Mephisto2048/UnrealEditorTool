// Fill out your copyright notice in the Description page of Project Settings.


#include "FindReplaceTextureWidget.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "DebugUtil.h"
#include "EditorUtilityLibrary.h"
#include "IContentBrowserSingleton.h"
#include "MaterialGraph/MaterialGraph.h"
#include "Materials/MaterialExpressionTextureSample.h"

void UFindReplaceTextureWidget::FindTextures()
{
	//防止对两批material点击find后都被替换的情况
	TextureBaseExpressionArray.Empty();
	CurrentMaterial = nullptr;

	TArray<FAssetData> SelectedAssetsData= UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<UMaterial*> MaterialsArray;
	FString MaterialsPath;

	/* Process Selected Data */
	if(!ProcessSelectedMaterials(SelectedAssetsData,MaterialsArray,MaterialsPath))
	{
		return;
	}

	UMaterial* SelectedMaterial = MaterialsArray[0];
	FMaterialExpressionCollection& MaterialExpressionCollection = SelectedMaterial->GetExpressionCollection();
	TArray<TObjectPtr<UMaterialExpression>> MaterialExpressions = MaterialExpressionCollection.Expressions;
	FString TexturesPath;

	//查找Material的TextureBase节点，添加到数组中
	for(TObjectPtr<UMaterialExpression> Expression : MaterialExpressions)
	{
		TObjectPtr<UMaterialExpressionTextureBase> TextureBaseExpression = Cast<UMaterialExpressionTextureBase>(Expression);
		if(!TextureBaseExpression) continue;

		TObjectPtr<UTexture> Texture = TextureBaseExpression->Texture;
		if(!Texture) continue;

		TextureBaseExpressionArray.AddUnique(TextureBaseExpression);
		ExpressionNameArray.Add(TextureBaseExpression->GetParameterName().ToString());
		TexturesPath += Texture.GetPath() + "\n";
	}

	//查找Material的Function的TextureSample节点，添加到数组中
	TArray<UMaterialFunctionInterface*> DependentFunctions;
	SelectedMaterial->GetDependentFunctions(DependentFunctions);
	for(UMaterialFunctionInterface* DependentFunction : DependentFunctions)
	{
		TConstArrayView<TObjectPtr<UMaterialExpression>> Expressions = DependentFunction->GetExpressions();

		for(TObjectPtr<UMaterialExpression> Expression : Expressions)
		{
			TObjectPtr<UMaterialExpressionTextureBase> TextureBaseExpression = Cast<UMaterialExpressionTextureBase>(Expression);
			if(!TextureBaseExpression) continue;

			TObjectPtr<UTexture> Texture = TextureBaseExpression->Texture;
			if(!Texture) continue;

			TextureBaseExpressionArray.AddUnique(TextureBaseExpression);
			ExpressionNameArray.Add(TextureBaseExpression->GetParameterName().ToString());
			TexturesPath += Texture.GetPath() + "\n";
		}
	}

	/* output */
	MaterialName = SelectedMaterial->GetName();
	MaterialPath = MaterialsPath + "/" + MaterialName;
	TextureList = TexturesPath;
	CurrentMaterial = SelectedMaterial;
}

void UFindReplaceTextureWidget::FindTexturesInBatches(TArray<FString> AssetPaths)
{
	TextureBaseExpressionArray.Empty();
	MaterialName.Empty();
	CurrentMaterialNames.Empty();
	MaterialPath.Empty();
	ExpressionNameArray.Empty();
	MaterialNum = 0;

	for (const FString& AssetPath: AssetPaths)
	{

		TObjectPtr<UMaterial> Material = LoadMaterialByPath(AssetPath);

		if(!Material)continue;

		MaterialNum++;
		MaterialPath = Material.GetPathName();
		MaterialName += FString::FromInt(MaterialNum)+ ". "+ Material.GetName() + "    "  + MaterialPath   + "\n";

		FMaterialExpressionCollection& MaterialExpressionCollection = Material->GetExpressionCollection();
		TArray<TObjectPtr<UMaterialExpression>> MaterialExpressions = MaterialExpressionCollection.Expressions;

		//查找Material的TextureBase节点，添加到数组中
		for(TObjectPtr<UMaterialExpression> Expression : MaterialExpressions)
		{
			TObjectPtr<UMaterialExpressionTextureBase> TextureBaseExpression = Cast<UMaterialExpressionTextureBase>(Expression);
			if(!TextureBaseExpression) continue;

			TObjectPtr<UTexture> Texture = TextureBaseExpression->Texture;
			if(!Texture) continue;

			TextureBaseExpressionArray.AddUnique(TextureBaseExpression);
			ExpressionNameArray.Add(TextureBaseExpression->GetParameterName().ToString());
			CurrentMaterialNames.Add(Material.GetName());
		}

		//查找Material的Function的TextureSample节点，添加到数组中
		TArray<UMaterialFunctionInterface*> DependentFunctions;
		Material->GetDependentFunctions(DependentFunctions);
		if(DependentFunctions.Num()==0) continue;
		for(UMaterialFunctionInterface* DependentFunction : DependentFunctions)
		{
			TConstArrayView<TObjectPtr<UMaterialExpression>> Expressions = DependentFunction->GetExpressions();

			for(TObjectPtr<UMaterialExpression> Expression : Expressions)
			{
				TObjectPtr<UMaterialExpressionTextureBase> TextureBaseExpression = Cast<UMaterialExpressionTextureBase>(Expression);
				if(!TextureBaseExpression) continue;

				TObjectPtr<UTexture> Texture = TextureBaseExpression->Texture;
				if(!Texture) continue;

				TextureBaseExpressionArray.AddUnique(TextureBaseExpression);
				ExpressionNameArray.Add(TextureBaseExpression->GetParameterName().ToString());
				CurrentMaterialNames.Add(DependentFunction->GetName());
			}
		}
	}
}

void UFindReplaceTextureWidget::OpenMaterialEditor()
{
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(CurrentMaterial);

	 const TSharedRef<FGlobalTabmanager>& TabManager = FGlobalTabmanager::Get();
}

TArray<FString> UFindReplaceTextureWidget::GetSelectedPathsInContentBrowser()
{
	// 获取Content Browser模块
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	// 用于存储选中的资产和文件夹的路径
	TArray<FString> SelectedPaths;

	// 获取当前选中的资产
	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

	// 遍历选中的资产，添加每个资产的路径到数组
	for (const FAssetData& Asset : SelectedAssets)
	{

#if ENGINE_MAJOR_VERSION >= 5
		SelectedPaths.Add(Asset.GetObjectPathString());
#else
		SelectedPaths.Add(Asset.ObjectPath.ToString());
#endif
	}

	// 获取当前选中的文件夹
	TArray<FString> SelectedFolders;
	ContentBrowserModule.Get().GetSelectedFolders(SelectedFolders);

	//if(SelectedFolders.Num()==0)DebugUtil::MessageDialog(TEXT("No Folder Selected"));

	// 遍历选中的文件夹，添加每个文件夹的路径到数组
	for (const FString& Folder : SelectedFolders)
	{
		FString ConvertedPath = Folder;

		// 方案一：直接替换
		ConvertedPath.RemoveFromStart("/All");

		// 方案二：使用引擎API（二选一）
		// FPackageName::TryConvertFilenameToLongPackageName(Folder, ConvertedPath);

		SelectedPaths.Add(ConvertedPath);
	}

	TArray<FString> SelectedPathViewFolders;
	ContentBrowserModule.Get().GetSelectedPathViewFolders(SelectedPathViewFolders);

	for (const FString& Folder : SelectedPathViewFolders)
	{
		FString ConvertedPath = Folder;

		// 方案一：直接替换
		ConvertedPath.RemoveFromStart("/All");

		// 方案二：使用引擎API（二选一）
		// FPackageName::TryConvertFilenameToLongPackageName(Folder, ConvertedPath);

		SelectedPaths.Add(ConvertedPath);
	}

	// 返回包含所有选中资产和文件夹路径的数组
	return SelectedPaths;
}

void UFindReplaceTextureWidget::GetFilteredTextures()
{
	TArray<UObject*> SelectedAssets= UEditorUtilityLibrary::GetSelectedAssets();

	PathArray.Empty();

	for (UObject* Asset :SelectedAssets)
	{
		UTexture2D* Texture  = Cast<UTexture2D>(Asset);
		if(!Texture) continue;

		if(Texture->CompressionSettings != TC_Grayscale) continue;

		PathArray += Texture->GetName() + "\n";

	}
}

void UFindReplaceTextureWidget::GetTargetTexturePath(UMaterialExpressionTextureBase* Expression,FString& Outpath)
{
	switch (Expression->SamplerType)
	{
	case SAMPLERTYPE_Color:
		Outpath = TargetColorTexture.GetPathName();
		break;
	case SAMPLERTYPE_LinearColor:
		Outpath = TargetLinearColorTexture.GetPathName();
		break;
	case SAMPLERTYPE_Normal:
		Outpath = TargetNormalTexture.GetPathName();
		break;
	case SAMPLERTYPE_Grayscale:
		Outpath = TargetGrayscaleTexture.GetPathName();
		break;
	case SAMPLERTYPE_LinearGrayscale:
		Outpath = TargetLinearGrayscaleTexture.GetPathName();
		break;
	case SAMPLERTYPE_Alpha:
		Outpath = TargetAlphaTexture.GetPathName();
		break;
	case SAMPLERTYPE_Masks:
		Outpath = TargetMasksTexture.GetPathName();
		break;
	default: break;
	}
}

void UFindReplaceTextureWidget::ReplaceTextures()
{
	/* Process Selected Data */
	if(!ProcessTargetTextures())
	{
		return;
	}
	for(TObjectPtr<UMaterialExpressionTextureBase> Expression:TextureBaseExpressionArray)
	{
		switch (Expression->SamplerType)
		{
		case SAMPLERTYPE_Color:
			Expression->Texture = TargetColorTexture;
			break;
		case SAMPLERTYPE_LinearColor:
			Expression->Texture = TargetLinearColorTexture;
			break;
		case SAMPLERTYPE_Normal:
			Expression->Texture = TargetNormalTexture;
			break;
		case SAMPLERTYPE_Grayscale:
			Expression->Texture = TargetGrayscaleTexture;
			break;
		case SAMPLERTYPE_LinearGrayscale:
			Expression->Texture = TargetLinearGrayscaleTexture;
			break;
		case SAMPLERTYPE_Alpha:
			Expression->Texture = TargetAlphaTexture;
			break;
		case SAMPLERTYPE_Masks:
			Expression->Texture = TargetMasksTexture;
			break;
		}

		Expression->PostEditChange();
	}

	TextureBaseExpressionArray.Empty();

	for(TObjectPtr<UMaterial>Material : CurrentMaterials)
	{
		Material->PostEditChange();
	}

}

void UFindReplaceTextureWidget::ResetTargetTextures()
{
	TargetColorTexture = LoadTextureByPath(TEXT("/Engine/EditorResources/Ai_Spawnpoint"));
	TargetLinearColorTexture = LoadTextureByPath(TEXT("/Engine/ArtTools/RenderToTexture/Textures/127grey"));
	TargetNormalTexture = LoadTextureByPath(TEXT("/Engine/EngineMaterials/FlatNormal"));

	TargetGrayscaleTexture = LoadTextureByPath(TEXT("/Engine/MapTemplates/TestCard"));
	TargetLinearGrayscaleTexture = LoadTextureByPath(TEXT("/Engine/Functions/Engine_MaterialFunctions02/ExampleContent/Textures/LowResBlurredNoise"));
	TargetAlphaTexture = LoadTextureByPath(TEXT("/AssetCheckTool/parts/Resource/T_DefaultDisplacement"));
	TargetMasksTexture = LoadTextureByPath(TEXT("/AssetCheckTool/parts/Resource/T_DefaultMasks"));
}

bool UFindReplaceTextureWidget::ProcessSelectedMaterials(const TArray<FAssetData>& SelectedData,TArray<UMaterial*>& OutMaterialsArray, FString& MaterialsPath)
{
	//如果没选任何资产，就退出
	if(SelectedData.Num() == 0)
	{
		DebugUtil::MessageDialog(TEXT("No Material Selected"));
		return false;
	}

	UObject* SelectedAsset = SelectedData[0].GetAsset();

	UMaterial* SelectedMaterial = Cast<UMaterial>(SelectedAsset);

	if(!SelectedMaterial)
	{
		DebugUtil::MessageDialog(SelectedAsset->GetName() + TEXT(" is not a material"));
		return false;
	}

	OutMaterialsArray.Add(SelectedMaterial);
	MaterialsPath = SelectedData[0].PackagePath.ToString();
	return true;
}

bool UFindReplaceTextureWidget::ProcessTargetTextures()
{
	//如果没选目标纹理，就退出
	if(!TargetColorTexture)
	{
		DebugUtil::MessageDialog(TEXT("No TargetColorTexture Selected"));
		return false;
	}
	if(!TargetLinearColorTexture)
	{
		DebugUtil::MessageDialog(TEXT("No TargetLinearColorTexture Selected"));
		return false;
	}
	if(!TargetNormalTexture)
	{
		DebugUtil::MessageDialog(TEXT("No TargetNormalTexture Selected"));
		return false;
	}
	if(!TargetGrayscaleTexture)
	{
		DebugUtil::MessageDialog(TEXT("No TargetGrayscaleTexture Selected"));
		return false;
	}
	if(!TargetAlphaTexture)
	{
		DebugUtil::MessageDialog(TEXT("No TargetAlphaTexture Selected"));
		return false;
	}
	if(!TargetMasksTexture)
	{
		DebugUtil::MessageDialog(TEXT("No TargetMasksTexture Selected"));
		return false;
	}
	return true;
}

void UFindReplaceTextureWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ResetTargetTextures();
}

TObjectPtr<UTexture2D> UFindReplaceTextureWidget::LoadTextureByPath(const FString& TexturePath)
{
	// 创建 FSoftObjectPath 对象
	FSoftObjectPath SoftObjectPath(TexturePath);

	// 使用 LoadObject 加载纹理
	UTexture2D* LoadedTexture = Cast<UTexture2D>(SoftObjectPath.TryLoad());

	if (LoadedTexture)
	{
		return LoadedTexture;
	}
	else
	{
		DebugUtil::MessageDialog(TEXT("Can not Find ") + TexturePath);
		return nullptr;
	}
}

TObjectPtr<UMaterial> UFindReplaceTextureWidget::LoadMaterialByPath(const FString& Path)
{
	// 创建 FSoftObjectPath 对象
	FSoftObjectPath SoftObjectPath(Path);

	// 使用 LoadObject 加载纹理
	UMaterial* LoadedMaterial = Cast<UMaterial>(SoftObjectPath.TryLoad());

	if (LoadedMaterial)
	{
		return LoadedMaterial;
	}
	else
	{
		DebugUtil::MessageDialog(TEXT("Can not Find ") + Path);
		return nullptr;
	}
}
