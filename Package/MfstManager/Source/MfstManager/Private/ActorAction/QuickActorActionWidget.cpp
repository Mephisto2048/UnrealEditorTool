// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorAction/QuickActorActionWidget.h"

#include "Subsystems/EditorActorSubsystem.h"
#include "DebugUtil.h"
void UQuickActorActionWidget::SelectActorsWithName()
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

void UQuickActorActionWidget::DuplicateActors()
{
	if(!GetEditorActorSubSystem())return;

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 DuplicateCount = 0;

	if(SelectedActors.Num()==0)
	{
		DebugUtil::ShowNotify(TEXT("No actor selected"));
		return;
	}
	if(NumberOfDuplicates <=0 || OffsetDist == 0)
	{
		DebugUtil::ShowNotify(TEXT("Invalid input of NumberOfDuplicates or OffsetDist"));
		return;
	}

	for(AActor* SelectedActor:SelectedActors)
	{
		for(int32 i = 0;i<NumberOfDuplicates;i++)
		{
			AActor* DuplicatedActor =
				EditorActorSubsystem->DuplicateActor(SelectedActor,SelectedActor->GetWorld());

			const float DuplicationOffsetDist = (i+1)*OffsetDist;

			switch (Axis)
			{
			case EDuplicationAxis::EDA_XAxis:
				DuplicatedActor->AddActorWorldOffset(FVector(DuplicationOffsetDist,0.0f,0.0f));
				break;
			case EDuplicationAxis::EDA_YAxis:
				DuplicatedActor->AddActorWorldOffset(FVector(0.0f,DuplicationOffsetDist,0.0f));
				break;
			case EDuplicationAxis::EDA_ZAxis:
				DuplicatedActor->AddActorWorldOffset(FVector(0.0f,0.0f,DuplicationOffsetDist));
				break;
			case EDuplicationAxis::EDA_MAX:
				break;
			}

			EditorActorSubsystem->SetActorSelectionState(DuplicatedActor,true);
			DuplicateCount++;
		}
	}
	if(DuplicateCount>0)
	{
		DebugUtil::ShowNotify(FString::FromInt(DuplicateCount)+TEXT(" Actors duplicated successfully"));
	}
}

void UQuickActorActionWidget::RandomizeActorTransform()
{
	const bool ConditionNotSet =
		!RandomActorRotation.bRotYaw &&
			!RandomActorRotation.bRotPitch &&
				!RandomActorRotation.bRotRoll &&
					!bScale &&
						!bOffset;

	if(ConditionNotSet)
	{
		DebugUtil::ShowNotify(TEXT("No variation"));
		return;
	}
	
	if(!GetEditorActorSubSystem())return;

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 RandomizeCount = 0;

	if(SelectedActors.Num()==0)
	{
		DebugUtil::ShowNotify(TEXT("No actor selected"));
		return;
	}
	for(AActor* SelectedActor:SelectedActors)
	{
		if (RandomActorRotation.bRotYaw)
		{
			const float RandomRotYaw = FMath::RandRange(RandomActorRotation.RotYawMin,RandomActorRotation.RotYawMax);

			SelectedActor->AddActorWorldRotation(FRotator(0.0f,RandomRotYaw,0.0f));
		}

		if (RandomActorRotation.bRotPitch)
		{
			const float RandomRotPitch = FMath::RandRange(RandomActorRotation.RotPitchMin,RandomActorRotation.RotPitchMax);

			SelectedActor->AddActorWorldRotation(FRotator(RandomRotPitch,0.0f,0.0f));
		}

		if (RandomActorRotation.bRotRoll)
		{
			const float RandomRotRoll = FMath::RandRange(RandomActorRotation.RotRollMin,RandomActorRotation.RotRollMax);

			SelectedActor->AddActorWorldRotation(FRotator(0.0f,0.0f,RandomRotRoll));
		}

		if(bScale)
		{
			SelectedActor->SetActorScale3D(FVector(FMath::RandRange(ScaleMin,ScaleMax)));
		}

		if (bOffset)
		{
			const float RandomOffset = FMath::RandRange(OffsetMin,OffsetMax);

			SelectedActor->AddActorWorldOffset(FVector(RandomOffset,RandomOffset,0.0f));
		}
		
		RandomizeCount++;
		
	}

	DebugUtil::ShowNotify(FString::FromInt(RandomizeCount)+TEXT("Actors set successfully"));
}

bool UQuickActorActionWidget::GetEditorActorSubSystem()
{
	if(!EditorActorSubsystem)
	{
		EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}
	return EditorActorSubsystem != nullptr;
}
