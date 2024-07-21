// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"


#include "Materials/Material.h"
#include "Materials/MaterialinstanceConstant.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/Texture.h"
#include "Blueprint/UserWidget.h"   //UMG
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraSystem.h"    //Niagara
#include "NiagaraEmitter.h"

#include "QuickAssetActionUtility.generated.h"
/**
 * 
 */
UCLASS()
class MFSTMANAGER_API UQuickAssetActionUtility : public UAssetActionUtility
{
	GENERATED_BODY()
public:
	UFUNCTION(CallInEditor)
	void TestPrint();
	UFUNCTION(CallInEditor)
	void TestMessageDialog();
	UFUNCTION(CallInEditor)
	void TestNotification();

	UFUNCTION(CallInEditor)
	void AutoPrefix();
private:
	TMap<UClass*, FString>PrefixMap =
	{
		{UBlueprint::StaticClass(), TEXT("BP_")},
		{UStaticMesh::StaticClass(),TEXT("SM_")},
		{UMaterial::StaticClass(), TEXT("M_")},
		{UMaterialInstanceConstant::StaticClass(),TEXT("MI_")},
		{UMaterialFunctionInterface::StaticClass(), TEXT("MF_")},
	
		{USoundCue::StaticClass(), TEXT("SC_")},
		{USoundWave::StaticClass(), TEXT("SW_")},
		{UTexture::StaticClass(), TEXT("T_")},
		{UTexture2D::StaticClass(), TEXT("T_")},
		{UUserWidget::StaticClass(), TEXT("WBP_")},
		{USkeletalMeshComponent::StaticClass(), TEXT("SK_")},
		{UNiagaraSystem::StaticClass(), TEXT("NS_")},
		{UNiagaraEmitter::StaticClass(), TEXT("NE_")}
	};
};
