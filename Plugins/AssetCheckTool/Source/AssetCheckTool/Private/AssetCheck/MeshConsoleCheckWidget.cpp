// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetCheck/MeshConsoleCheckWidget.h"

bool UMeshConsoleCheckWidget::MeshConsoleCheck(const FString& PackagePath,const int32 Num)
{
	UStaticMesh* StaticMesh = LoadAssetByPackagePath<UStaticMesh>(PackagePath);
	if(!StaticMesh) return false;
	int32 TriangleCount = StaticMesh->GetRenderData()->LODResources[0].GetNumTriangles();

	return TriangleCount < Num;
}

template <typename T>
T* UMeshConsoleCheckWidget::LoadAssetByPackagePath(const FString& PackagePath)
{
	// 创建 FSoftObjectPath 对象
	FSoftObjectPath SoftObjectPath(PackagePath);

	
	T* LoadedAsset = Cast<T>(SoftObjectPath.TryLoad());

	if (LoadedAsset)
	{
		return LoadedAsset;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load %s!"),*PackagePath);
		return nullptr;
	}
}

void UMeshConsoleCheckWidget::MeshConsoleCheckLog(bool flag)
{
	if(!IsRunningCommandlet()) return;

	UE_LOG(LogTemp, Display, TEXT("MeshConsoleCheckLog : %s"),*LexToString(flag));
}

bool UMeshConsoleCheckWidget::IsCommandlet()
{
	return IsRunningCommandlet();
}
