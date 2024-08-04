// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickActorActionWidget.generated.h"

class UEditorActorSubsystem;

UENUM(BlueprintType)
enum class EDuplicationAxis :uint8
{
	EDA_XAxis UMETA(DisplayName = "X"),
	EDA_YAxis UMETA(DisplayName = "Y"),
	EDA_ZAxis UMETA(DisplayName = "Z"),
	EDA_MAX UMETA(DisplayName = "Default Max"),
};

USTRUCT(BlueprintType)
struct FRandomActorRotation
{
	GENERATED_BODY()
	/* Yaw */
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool bRotYaw = false;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta = (EditCondition = "bRotYaw"))
	float RotYawMin = -45.0f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta = (EditCondition = "bRotYaw"))
	float RotYawMax = 45.0f;
	/* Pitch */
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool bRotPitch = false;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta = (EditCondition = "bRotPitch"))
	float RotPitchMin = -45.0f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta = (EditCondition = "bRotPitch"))
	float RotPitchMax = 45.0f;
	/* Roll */
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool bRotRoll = false;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta = (EditCondition = "bRotRoll"))
	float RotRollMin = -45.0f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta = (EditCondition = "bRotRoll"))
	float RotRollMax = 45.0f;
};
/**
 * 
 */
UCLASS()
class MFSTMANAGER_API UQuickActorActionWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
public:
	/* 批量选择 */
	UFUNCTION(BlueprintCallable)
	void SelectActorsWithName();

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "ActorBatchSelection")
	TEnumAsByte<ESearchCase::Type> SearchCase = ESearchCase::CaseSensitive;
	/* 批量复制 */
	UFUNCTION(BlueprintCallable)
	void DuplicateActors();

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "ActorBatchDuplication")
	EDuplicationAxis Axis = EDuplicationAxis::EDA_XAxis;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "ActorBatchDuplication")
	int32 NumberOfDuplicates = 5;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "ActorBatchDuplication")
	float OffsetDist = 300.0f;
	
	/* 随机化 */
	UFUNCTION(BlueprintCallable)
	void RandomizeActorTransform();
	//rotate
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "RandomizeActorTransform")
	FRandomActorRotation RandomActorRotation;
	//scale
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "RandomizeActorTransform")
	bool bScale = false;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "RandomizeActorTransform",meta = (EditCondition = "bScale"))
	float ScaleMin = 0.8f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "RandomizeActorTransform",meta = (EditCondition = "bScale"))
	float ScaleMax = 1.2f;
	//offset
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "RandomizeActorTransform")
	bool bOffset = false;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "RandomizeActorTransform",meta = (EditCondition = "bOffset"))
	float OffsetMin = -50.0f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "RandomizeActorTransform",meta = (EditCondition = "bOffset"))
	float OffsetMax = 50.0f;
private:
	UPROPERTY()
	UEditorActorSubsystem* EditorActorSubsystem;

	bool GetEditorActorSubSystem();
};
