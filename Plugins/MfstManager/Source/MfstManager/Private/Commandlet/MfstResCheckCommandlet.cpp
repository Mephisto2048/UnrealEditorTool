// Fill out your copyright notice in the Description page of Project Settings.


#include "Commandlet/MfstResCheckCommandlet.h"

int32 UMfstResCheckCommandlet::Main(const FString& Params)
{
	UE_LOG(LogInit, Warning, TEXT("hello Commandlet"));
	
	return Super::Main(Params);
}
