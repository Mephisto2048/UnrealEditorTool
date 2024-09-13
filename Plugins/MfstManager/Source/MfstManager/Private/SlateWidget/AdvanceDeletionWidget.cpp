// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidget/AdvanceDeletionWidget.h"

#include "DebugUtil.h"
#include "MfstManager.h"


void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	StoredAssetData = InArgs._AssetDataToStore;
	DisplayedAssetsData = StoredAssetData;
	ComboBoxSourceItems.Add(MakeShared<FString>(TEXT("List All Assets")));
	ComboBoxSourceItems.Add(MakeShared<FString>(TEXT("List Selected Assets")));
	
	TitleTextFont.Size = 20.f;
	BaseTextFont.Size = 10.f;
	ChildSlot
	[
		//main
		SNew(SVerticalBox)
		//1.title
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Advance Deletion")))
			.Font(TitleTextFont)
			.Justification(ETextJustify::Left)
		]
		//2.list condition
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		[
			ConstructComboBox()
		]
		//3.asset list
		+SVerticalBox::Slot()
		.VAlign(VAlign_Fill)    //让bar显示出来	
		[
			SNew(SScrollBox)
			+SScrollBox::Slot()
			[
				ConstructAssetListView()
			]
		]
		//4.buttons
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Delete All Seleted")))
				.OnClicked(this,&SAdvanceDeletionTab::OnDeleteAllButtonClicked)
			]
			+SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Select All")))
				.OnClicked(this,&SAdvanceDeletionTab::OnSelectAllButtonClicked)
			]
			+SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("DeSelected All")))
				.OnClicked(this,&SAdvanceDeletionTab::OnDeselectAllButtonClicked)
			]
		]
	];
}

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvanceDeletionTab::ConstructAssetListView()
{
	ConstructAssetList =
		SNew(SListView<TSharedPtr<FAssetData>>)
		.ItemHeight(24.f)
		.ListItemsSource(&DisplayedAssetsData)
		.OnGenerateRow(this,&SAdvanceDeletionTab::OnGenerateRowForList);
	return ConstructAssetList.ToSharedRef();
}

TSharedRef<ITableRow> SAdvanceDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();
	const FString DisplayAssetClass = AssetDataToDisplay->GetClass()->GetName();
	
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget =
		SNew(STableRow<TSharedPtr<FAssetData>>,OwnerTable)
		.Padding(FMargin(0.5f))
		[
			SNew(SHorizontalBox)
			//1.check box
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.FillWidth(.10f)
			[
				ConstructCheckBox(AssetDataToDisplay)
			]
			//2.asset name
			+SHorizontalBox::Slot()
			[
				ConstructTextRowWidget(DisplayAssetName)
			]
			//3.class
			+SHorizontalBox::Slot()
			[
				ConstructTextRowWidget(DisplayAssetClass)
			]
			//4.button
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			[
				ConstructButtonForRowWidget(AssetDataToDisplay)
			]
		];
	return ListViewRowWidget;
}

void SAdvanceDeletionTab::RefreshAssetListView()
{
	AssetsToDelete.Empty();
	if(ConstructAssetList.IsValid())
	{
		ConstructAssetList->RebuildList();
	}
}

TSharedRef<SCheckBox> SAdvanceDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetData)
{
	TSharedRef<SCheckBox> CheckBox =
		SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		.OnCheckStateChanged(this,&SAdvanceDeletionTab::OnCheckBoxStateChanged,AssetData)
		.Visibility(EVisibility::Visible);
	CheckBoxesArray.AddUnique(CheckBox);
	return CheckBox;
}

void SAdvanceDeletionTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch(NewState)
	{
	case ECheckBoxState::Checked :
		AssetsToDelete.AddUnique(AssetData);
		DebugUtil::Print(AssetData->AssetName.ToString() + TEXT(" is checked"));
		break;
	case ECheckBoxState::Unchecked:
		if(AssetsToDelete.Contains(AssetData))
		{
			AssetsToDelete.Remove(AssetData);
		}
		DebugUtil::Print(AssetData->AssetName.ToString() + TEXT(" is unchecked"));
		break;
	default:
		break;
	}
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextRowWidget(const FString& String)
{
	TSharedRef<STextBlock> TextBlock = 
		SNew(STextBlock)
		.Text(FText::FromString(String))
		.Font(BaseTextFont)
		.Justification(ETextJustify::Left)
		;
	
	return TextBlock;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetData)
{
	TSharedRef<SButton> Button =
		SNew(SButton)
		.Text(FText::FromString(TEXT("Delete")))
		.OnClicked(this,&SAdvanceDeletionTab::OnDeleteButtonClicked,AssetData);
	return Button;
}

