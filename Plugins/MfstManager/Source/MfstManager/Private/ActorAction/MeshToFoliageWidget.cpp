// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorAction/MeshToFoliageWidget.h"

#include "AssetToolsModule.h"
#include "DebugUtil.h"
#include "Engine/StaticMeshActor.h"
#include "Runtime/Foliage/Public/InstancedFoliageActor.h"
#include "Subsystems/EditorActorSubsystem.h"

void UMeshToFoliageWidget::SelectActorsWithName()
{
	if(!GetEditorActorSubSystem())return;

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 SelectionCount = 0;

	if(SelectedActors.Num()==0)
	{
		DebugUtil::ShowNotify(TEXT("No actor selected"));
		return;
	}
	if(SelectedActors.Num()>1)
	{
		DebugUtil::ShowNotify(TEXT("Only one actor you can select"));
		return;
	}

	FString SelectedActorName = SelectedActors[0]->GetActorLabel();
	
	FString NameToSearch = SelectedActorName;
	//const FString NameToSearch = SelectedActorName.LeftChop(4);    //删除该字符串末尾的4个字符
	// 使用正则表达式匹配末尾的数字
	FString Pattern = TEXT("\\d+$");
	FRegexPattern RegexPattern(Pattern);
	FRegexMatcher Matcher(RegexPattern, SelectedActorName);
	
	// 去掉末尾的数字
	if (Matcher.FindNext())
	{
		int32 StartIndex = Matcher.GetMatchBeginning();
		NameToSearch = SelectedActorName.Left(StartIndex);
	}
	
	TArray<AActor*> AllLevelActors = EditorActorSubsystem->GetAllLevelActors();

	for(AActor* ActorInLevel:AllLevelActors)
	{
		FString ActorLabel = ActorInLevel->GetActorLabel();
		FRegexMatcher Matcher2(RegexPattern, ActorLabel);
		if (Matcher2.FindNext())
		{
			int32 StartIndex = Matcher2.GetMatchBeginning();
			ActorLabel = ActorLabel.Left(StartIndex);
		}
		
		if(ActorLabel.Equals(NameToSearch,SearchCase))
		{
			EditorActorSubsystem->SetActorSelectionState(ActorInLevel,true);
			SelectionCount++;
		}
	}

	if(SelectionCount>0)
	{
		DebugUtil::ShowNotify(FString::FromInt(SelectionCount) + TEXT(" actors selected with name search"));
	}
	else
	{
		DebugUtil::ShowNotify(TEXT("No actors selected with name search"));
	}
}

void UMeshToFoliageWidget::ConvertMeshActorsToFoliage()
{
	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	TArray<AStaticMeshActor*>StaticMeshActors;
	for(AActor* Actor: SelectedActors)
	{
		AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor);
		if(!StaticMeshActor)
		{
			DebugUtil::MessageDialog(TEXT("you select a non-staticmeshactor"));
			return;
		}

		StaticMeshActors.AddUnique(StaticMeshActor);
	}
	ULevel* Level = StaticMeshActors[0]->GetLevel();
	FVector ActorLocation = StaticMeshActors[0]->GetActorLocation();
	
	AInstancedFoliageActor* IFA = AInstancedFoliageActor::Get(GWorld,true,Level,ActorLocation);

	FAssetToolsModule& AssetToolsModule =
	FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	
	//UFoliageType* FoliageType = AssetToolsModule.Get().CreateAsset()
}

bool UMeshToFoliageWidget::GetEditorActorSubSystem()
{
	if(!EditorActorSubsystem)
	{
		EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}
	return EditorActorSubsystem != nullptr;
}
