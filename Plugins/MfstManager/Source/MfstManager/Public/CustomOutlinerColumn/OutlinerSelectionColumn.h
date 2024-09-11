// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Editor/SceneOutliner/Public/ISceneOutlinerColumn.h"


class FOutlinerSelectionLockColumn :public ISceneOutlinerColumn
{
public:
	FOutlinerSelectionLockColumn(){}

	virtual FName GetColumnID()override{return FName("SelectionLock");}

	virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn()override;

	virtual const TSharedRef< SWidget > ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row) override;
};
