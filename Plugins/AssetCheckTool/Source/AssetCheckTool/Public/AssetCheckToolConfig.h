// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "UObject/UnrealType.h"
#include "ISourceControlModule.h"
#include "SourceControlHelpers.h"
#include "SourceControlOperations.h" // 包含 FCheckOut 的头文件
#include <filesystem>
#include "UObject/UnrealType.h"
#include "AssetCheckToolConfig.generated.h"
/**
 *
 */


USTRUCT(BlueprintType)
struct FArrayStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StringArray")
	TArray<FString> StringArray;
};
USTRUCT(BlueprintType)
struct FMapStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StringMap")
	TMap<FString,FString> StringMap;
};

USTRUCT(BlueprintType)
struct FClassMapStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StringClassMap")
	TMap<FString, UClass*> ClassMap;
};

USTRUCT(BlueprintType)
struct FStringArrayMapStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StringArrayMap")
	TMap<FString,FArrayStruct> StringArrayMap;
};

USTRUCT(BlueprintType)
struct FAssetFilterRuleStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetFilterRule",meta = (DisplayName = "忽略路径"))
	FArrayStruct IgnorePath;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetFilterRule",meta = (DisplayName = "忽略文件名"))
	FArrayStruct IgnoreName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetFilterRule",meta = (DisplayName = "仅处理文件名"))
	FArrayStruct ContainChar;
};



USTRUCT(BlueprintType)
struct FPropertStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "String")
	TMap<FString, FString> StringProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Float")
	TMap<FString, float> FloatProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vector")
	TMap<FString, FVector> VectorProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
	TMap<FString, FColor> ColorProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StringArray")
	TMap<FString, FArrayStruct> StringArrayProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StringMap")
	TMap<FString, FMapStruct> StringMapProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClassMap")
	TMap<FString, FClassMapStruct> ClassMapProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StringArrayMap")
	TMap<FString, FStringArrayMapStruct> StringArrayMapProperty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetFilterRule")
	TMap<FString, FAssetFilterRuleStruct> AssetFilterRuleProperty;

};


USTRUCT(BlueprintType)
//struct ENGINE_API FRenameRuleStruct
struct FAssetNameRuleStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetNameRule", meta = (DisplayName = "命名规则"))
	FString NameRule;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetNameRule", meta = (DisplayName = "路径规则"))
	TArray<FString> PathRule;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetNameRule", meta = (DisplayName = "关键词"))
	FMapStruct RuleKeys;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetNameRule", meta = (DisplayName = "类型过滤"))
	TArray<UClass*> ClassFilter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetNameRule", meta = (DisplayName = "标签过滤"))
	TMap<FString, FArrayStruct>  TagFilter;
};

USTRUCT(BlueprintType)
//struct ENGINE_API FFolderRuleStruct
struct FDirectoryRuleStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DirectoryRule", meta = (DisplayName = "目录规则"))
	TArray<FString> DirectoryRule;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DirectoryRule", meta = (DisplayName = "关键词"))
	FMapStruct RuleKeys;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DirectoryRule", meta = (DisplayName = "变体"))
	FMapStruct VariKeys;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DirectoryRule", meta = (DisplayName = "类型过滤"))
	TArray<UClass*> ClassFilter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DirectoryRule", meta = (DisplayName = "标签过滤"))
	TMap<FString, FArrayStruct>  TagFilter;
};

USTRUCT(BlueprintType)
struct FAssetNameRulesStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetNameRule")
	TMap<FString,FAssetNameRuleStruct> AssetNameRules;
};

USTRUCT(BlueprintType)
struct FDirectoryRulesStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DirectoryRule")
	TMap<FString,FDirectoryRuleStruct> DirectoryRules;
};

USTRUCT(BlueprintType)
struct FDirectoryRulesGroupStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DirectoryRule")
	TMap<FString,FDirectoryRulesStruct> DirectoryRulesGroup;
};


USTRUCT(BlueprintType)
struct FConfigStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ConfigMap")
	//TArray<FPropertStruct> FConfigmap;
	TMap<FString, FPropertStruct>FConfigmap;

};


USTRUCT(BlueprintType)
struct FMoveRuleSimple
{
	GENERATED_BODY()

public:
	// 源目录,需以/结尾
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common")
	FString SourcePath;

