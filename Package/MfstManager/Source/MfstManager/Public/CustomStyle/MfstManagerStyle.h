// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class FMfstManagerStyle
{
public:
	static void InitIcons();
	static void ShutDown();
private:
	static FName StyleSetName;
	
	static TSharedRef<FSlateStyleSet> CreateSlateStyleSet();
	static TSharedRef<FSlateStyleSet> SlateStyleSet;
public:
	static FName GetStyleSetName(){return StyleSetName;}
};