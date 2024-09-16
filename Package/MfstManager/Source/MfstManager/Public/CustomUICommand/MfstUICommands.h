// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include"Framework/Commands/Commands.h"

class FMfstUICommands : public TCommands<FMfstUICommands>
{
public:
	FMfstUICommands() : TCommands<FMfstUICommands>(
	TEXT("MfstManager"),
	FText::FromString(TEXT("Super Manager UI Command")),
	NAME_None,
	TEXT("MfstManager")
	){}

	virtual void RegisterCommands() override;

	//Command ID
	TSharedPtr<FUICommandInfo> LockActorSelection;
	TSharedPtr<FUICommandInfo> UnlockActorSelection;
};
