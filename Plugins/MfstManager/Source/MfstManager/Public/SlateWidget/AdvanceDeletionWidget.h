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
	/* List */
	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConstructAssetListView();
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,const TSharedRef<STableViewBase>& OwnerTable);
	void RefreshAssetListView();
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> ConstructAssetList;
	
	/* CheckBox */
	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>& AssetData);
	void OnCheckBoxStateChanged(ECheckBoxState NewState,TSharedPtr<FAssetData> AssetData);

	TSharedRef<STextBlock> ConstructTextRowWidget(const FString& String);
	/* Delete */
	TSharedRef<SButton> ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetData);
	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);

	/* 3button */
	FReply OnDeleteAllButtonClicked();
	FReply OnSelectAllButtonClicked();
	FReply OnDeselectAllButtonClicked();

	TArray<TSharedPtr<FAssetData>> AssetsToDelete;
	TArray<TSharedRef<SCheckBox>> CheckBoxesArray;
	/* combobox */
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructComboBox();
	TSharedRef<SWidget> OnGenerateComboContent(TSharedPtr<FString> SourceItem);
	void OnComboSelectionChanged(TSharedPtr<FString> SelectedOption,ESelectInfo::Type InSelectInfo);
	
	
	TArray<TSharedPtr<FString>> ComboBoxSourceItems;
	TSharedPtr<STextBlock> ComboDisplayTextBlock;
	TArray<TSharedPtr<FAssetData>> DisplayedAssetsData;
	/**/
	TArray<TSharedPtr<FAssetData>> StoredAssetData;

	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	FSlateFontInfo BaseTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
};