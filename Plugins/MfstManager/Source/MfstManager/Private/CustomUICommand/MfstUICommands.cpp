// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomUICommand/MfstUICommands.h"
//#include "InputCoreTypes.h"
#define LOCTEXT_NAMESPACE "FMfstManagerModule"
void FMfstUICommands::RegisterCommands()
{
	UI_COMMAND(
	LockActorSelection,
	"Lock Actor Selection",
	"Lock actor selection in level,actor can no longer be selected",
	EUserInterfaceActionType::Button,
	FInputChord(EKeys::W,EModifierKey::Alt)
	);
	UI_COMMAND(
	UnlockActorSelection,
	"Unlock Actor Selection",
	"Unlock All actor selection in level",
	EUserInterfaceActionType::Button,
	FInputChord(EKeys::W,EModifierKey::Alt|EModifierKey::Shift)
	);
}
#undef LOCTEXT_NAMESPACE