	// 目标目录，需以/结尾
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common")
	FString DestPath;
};






UCLASS(config = AssetCheckTool, defaultconfig)
class  UAssetCheckToolConfig : public UObject

{
	GENERATED_BODY()

public:
	UAssetCheckToolConfig(const FObjectInitializer& ObjectInitializer);

	//TMap<FString, FString> PropertyMap;
	// 获取配置对象的实例
	//UFUNCTION(BlueprintCallable, Category = "AssetCheckTool", meta = (DisplayName = "Get Config Instance"))
	//static UAssetCheckToolConfig* GetConfigInstance();

	// 从INI文件中读取配置
	//UFUNCTION(BlueprintCallable, Category = "AssetCheckTool", meta = (DisplayName = "Load Config from INI"))
	//void LoadConfigFromINI();

	// 保存配置到INI文件
	//UFUNCTION(BlueprintCallable, Category = "AssetCheckTool", meta = (DisplayName = "Save Config to INI"))
	//void SaveConfigToINI();


	//参数设置函数
		//工具参数组
	UFUNCTION(BlueprintCallable, Category = "AssetCheckTool", meta = (DisplayName = "Get Property Map"))
	static FConfigStruct GetPropertyMap();

	UFUNCTION(BlueprintCallable, Category = "AssetCheckTool", meta = (DisplayName = "SavelocalPropertyMap"))
	static void SavelocalPropertyMap(const FConfigStruct& NewPropertyMap);

	UFUNCTION(BlueprintCallable, Category = "AssetCheckTool", meta = (DisplayName = "Set Property Map"))
	static void SetPropertyMap(const FConfigStruct& NewPropertyMap);

		//常见型
	UFUNCTION(BlueprintCallable, Category = "AssetCheckTool", meta = (DisplayName = "Set Float Config"))
	static void SetFloatConfig(FString ConfigName, const FString& ConfigValue);

	UFUNCTION(BlueprintCallable, Category = "AssetCheckTool", meta = (DisplayName = "Get Float Config"))
	static float GetFloatConfig(FString ConfigName);

	UFUNCTION(BlueprintCallable, Category = "AssetCheckTool", meta = (DisplayName = "Set String Config"))
	static void SetStringConfig(FString ConfigName, FString ConfigValue);

	UFUNCTION(BlueprintCallable, Category = "AssetCheckTool", meta = (DisplayName = "Get String Config"))
	static FString GetStringConfig(FString ConfigName);

	UFUNCTION(BlueprintCallable, Category = "AssetCheckTool", meta = (DisplayName = "Set Color Config"))
	static void SetColorConfig(FString ConfigName, FColor ConfigValue);

	UFUNCTION(BlueprintCallable, Category = "AssetCheckTool", meta = (DisplayName = "Get Color Config"))
	static FColor GetColorConfig(FString ConfigName);

	UFUNCTION(BlueprintCallable, Category = "AssetCheckTool", meta = (DisplayName = "Set vector Config"))
	static void SetVectorConfig(FString ConfigName, FVector ConfigValue);

	UFUNCTION(BlueprintCallable, Category = "AssetCheckTool", meta = (DisplayName = "Get vector Config"))
	static FVector GetVectorConfig(FString ConfigName);


	UFUNCTION(BlueprintCallable, Category = "AssetCheckTool", meta = (DisplayName = "Set String ArrayConfig"))
	static void SetStringArrayConfig(FString ConfigName, const TArray<FString>& ConfigValue);

	UFUNCTION(BlueprintCallable, Category = "AssetCheckTool", meta = (DisplayName = "Get String ArrayConfig"))
	static TArray<FString> GetStringArrayConfig(FString ConfigName);

