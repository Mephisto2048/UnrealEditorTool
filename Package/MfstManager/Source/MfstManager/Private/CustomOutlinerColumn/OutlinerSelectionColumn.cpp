// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomOutlinerColumn/OutlinerSelectionColumn.h"

SHeaderRow::FColumn::FArguments FOutlinerSelectionLockColumn::ConstructHeaderRowColumn()
{
	return SHeaderRow::FColumn::FArguments();
}

const TSharedRef<SWidget> FOutlinerSelectionLockColumn::ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem,
	const STableRow<FSceneOutlinerTreeItemPtr>& Row)
{
	return SNullWidget::NullWidget;
}
