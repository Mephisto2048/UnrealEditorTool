// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomOutlinerColumn/OutlinerSelectionColumn.h"

SHeaderRow::FColumn::FArguments FOutlinerSelectionLockColumn::ConstructHeaderRowColumn()
{
	SHeaderRow::FColumn::FArguments HeaderRow=
	SHeaderRow::Column(GetColumnID())
	.DefaultTooltip(FText::FromString(TEXT("Press icon to lock actor selection")))
	[
		SNew(SImage)
		.ColorAndOpacity(FSlateColor::UseForeground())
		.Image(FCoreStyle::Get().GetBrush("DefaultBrush"))
	];
	return HeaderRow;
}

const TSharedRef<SWidget> FOutlinerSelectionLockColumn::ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem,
	const STableRow<FSceneOutlinerTreeItemPtr>& Row)
{
	return SNullWidget::NullWidget;
}
