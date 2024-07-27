// Copyright Epic Games, Inc. All Rights Reserved.

#include "MfstManager.h"

#define LOCTEXT_NAMESPACE "FMfstManagerModule"

void FMfstManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FMfstManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FMfstManagerModule::InitContentBrowserMenuExtension()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMfstManagerModule, MfstManager)