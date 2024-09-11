// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidget/AdvanceDeletionWidget.h"


void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	StoredAssetData = InArgs._AssetDataToStore;
	
	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TitleTextFont.Size = 20.f;
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
		[
			SNew(SHorizontalBox)
		]
		//3.asset list
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SScrollBox)
			+SScrollBox::Slot()
			[
				SNew(SListView<TSharedPtr<FAssetData>>)
				.ItemHeight(24.f)
				.ListItemsSource(&StoredAssetData)
				.OnGenerateRow(this,&SAdvanceDeletionTab::OnGenerateRowForList)
			]
		]
		//4.buttons
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SButton)
		]
	];
}

TSharedRef<ITableRow> SAdvanceDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();
	
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget =
		SNew(STableRow<TSharedPtr<FAssetData>>,OwnerTable)
		[
			SNew(STextBlock)
			.Text(FText::FromString(DisplayAssetName))
		];
	return ListViewRowWidget;
}
