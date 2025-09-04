// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "SlowTaskWidget.generated.h"

/**
 * 
 */
UCLASS()
class MFSTMANAGER_API USlowTaskWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	void PerformSlowTask(int32 TaskNum);

	UFUNCTION(BlueprintCallable)
	FString PropertyToString(UObject* Object, const FString& PropertyName);

	UFUNCTION(BlueprintCallable)
	bool StringToProperty(UObject* Object, const FString& PropertyName, const FString& Value);
};
