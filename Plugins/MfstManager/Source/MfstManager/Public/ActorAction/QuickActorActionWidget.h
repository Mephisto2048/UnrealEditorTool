// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickActorActionWidget.generated.h"

class UEditorActorSubsystem;
/**
 * 
 */
UCLASS()
class MFSTMANAGER_API UQuickActorActionWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void SelectActorsWithName();

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "ActorBatchSelection")
	TEnumAsByte<ESearchCase::Type> SearchCase = ESearchCase::CaseSensitive;
private:
	UPROPERTY()
	UEditorActorSubsystem* EditorActorSubsystem;

	bool GetEditorActorSubSystem();
};
