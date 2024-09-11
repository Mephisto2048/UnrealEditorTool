// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


class SAdvanceDeletionTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvanceDeletionTab){}
		SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>,AssetDataToStore)
	SLATE_END_ARGS()
public:
	void Construct(const FArguments& InArgs);
	
private:
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,const TSharedRef<STableViewBase>& OwnerTable);
	TArray<TSharedPtr<FAssetData>> StoredAssetData;
};