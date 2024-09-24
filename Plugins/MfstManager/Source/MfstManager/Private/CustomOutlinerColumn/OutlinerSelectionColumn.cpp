// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomOutlinerColumn/OutlinerSelectionColumn.h"

#include "ActorTreeItem.h"
#include "ISceneOutlinerTreeItem.h"
#include "MfstManager.h"

SHeaderRow::FColumn::FArguments FOutlinerSelectionLockColumn::ConstructHeaderRowColumn()
{
	SHeaderRow::FColumn::FArguments HeaderRow=
	SHeaderRow::Column(GetColumnID())
		.FixedWidth(45.f)
		.HAlignCell(HAlign_Left)
		.VAlignCell(VAlign_Center)
		.HAlignHeader(HAlign_Left)
		.VAlignHeader(VAlign_Center)
	.DefaultTooltip(FText::FromString(TEXT("Press icon to lock actor selection")))
	[
		SNew(SImage)
		.ColorAndOpacity(FSlateColor::UseForeground())
		.Image(FAppStyle::Get().GetBrush("Icons.Warning"))
	];
	return HeaderRow;
}

const TSharedRef<SWidget> FOutlinerSelectionLockColumn::ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem,
	const STableRow<FSceneOutlinerTreeItemPtr>& Row)
{
	FActorTreeItem* ActorTreeItem = TreeItem->CastTo<FActorTreeItem>();

	if(!ActorTreeItem || !ActorTreeItem->IsValid()) return SNullWidget::NullWidget;

	FMfstManagerModule& MfstModule =
	FModuleManager::LoadModuleChecked<FMfstManagerModule>(TEXT("MfstManager"));

	const bool bIsActorSelectionLocked = 
		MfstModule.IsActorSelectionLocked(ActorTreeItem->Actor.Get());
	TSharedRef<SCheckBox> ConstructedRowWidgetCheckBox =
		SNew(SCheckBox)
		.Visibility(EVisibility::Visible)
		.HAlign(HAlign_Center)
		.IsChecked(bIsActorSelectionLocked?ECheckBoxState::Checked : ECheckBoxState::Unchecked)
		.OnCheckStateChanged(this,&FOutlinerSelectionLockColumn::OnRowWidgetCheckStateChanged,ActorTreeItem->Actor);
	return ConstructedRowWidgetCheckBox;
}

void FOutlinerSelectionLockColumn::OnRowWidgetCheckStateChanged(ECheckBoxState NewState,
	TWeakObjectPtr<AActor> CorrespondingActor)
{
	FMfstManagerModule& MfstModule =
		FModuleManager::LoadModuleChecked<FMfstManagerModule>(TEXT("MfstManager"));
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		MfstModule.ProcessLockingForOutliner(CorrespondingActor.Get(),false);
		break;
	case ECheckBoxState::Checked:
		MfstModule.ProcessLockingForOutliner(CorrespondingActor.Get(),true);
		break;
	case ECheckBoxState::Undetermined:
		break;
	}
}










