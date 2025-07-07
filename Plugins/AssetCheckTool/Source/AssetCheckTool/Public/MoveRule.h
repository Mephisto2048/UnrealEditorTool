// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MoveRule.generated.h"

USTRUCT(BlueprintType)
struct FRenameAction
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common")
	FString RenameSrcStr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common")
	FString RenameDestStr;
};

USTRUCT(BlueprintType)
struct FMoveRule
{
	GENERATED_BODY()

public:
	// 源目录,需以/结尾
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common")
	FString SourcePath;

	// 目标目录，需以/结尾
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common")
	FString DestPath;

	// 匹配类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common")
	TSubclassOf<UObject> FilterClassType;

	// 是否启用依赖跟随
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	bool IsDependencyFollow;

	// 依赖跟随类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow",meta = (EditCondition = "IsDependencyFollow"))
	TArray<TSubclassOf<UObject>> FollowDependencyTypes;

	// 是否启用依赖跟随（字符串）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow",meta = (EditCondition = "IsDependencyFollow"))
	bool IsDependencyFollowByStr;

	// 依赖跟随前缀
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow",meta = (EditCondition = "IsDependencyFollow"))
	FString FollowPrefix;

	// 依赖跟随仅限于源目录下
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow",meta = (EditCondition = "IsDependencyFollow"))
	bool IsDependencyFollowLocalFolderOnly;

	// 依赖查找层数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow",meta = (EditCondition = "IsDependencyFollow"))
	int DependencyDepth=2;

	// 是否启用引用跟随
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ref")
	bool IsRefFollow;

	// 引用跟随类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ref",meta = (EditCondition = "IsRefFollow"))
	TArray<TSubclassOf<UObject>> FollowRefTypes;

	// 是否启用引用跟随（字符串）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ref",meta = (EditCondition = "IsRefFollow"))
	bool IsRefFollowByStr;

	// 引用跟随前缀
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ref",meta = (EditCondition = "IsRefFollow"))
	FString RefFollowPrefix;

	// 引用跟随仅限于源目录下
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ref",meta = (EditCondition = "IsRefFollow"))
	bool IsRefFollowLocalFolderOnly;

	// 引用查找深度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ref",meta = (EditCondition = "IsRefFollow"))
	int RefDepth=2;

	// 是否递归子文件夹
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common")
	bool IsRecursive;

	// 匹配 PlatformTag
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common")
	FString PlatformTag;

	// 匹配 AreaTag
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common")
	FString AreaTag;

	// 匹配 BiomeTag
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common")
	FString BiomeTag;

	// 移动后重命名
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rename")
	TArray<FRenameAction> RenameConfigs;
};

USTRUCT(BlueprintType)
struct FMoveAction
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MoveAction")
    FString SourcePath;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MoveAction")
    FString DestPath;
};
USTRUCT(BlueprintType)
struct FResaveRule
{
	GENERATED_BODY()

public:
	// FString成员
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YourStruct")
	FString Path;

	// 类类型成员
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YourStruct")
	TSubclassOf<UObject> FilterClassType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YourStruct")
	bool IsRecursive;
};
