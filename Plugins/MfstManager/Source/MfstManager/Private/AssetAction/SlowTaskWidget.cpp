// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetAction/SlowTaskWidget.h"

void USlowTaskWidget::PerformSlowTask(int32 TaskNum)
{
	// 创建并启动慢任务
	FScopedSlowTask SlowTask(TaskNum, FText::FromString(TEXT("PerformSlowTask...")));
	SlowTask.MakeDialog(true);

	// 模拟一个需要长时间执行的操作（例如100步）
	for (int32 i = 0; i < TaskNum; ++i)
	{
		// 更新进度条
		SlowTask.EnterProgressFrame(1, FText::FromString(FString::Printf(TEXT("Processing "))));

		// 模拟一些耗时操作
		FPlatformProcess::Sleep(0.05f); // 每个步骤休眠0.05秒

		// 如果任务被取消，停止操作
		if (SlowTask.ShouldCancel())
		{
			break;
		}
	}
}

FString USlowTaskWidget::PropertyToString(UObject* Object, const FString& PropertyName)
{
	if (!Object)
		return TEXT("");
        
	FProperty* Property = Object->GetClass()->FindPropertyByName(*PropertyName);
	if (!Property)
		return TEXT("");
        
	const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Object);
	FString OutString;
	Property->ExportText_Direct(OutString, ValuePtr, ValuePtr, Object, PPF_None);
    
	return OutString;
}

bool USlowTaskWidget::StringToProperty(UObject* Object, const FString& PropertyName, const FString& Value)
{
	if (!Object)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Object"));
		return false;
	}

	// 查找目标属性
	FProperty* Property = Object->GetClass()->FindPropertyByName(*PropertyName);
	if (!Property)
	{
		UE_LOG(LogTemp, Warning, TEXT("Property '%s' not found"), *PropertyName);
		return false;
	}

	// 获取属性值地址
	void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Object);
	if (!ValuePtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get property value pointer"));
		return false;
	}

	// 使用 ImportText 将字符串导入到属性
	const TCHAR* Buffer = *Value;
	const TCHAR* Result = Property->ImportText_Direct(Buffer, ValuePtr, Object, PPF_None);
    
	if (Result == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to import text to property '%s'"), *PropertyName);
		return false;
	}

	// 标记对象为已修改（用于编辑器）
	if (Object->HasAnyFlags(RF_Transactional))
	{
		Object->Modify();
	}

	return true;
}