FReply SAdvanceDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	const bool bAssetDeleted =
		FMfstManagerModule::DeleteSingleAssetForAssetList(*ClickedAssetData.Get());

	if(bAssetDeleted)
	{
		if(StoredAssetData.Contains(ClickedAssetData))
		{
			StoredAssetData.Remove(ClickedAssetData);
		}
		if(DisplayedAssetsData.Contains(ClickedAssetData))
		{
			DisplayedAssetsData.Remove(ClickedAssetData);
		}
		RefreshAssetListView();
	}
	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnDeleteAllButtonClicked()
{
	if(AssetsToDelete.Num()==0)
	{
		DebugUtil::MessageDialog(TEXT("No Asset Selected"));
		return FReply::Handled();
	}
	TArray<FAssetData> AssetDataToDelete;
	for(const TSharedPtr<FAssetData>& Asset:AssetsToDelete)
	{
		AssetDataToDelete.AddUnique(*Asset.Get());
	}
	
	const bool bAssetDeleted =
		FMfstManagerModule::DeleteMulAssetsForAssetList(AssetDataToDelete);

	if(bAssetDeleted)
	{
		for(const TSharedPtr<FAssetData>& Asset:AssetsToDelete)
        	{
				if(StoredAssetData.Contains(Asset))
				{
					StoredAssetData.Remove(Asset);
				}
				if(DisplayedAssetsData.Contains(Asset))
				{
					DisplayedAssetsData.Remove(Asset);
				}
        	}
		RefreshAssetListView();
	}
	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnSelectAllButtonClicked()
{
	for(const TSharedRef<SCheckBox> CheckBox:CheckBoxesArray)
	{
		if(!CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}
	DebugUtil::Print(TEXT("OnSelectAllButtonClicked"));
	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnDeselectAllButtonClicked()
{
	for(const TSharedRef<SCheckBox> CheckBox:CheckBoxesArray)
	{
		if(CheckBox->IsChecked()) CheckBox->ToggleCheckedState();
	}
	DebugUtil::Print(TEXT("OnDeselectAllButtonClicked"));
	return FReply::Handled();
}

TSharedRef<SComboBox<TSharedPtr<FString>>> SAdvanceDeletionTab::ConstructComboBox()
{
	TSharedRef<SComboBox<TSharedPtr<FString>>> ComboBox = 
	SNew(SComboBox<TSharedPtr<FString>>)
	.OptionsSource(&ComboBoxSourceItems)
	.OnGenerateWidget(this,&SAdvanceDeletionTab::OnGenerateComboContent)
	.OnSelectionChanged(this,&SAdvanceDeletionTab::OnComboSelectionChanged)
	[
		SAssignNew(ComboDisplayTextBlock,STextBlock)
		.Text(FText::FromString(TEXT("Option")))
	]
	;
	
	return ComboBox;
}

TSharedRef<SWidget> SAdvanceDeletionTab::OnGenerateComboContent(TSharedPtr<FString> SourceItem)
{
	TSharedRef<STextBlock> TextBlock=
		SNew(STextBlock)
		.Text(FText::FromString(*SourceItem.Get()))
		;
	return TextBlock;
}

void SAdvanceDeletionTab::OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo)
{
	ComboDisplayTextBlock->SetText(FText::FromString(*SelectedOption.Get()));

	if(*SelectedOption.Get() == TEXT("List All Assets"))
	{
		DisplayedAssetsData = StoredAssetData;
		RefreshAssetListView();
	}
	else if (*SelectedOption.Get() == TEXT("List Selected Assets"))
	{
		FMfstManagerModule::ListUnusedAssetsForAssetList(StoredAssetData,DisplayedAssetsData);
		RefreshAssetListView();
	}
}

















