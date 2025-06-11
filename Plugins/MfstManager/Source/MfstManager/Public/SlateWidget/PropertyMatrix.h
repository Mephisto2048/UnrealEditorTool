// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include"Widgets/SCompoundWidget.h"
/**
 * 
 */
class SPropertyMatrixTab : public SCompoundWidget
{
    SLATE_BEGIN_ARGS(SPropertyMatrixTab){}
    	SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>,AssetDataToStore)
    SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	// The table which holds our editable properties.
	TSharedPtr<IPropertyTable> PropertyTable;
};