	UFUNCTION(BlueprintCallable, Category = "AssetCheckTool", meta = (DisplayName = "CopyProperties"))
	static void CopyProperties(UObject* SourceObject, UObject* TargetObject);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get All Variable Names ", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetAllVariableNamesAndTypes(UObject* Object, TArray<FString>& VariableNames, TArray<FString>& VariableTypes);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Float Variable Names And Values", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetFloatVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<float>& VariableValues);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get String Variable Names And Values",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetStringVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<FString>& VariableValues);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get String Map Variable Names And Values",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetStringMapVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<FMapStruct>& Maps);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Color Variable Names And Values",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetColorVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<FLinearColor>& VariableValues);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Int Variable Names And Values",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetIntVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<int32>& VariableValues);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Vector Variable Names And Values",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetVectorVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<FVector>& VariableValues);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get SoftObjectPath Variable Names And Values",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetSoftObjectPathVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<FSoftObjectPath>& VariableValues);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get FilePath Variable Names And Values",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetFilePathVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<FString>& VariableValues);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get String Array Variable Names And Values",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetStringArrayVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<FArrayStruct>& VariableValues);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetStringClassMapVariableNamesAndValues",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetStringClassMapVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<FClassMapStruct>& Maps);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetStringArrayMapProperty",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetStringArrayMapProperty(UObject* Object, TArray<FString>& VariableNames, TArray<FStringArrayMapStruct>& Maps);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetAssetFilterRulesProperty",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetAssetFilterRulesProperty(UObject* Object, TArray<FString>& VariableNames, TArray<FAssetFilterRuleStruct>& AssetFilterRules);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set String Variable Value",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void SetStringVariableValue(UObject* Object, const FString& VariableName, const FString& Value);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set String Map Variable Value",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void SetStringMapVariableNamesAndValues(UObject* Object, const FString& VariableName, const FMapStruct& MapToSet);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Float Variable Value",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void SetFloatVariableValue(UObject* Object, const FString& VariableName, float Value);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Color Variable Value",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void SetColorVariableValue(UObject* Object, const FString& VariableName, const FLinearColor& Value);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Vector Variable Value",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void SetVectorVariableValue(UObject* Object, const FString& VariableName, const FVector& Value);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set String Array Variable Value",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void SetStringArrayVariableValue(UObject* Object, const FString& VariableName, const TArray<FString>& StringArrayValue);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetStringClassMapVariableNamesAndValues",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void SetStringClassMapVariableNamesAndValues(UObject* Object, const FString& VariableName, const FClassMapStruct& MapToSet);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetStringArrayMapProperty",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void SetStringArrayMapProperty(UObject* Object, const FString& VariableName, const FStringArrayMapStruct& InStringArrayMap);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetAssetFilterRuleMapProperty",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void SetAssetFilterRuleMapProperty(UObject* Object, const FString& VariableName, const FAssetFilterRuleStruct& InAssetFilterRule);


	UFUNCTION(BlueprintCallable, DisplayName = "SetAssetNameRuleConfig",Category="AssetCheckTool")
	static void SetAssetNameRuleConfig(const FAssetNameRulesStruct& InConfigs);

	UFUNCTION(BlueprintCallable, DisplayName = "GetAssetNameRuleConfig",Category = "AssetCheckTool")
	static const FAssetNameRulesStruct& GetAssetNameRuleConfig();

	UFUNCTION(BlueprintCallable, DisplayName = "SetDirectoryRuleConfig",Category="AssetCheckTool")
	static void SetDirectoryRuleConfig(const FDirectoryRulesGroupStruct& InConfigs);

	UFUNCTION(BlueprintCallable, DisplayName = "GetDirectoryRuleConfig",Category = "AssetCheckTool")
	static const FDirectoryRulesGroupStruct& GetDirectoryRuleConfig();

	UFUNCTION(BlueprintCallable, DisplayName = "ExportMoveRuleSimpleConfig",Category = "AssetCheckTool")
	static void ExportMoveRuleSimpleConfig(const TArray<FMoveRuleSimple>& MoveRules);

	//下面是参数
	UPROPERTY(EditAnywhere, Config, category = "通用设置", meta=(DisplayName="资产检测工具入口"))
	FSoftObjectPath AssetCheckToolMainWidget;

	UPROPERTY(EditAnywhere, Config, meta = (DisplayName = "工具默认参数"))
	FConfigStruct PropertyMap;

	UPROPERTY(EditAnywhere, Config, meta = (DisplayName = "资产命名规则"))
	FAssetNameRulesStruct AssetNameRuleConfig;

	UPROPERTY(EditAnywhere, Config, meta = (DisplayName = "目录结构规则"))
	FDirectoryRulesGroupStruct DirectoryRuleConfig;




};