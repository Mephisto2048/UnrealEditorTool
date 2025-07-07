// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetCheckToolConfig.h"



#include "JsonObjectConverter.h"
#include "Misc/ConfigCacheIni.h"

#define LOCTEXT_NAMESPACE "AssetCheckTool"
TArray<FString> AdditionalParameters;

FString ConfigFilePath = FPaths::ProjectConfigDir() + TEXT("DefaultAssetCheckTool.ini");
//UAssetCheckToolConfig* UAssetCheckToolConfig::ConfigInstance = nullptr;

UAssetCheckToolConfig::UAssetCheckToolConfig(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 在这里设置默认值
	AssetCheckToolMainWidget = FSoftObjectPath(TEXT("/AssetCheckTool/EUW_AssetCheckTools.EUW_AssetCheckTools"));
	PropertyMap ;
}


FConfigStruct UAssetCheckToolConfig::GetPropertyMap()
{
	UAssetCheckToolConfig* Config = GetMutableDefault<UAssetCheckToolConfig>();
	if (Config)
	{
		FProperty* Property = Config->GetClass()->FindPropertyByName("PropertyMap");
		if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			if (StructProperty->Struct == FConfigStruct::StaticStruct())
			{
				FConfigStruct PropertyMap = *StructProperty->ContainerPtrToValuePtr<FConfigStruct>(Config);
				return PropertyMap;
			}
		}
	}

	// 如果未找到合适的属性或者条件不满足，则返回一个默认的FConfigStruct实例
	return FConfigStruct();
}

void UAssetCheckToolConfig::SavelocalPropertyMap(const FConfigStruct& NewPropertyMap)
{
	UAssetCheckToolConfig* Config = GetMutableDefault<UAssetCheckToolConfig>();
	if (Config)
	{
		FProperty* Property = Config->GetClass()->FindPropertyByName("PropertyMap");
		if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			if (StructProperty->Struct == FConfigStruct::StaticStruct())
			{
				// 直接修改配置实例中的值
				*StructProperty->ContainerPtrToValuePtr<FConfigStruct>(Config) = NewPropertyMap;

				// 确保更改被保存到配置文件
				Config->MarkPackageDirty(); // 标记配置已修改，需要保存

				// 保存配置到文件
				Config->SaveConfig();

				// 刷新配置，确保所有更改都被写入文件
				if (GConfig)
				{
					GConfig->Flush(false, Config->GetDefaultConfigFilename());
				}
			}
		}
	}
}

#include "HAL/FileManager.h"

void UAssetCheckToolConfig::SetPropertyMap(const FConfigStruct& NewPropertyMap)
{
	UAssetCheckToolConfig* Config = GetMutableDefault<UAssetCheckToolConfig>();
	if (Config)
	{
		FProperty* Property = Config->GetClass()->FindPropertyByName("PropertyMap");
		if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			if (StructProperty->Struct == FConfigStruct::StaticStruct())
			{
				*StructProperty->ContainerPtrToValuePtr<FConfigStruct>(Config) = NewPropertyMap;
				// Flush the runtime configuration changes to the .ini file

				if (GConfig)
				{
					GConfig->Flush(false);
				}
#if ENGINE_MAJOR_VERSION >= 5
				Config->TryUpdateDefaultConfigFile();
#else
				Config->UpdateDefaultConfigFile();
#endif

				// 获取配置文件路径
				FString LocalConfigFilePath = FPaths::ProjectConfigDir() / TEXT("DefaultAssetCheckTool.ini");

				// Make the file writable directly using filesystem operations
				std::filesystem::path FilePath = TCHAR_TO_UTF8(*LocalConfigFilePath); // Convert FString to std::filesystem::path
				if (std::filesystem::exists(FilePath))
				{
					std::filesystem::permissions(FilePath, std::filesystem::perms::owner_write, std::filesystem::perm_options::add);
					UE_LOG(LogTemp, Log, TEXT("File made writable: %s"), *LocalConfigFilePath);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("File not found: %s"), *LocalConfigFilePath);
				}
			}
		}
	}
}
void UAssetCheckToolConfig::SetFloatConfig(FString ConfigName, const FString& ConfigValue)
{
	UAssetCheckToolConfig* Config = GetMutableDefault<UAssetCheckToolConfig>();
	if (Config)
	{
		FProperty* Property = Config->GetClass()->FindPropertyByName(*ConfigName);
		if (Property)
		{
			if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property))
			{
				float FloatValue = FCString::Atof(*ConfigValue);
				FloatProperty->SetPropertyValue_InContainer(Config, FloatValue);
			}
			else if (FIntProperty* IntProperty = CastField<FIntProperty>(Property))
			{
				int32 IntValue = FCString::Atoi(*ConfigValue);
				IntProperty->SetPropertyValue_InContainer(Config, IntValue);
			}
			else if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
			{
				bool BoolValue = ConfigValue.Equals(TEXT("true"), ESearchCase::IgnoreCase) || ConfigValue.Equals(TEXT("1"));
				BoolProperty->SetPropertyValue_InContainer(Config, BoolValue);
			}
		}
	}
}

float UAssetCheckToolConfig::GetFloatConfig(FString ConfigName)
{
	UAssetCheckToolConfig* Config = GetMutableDefault<UAssetCheckToolConfig>();
	if (Config)
	{
		FProperty* Property = Config->GetClass()->FindPropertyByName(*ConfigName);
		if (Property)
		{
			if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property))
			{
				return FloatProperty->GetPropertyValue_InContainer(Config);
			}
			else if (FIntProperty* IntProperty = CastField<FIntProperty>(Property))
			{
				return static_cast<float>(IntProperty->GetPropertyValue_InContainer(Config));
			}
			else if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
			{
				return BoolProperty->GetPropertyValue_InContainer(Config) ? 1.0f : 0.0f;
			}
		}
	}
	return 0.0f; // 默认返回值，可以根据需要进行修改
}

void UAssetCheckToolConfig::SetStringConfig(FString ConfigName, FString ConfigValue)
{
	UAssetCheckToolConfig* Config = GetMutableDefault<UAssetCheckToolConfig>();
	if (Config)
	{
		FProperty* Property = Config->GetClass()->FindPropertyByName(*ConfigName);
		if (Property)
		{
			if (FStrProperty* StrProperty = CastField<FStrProperty>(Property))
			{
				StrProperty->SetPropertyValue_InContainer(Config, ConfigValue);
			}
			else if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				// 使用 FindObject 获取 UScriptStruct
				static UScriptStruct* FilePathStruct = FindObject<UScriptStruct>(nullptr, TEXT("/Script/CoreUObject.FilePath"));
				static UScriptStruct* DirectoryPathStruct = FindObject<UScriptStruct>(nullptr, TEXT("/Script/CoreUObject.DirectoryPath"));

				if (StructProperty->Struct == FilePathStruct)
				//if (StructProperty->Struct == FFilePath::StaticStruct())
				{
					FFilePath FilePath;
					FilePath.FilePath = ConfigValue;
					*StructProperty->ContainerPtrToValuePtr<FFilePath>(Config) = FilePath;
				}
				//else if (StructProperty->Struct == FDirectoryPath::StaticStruct())
				else if (StructProperty->Struct == DirectoryPathStruct)
				{
					FDirectoryPath DirectoryPath;
					DirectoryPath.Path = ConfigValue;
					*StructProperty->ContainerPtrToValuePtr<FDirectoryPath>(Config) = DirectoryPath;
				}
			}
			else if (FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(Property))
			{
				FSoftObjectPath SoftObjectPath(ConfigValue);
				UObject* Object = SoftObjectPath.TryLoad();
				SoftObjectProperty->SetObjectPropertyValue_InContainer(Config, Object);
			}
			// Flush the runtime configuration changes to the .ini file
			if (GConfig)
			{
				GConfig->Flush(false);
			}

#if ENGINE_MAJOR_VERSION >= 5
			Config->TryUpdateDefaultConfigFile();
#else
			Config->UpdateDefaultConfigFile();
#endif
		}
	}
}

FString UAssetCheckToolConfig::GetStringConfig(FString ConfigName)
{
	UAssetCheckToolConfig* Config = GetMutableDefault<UAssetCheckToolConfig>();
	if (Config)
	{
		FProperty* Property = Config->GetClass()->FindPropertyByName(*ConfigName);
		if (Property)
		{
			if (FStrProperty* StrProperty = CastField<FStrProperty>(Property))
			{
				return StrProperty->GetPropertyValue_InContainer(Config);
			}
			else if (FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(Property))
			{
				FSoftObjectPtr SoftObjectPtr = SoftObjectProperty->GetPropertyValue_InContainer(Config);
				return SoftObjectPtr.ToString();
			}
			else if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				// 使用 FindObject 获取 UScriptStruct
				static UScriptStruct* FilePathStruct = FindObject<UScriptStruct>(nullptr, TEXT("/Script/CoreUObject.FilePath"));
				static UScriptStruct* DirectoryPathStruct = FindObject<UScriptStruct>(nullptr, TEXT("/Script/CoreUObject.DirectoryPath"));

				if (StructProperty->Struct == FilePathStruct)
				//if (StructProperty->Struct == FFilePath::StaticStruct())
				{
					FFilePath FilePath = *StructProperty->ContainerPtrToValuePtr<FFilePath>(Config);
					return FilePath.FilePath;
				}
				else if (StructProperty->Struct == DirectoryPathStruct)
				//else if (StructProperty->Struct == FDirectoryPath::StaticStruct())
				{
					FDirectoryPath DirectoryPath = *StructProperty->ContainerPtrToValuePtr<FDirectoryPath>(Config);
					return DirectoryPath.Path;
				}
			}
			// Add more cases here if you have properties of other types.
		}
	}
	return FString();
}


void UAssetCheckToolConfig::SetColorConfig(FString ConfigName, FColor ConfigValue)
{
	UAssetCheckToolConfig* Config = GetMutableDefault<UAssetCheckToolConfig>();
	if (Config)
	{
		FProperty* Property = Config->GetClass()->FindPropertyByName(*ConfigName);
		if (Property)
		{
			if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				if (StructProperty->Struct == TBaseStructure<FColor>::Get())
				{
					*StructProperty->ContainerPtrToValuePtr<FColor>(Config) = ConfigValue;
				}
			}
		}
	}
}

FColor UAssetCheckToolConfig::GetColorConfig(FString ConfigName)
{
	UAssetCheckToolConfig* Config = GetMutableDefault<UAssetCheckToolConfig>();
	if (Config)
	{
		FProperty* Property = Config->GetClass()->FindPropertyByName(*ConfigName);
		if (Property)
		{
			if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				if (StructProperty->Struct == TBaseStructure<FColor>::Get())
				{
					return *StructProperty->ContainerPtrToValuePtr<FColor>(Config);
				}
			}
		}
	}
	return FColor::Black; // 默认返回值，可以根据需要进行修改
}


void UAssetCheckToolConfig::SetVectorConfig(FString ConfigName, FVector ConfigValue)
{
	UAssetCheckToolConfig* Config = GetMutableDefault<UAssetCheckToolConfig>();
	if (Config)
	{
		FProperty* Property = Config->GetClass()->FindPropertyByName(*ConfigName);
		if (Property)
		{
			if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				if (StructProperty->Struct == TBaseStructure<FVector>::Get())
				{
					*StructProperty->ContainerPtrToValuePtr<FVector>(Config) = ConfigValue;
				}
			}
		}
	}
}

FVector UAssetCheckToolConfig::GetVectorConfig(FString ConfigName)
{
	UAssetCheckToolConfig* Config = GetMutableDefault<UAssetCheckToolConfig>();
	if (Config)
	{
		FProperty* Property = Config->GetClass()->FindPropertyByName(*ConfigName);
		if (Property)
		{
			if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				if (StructProperty->Struct == TBaseStructure<FVector>::Get())
				{
					return *StructProperty->ContainerPtrToValuePtr<FVector>(Config);
				}
			}
		}
	}
	return FVector::ZeroVector; // 默认返回值，可以根据需要进行修改
}


void UAssetCheckToolConfig::SetStringArrayConfig(FString ConfigName, const TArray<FString>& ConfigValue)
{
	UAssetCheckToolConfig* Config = GetMutableDefault<UAssetCheckToolConfig>();
	if (Config)
	{
		FProperty* Property = Config->GetClass()->FindPropertyByName(*ConfigName);
		if (Property)
		{
			if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
			{
				FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(Config));
				ArrayHelper.EmptyAndAddValues(ConfigValue.Num());
				for (int32 Index = 0; Index < ConfigValue.Num(); ++Index)
				{
					FString* StringPtr = (FString*)ArrayHelper.GetRawPtr(Index);
					if (StringPtr)
					{
						*StringPtr = ConfigValue[Index];
					}
				}
			}
		}
	}
}

TArray<FString> UAssetCheckToolConfig::GetStringArrayConfig(FString ConfigName)
{
	UAssetCheckToolConfig* Config = GetMutableDefault<UAssetCheckToolConfig>();
	if (Config)
	{
		FProperty* Property = Config->GetClass()->FindPropertyByName(*ConfigName);
		if (Property)
		{
			if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
			{
				FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(Config));
				TArray<FString> ResultArray;
				ResultArray.Reserve(ArrayHelper.Num());
				for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
				{
					FString* StringPtr = (FString*)ArrayHelper.GetRawPtr(Index);
					if (StringPtr)
					{
						ResultArray.Add(*StringPtr);
					}
				}
				return ResultArray;
			}
		}
	}
	return TArray<FString>(); // 默认返回值，可以根据需要进行修改
}



void UAssetCheckToolConfig::CopyProperties(UObject* SourceObject, UObject* TargetObject)
{
	if (!SourceObject || !TargetObject)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid input: SourceObject or TargetObject is nullptr."));
		return;
	}

	// 遍历源对象的所有属性
	for (TFieldIterator<FProperty> PropIt(SourceObject->GetClass()); PropIt; ++PropIt)
	{
		FProperty* SourceProperty = *PropIt;
		if (SourceProperty && SourceProperty->HasAnyPropertyFlags(CPF_BlueprintVisible))
		{
			// 检查目标对象是否有相同的属性
			FProperty* TargetProperty = FindFProperty<FProperty>(TargetObject->GetClass(), SourceProperty->GetFName());

			if (!TargetProperty)
			{
				// 如果目标对象没有这个属性，我们可以选择跳过或者打印一条消息
				UE_LOG(LogTemp, Warning, TEXT("Property %s does not exist in TargetObject and will be skipped."), *SourceProperty->GetName());
				continue;
			}

			// 获取源和目标的值指针
			void* SourceValue = SourceProperty->ContainerPtrToValuePtr<void>(SourceObject);
			void* TargetValue = TargetProperty->ContainerPtrToValuePtr<void>(TargetObject);

			if (SourceValue && TargetValue)
			{
				// 复制属性值
				TargetProperty->CopyCompleteValue(TargetValue, SourceValue);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to copy property values: Invalid pointers for property %s."), *SourceProperty->GetName());
			}
		}
	}
}

/*
void UAssetCheckToolConfig::GetFloatVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<float>& VariableValues)
{
	if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
	{
		if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
		{
			for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				if (Property->IsA<FFloatProperty>())
				{
					FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property);
					void* ValuePtr = FloatProperty->ContainerPtrToValuePtr<void>(Object);
					float VariableValue = FloatProperty->GetPropertyValue(ValuePtr);
					VariableNames.Add(Property->GetName());
					VariableValues.Add(VariableValue);
				}
				else if (Property->IsA<FIntProperty>())
				{
					FIntProperty* IntProperty = CastField<FIntProperty>(Property);
					void* ValuePtr = IntProperty->ContainerPtrToValuePtr<void>(Object);
					int32 VariableValue = IntProperty->GetPropertyValue(ValuePtr);
					VariableNames.Add(Property->GetName());
					VariableValues.Add(static_cast<float>(VariableValue));
				}
				else if (Property->IsA<FBoolProperty>())
				{
					FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property);
					void* ValuePtr = BoolProperty->ContainerPtrToValuePtr<void>(Object);
					bool VariableValue = BoolProperty->GetPropertyValue(ValuePtr);
					VariableNames.Add(Property->GetName());
					VariableValues.Add(VariableValue ? 1.0f : 0.0f);
				}
			}
		}
	}
}
*/

void UAssetCheckToolConfig::GetAllVariableNamesAndTypes(UObject* Object, TArray<FString>& VariableNames, TArray<FString>& VariableTypes)
{
	if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
	{
		if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
		{
			for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				FString VariableName = Property->GetName();
				FString VariableType = Property->GetCPPType();
				VariableNames.Add(VariableName);
				VariableTypes.Add(VariableType);
			}
		}
	}
}



void UAssetCheckToolConfig::GetFloatVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<float>& VariableValues)
{
	if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
	{
		if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
		{
			for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				if (Property->IsA<FFloatProperty>())
				{
					FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property);
					void* ValuePtr = FloatProperty->ContainerPtrToValuePtr<void>(Object);
					float VariableValue = FloatProperty->GetPropertyValue(ValuePtr);
					VariableNames.Add(Property->GetName());
					VariableValues.Add(VariableValue);
				}
				else if (Property->IsA<FIntProperty>())
				{
					FIntProperty* IntProperty = CastField<FIntProperty>(Property);
					void* ValuePtr = IntProperty->ContainerPtrToValuePtr<void>(Object);
					int32 VariableValue = IntProperty->GetPropertyValue(ValuePtr);
					VariableNames.Add(Property->GetName());
					VariableValues.Add(static_cast<float>(VariableValue));
				}
				else if (Property->IsA<FBoolProperty>())
				{
					FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property);
					void* ValuePtr = BoolProperty->ContainerPtrToValuePtr<void>(Object);
					bool VariableValue = BoolProperty->GetPropertyValue(ValuePtr);
					VariableNames.Add(Property->GetName());
					VariableValues.Add(VariableValue ? 1.0f : 0.0f);
				}
			}
		}
	}
}

#include "BlueprintCompilationManager.h"
#include "UObject/MetaData.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_Event.h"
#include "K2Node_CallParentFunction.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_Self.h"
#include "K2Node_VariableGet.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "K2Node_CallFunction.h"

#include "EdGraphUtilities.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "KismetCastingUtils.h"
#include "KismetCompiler.h"
#include "Net/Core/PushModel/PushModelMacros.h"

void UAssetCheckToolConfig::GetStringVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<FString>& VariableValues)
{
    if (!Object)
    {
        return;
    }

    // 获取对象的类
    UClass* ObjectClass = Object->GetClass();
    if (!ObjectClass)
    {
        return;
    }

    // 检查是否是蓝图生成的类
    UBlueprint* Blueprint = Cast<UBlueprint>(ObjectClass->ClassGeneratedBy);
    if (!Blueprint)
    {
        return;
    }

    // 获取蓝图生成的类
    UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass);
    if (!GeneratedClass)
    {
        return;
    }

    // 获取 FFilePath 和 FDirectoryPath 的 UScriptStruct
#if ENGINE_MAJOR_VERSION >= 5
	UScriptStruct* FilePathStruct = FindObject<UScriptStruct>(nullptr, TEXT("/Script/CoreUObject.FilePath"));
	UScriptStruct* DirectoryPathStruct = FindObject<UScriptStruct>(nullptr, TEXT("/Script/CoreUObject.DirectoryPath"));

#else
	UScriptStruct* FilePathStruct = FindObject<UScriptStruct>(ANY_PACKAGE, TEXT("FFilePath"));
	UScriptStruct* DirectoryPathStruct = FindObject<UScriptStruct>(ANY_PACKAGE, TEXT("FDirectoryPath"));
#endif


    // 遍历类的属性
    for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
    {
        FProperty* Property = *PropertyIt;
        if (!Property)
        {
            continue;
        }

        FString VariableValue;
        bool bShouldOutput = false;

        // 处理字符串属性
        if (Property->IsA<FStrProperty>())
        {
            FStrProperty* StringProperty = CastField<FStrProperty>(Property);
            if (StringProperty)
            {
                void* ValuePtr = StringProperty->ContainerPtrToValuePtr<void>(Object);
                VariableValue = StringProperty->GetPropertyValue(ValuePtr);
                bShouldOutput = true;
            }
        }
        // 处理结构体属性
        else if (Property->IsA<FStructProperty>())
        {
            FStructProperty* StructProperty = CastField<FStructProperty>(Property);
            if (StructProperty)
            {
                // 处理 FFilePath 类型
                if (FilePathStruct && StructProperty->Struct == FilePathStruct)
                {
                    void* ValuePtr = StructProperty->ContainerPtrToValuePtr<void>(Object);
                    FFilePath* FilePathValue = static_cast<FFilePath*>(ValuePtr);
                    VariableValue = FilePathValue->FilePath;
                    bShouldOutput = true;
                }
                // 处理 FDirectoryPath 类型
                else if (DirectoryPathStruct && StructProperty->Struct == DirectoryPathStruct)
                {
                    void* ValuePtr = StructProperty->ContainerPtrToValuePtr<void>(Object);
                    FDirectoryPath* DirectoryPathValue = static_cast<FDirectoryPath*>(ValuePtr);
                    VariableValue = DirectoryPathValue->Path;
                    bShouldOutput = true;
                }
            }
        }
        // 处理对象属性
        else if (Property->IsA<FObjectPropertyBase>())
        {
            FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property);
            if (ObjectProperty)
            {
                void* ValuePtr = ObjectProperty->ContainerPtrToValuePtr<void>(Object);
                UObject* ObjectValue = ObjectProperty->GetObjectPropertyValue(ValuePtr);
                if (ObjectValue)
                {
                    VariableValue = ObjectValue->GetPathName();
                    bShouldOutput = true;
                }
            }
        }

        // 如果需要输出，则将属性名和值添加到数组中
        if (bShouldOutput)
        {
            VariableNames.Add(Property->GetName());
            VariableValues.Add(VariableValue);
        }
    }
}

void UAssetCheckToolConfig::GetStringMapVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<FMapStruct>& Maps)
{
	if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
	{
		if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
		{
			for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{
				bool output = false;
				FProperty* Property = *PropertyIt;
				FMapStruct CurrentMap;

				if (Property->IsA<FMapProperty>())
				{
					FMapProperty* MapProperty = CastField<FMapProperty>(Property);
					void* ValuePtr = MapProperty->ContainerPtrToValuePtr<void>(Object);
					if (MapProperty->KeyProp->IsA<FStrProperty>() && MapProperty->ValueProp->IsA<FStrProperty>())
					{
						FScriptMapHelper MapHelper(MapProperty, ValuePtr);
						for (int32 i = 0; i < MapHelper.Num(); ++i)
						{
							FStrProperty* KeyProp = CastField<FStrProperty>(MapProperty->KeyProp);
							FStrProperty* ValueProp = CastField<FStrProperty>(MapProperty->ValueProp);

							FString KeyString = KeyProp->GetPropertyValue(MapHelper.GetKeyPtr(i));
							FString ValueString = ValueProp->GetPropertyValue(MapHelper.GetValuePtr(i));

							CurrentMap.StringMap.Add(KeyString, ValueString);
						}
						output = true;
					}
				}
				if(output == true)
				{
					VariableNames.Add(Property->GetName());
					Maps.Add(CurrentMap);
				}
			}
		}
	}
}



void UAssetCheckToolConfig::GetColorVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<FLinearColor>& VariableValues)
{
	if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
	{
		if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
		{
			for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				if (Property->IsA<FStructProperty>() && Property->GetCPPType().Contains("FColor"))
				{
					FStructProperty* StructProperty = CastField<FStructProperty>(Property);
					if (StructProperty->Struct == TBaseStructure<FColor>::Get())
					{
						void* ValuePtr = StructProperty->ContainerPtrToValuePtr<void>(Object);
						FColor* VariableColor = StructProperty->ContainerPtrToValuePtr<FColor>(ValuePtr);
						FLinearColor VariableValue(*VariableColor);
						VariableNames.Add(Property->GetName());
						VariableValues.Add(VariableValue);
					}
				}
			}
		}
	}
}

void UAssetCheckToolConfig::GetIntVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<int32>& VariableValues)
{
	if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
	{
		if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
		{
			for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				if (Property->IsA<FIntProperty>())
				{
					FIntProperty* IntProperty = CastField<FIntProperty>(Property);
					void* ValuePtr = IntProperty->ContainerPtrToValuePtr<void>(Object);
					int32 VariableValue = IntProperty->GetPropertyValue(ValuePtr);
					VariableNames.Add(Property->GetName());
					VariableValues.Add(VariableValue);
				}
			}
		}
	}
}

void UAssetCheckToolConfig::GetVectorVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<FVector>& VariableValues)
{
	if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
	{
		if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
		{
			for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				if (Property->IsA<FStructProperty>() && (Property->GetCPPType().Contains("FVector") || Property->GetCPPType().Contains("FVector2D")))
				{
					FStructProperty* VectorProperty = CastField<FStructProperty>(Property);
					if (VectorProperty->Struct == TBaseStructure<FVector>::Get())
					{
						void* ValuePtr = VectorProperty->ContainerPtrToValuePtr<void>(Object);
						const FVector* VariableVector = static_cast<FVector*>(ValuePtr);
						FVector VariableValue = *VariableVector;
						VariableNames.Add(Property->GetName());
						VariableValues.Add(VariableValue);
					}
					else if (VectorProperty->Struct == TBaseStructure<FVector2D>::Get())
					{
						void* ValuePtr = VectorProperty->ContainerPtrToValuePtr<void>(Object);
						const FVector2D* VariableVector2D = static_cast<FVector2D*>(ValuePtr);
						FVector VariableValue = FVector(VariableVector2D->X, VariableVector2D->Y, 0);
						VariableNames.Add(Property->GetName());
						VariableValues.Add(VariableValue);
					}
				}
			}
		}
	}
}

void UAssetCheckToolConfig::GetSoftObjectPathVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<FSoftObjectPath>& VariableValues)
{
	if (const UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
	{
		if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
		{
			for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{

				if (FProperty* Property = *PropertyIt; Property->IsA<FSoftObjectProperty>())
				{
					const FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(Property);
					const void* ValuePtr = SoftObjectProperty->ContainerPtrToValuePtr<void>(Object);
					//FSoftObjectPtr* SoftObjectPtr = static_cast<FSoftObjectPtr*>(ValuePtr);

					if (const UObject* ResolvedObject = SoftObjectProperty->GetObjectPropertyValue(ValuePtr))
					{
						FSoftObjectPath VariableValue(ResolvedObject);
						VariableNames.Add(Property->GetName());
						VariableValues.Add(VariableValue);
					}
				}
			}
		}
	}
}

void UAssetCheckToolConfig::GetFilePathVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<FString>& VariableValues)
#if ENGINE_MAJOR_VERSION >= 5
{//ue5
	if (!Object)
	{
		return;
	}

	// 获取对象的类
	UClass* ObjectClass = Object->GetClass();
	if (!ObjectClass)
	{
		return;
	}

	// 检查是否是蓝图生成的类
	UBlueprint* Blueprint = Cast<UBlueprint>(ObjectClass->ClassGeneratedBy);
	if (!Blueprint)
	{
		return;
	}

	// 获取蓝图生成的类
	UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass);
	if (!GeneratedClass)
	{
		return;
	}

	// 动态获取 FFilePath 的 UScriptStruct
#if ENGINE_MAJOR_VERSION >= 5
	UScriptStruct* FilePathStruct = FindObject<UScriptStruct>(nullptr, TEXT("/Script/CoreUObject.FilePath"));
#else
	UScriptStruct* FilePathStruct = FindObject<UScriptStruct>(ANY_PACKAGE, TEXT("FFilePath"));
#endif


	if (!FilePathStruct)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find FFilePath UScriptStruct."));
		return;
	}

	// 遍历类的属性
	for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
	{
		FProperty* Property = *PropertyIt;
		if (!Property)
		{
			continue;
		}

		// 检查属性是否为结构体属性
		if (Property->IsA<FStructProperty>())
		{
			FStructProperty* StructProperty = CastField<FStructProperty>(Property);
			if (StructProperty && StructProperty->Struct == FilePathStruct)
			{
				// 获取属性值
				void* ValuePtr = StructProperty->ContainerPtrToValuePtr<void>(Object);
				FFilePath* FilePathValue = static_cast<FFilePath*>(ValuePtr);
				FString VariableValue = FilePathValue->FilePath;

				// 将属性名和值添加到输出数组中
				VariableNames.Add(Property->GetName());
				VariableValues.Add(VariableValue);
			}
		}
	}
}
#else
{ //ue4
	if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
	{
		if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
		{
			for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				if (Property->IsA<FProperty>() && Property->GetClass() == FStructProperty::StaticClass())
				{
					FStructProperty* StructProperty = CastField<FStructProperty>(Property);
					if (StructProperty->Struct == FFilePath::StaticStruct())
					{
						void* ValuePtr = StructProperty->ContainerPtrToValuePtr<void>(Object);
						FFilePath* FilePathValue = static_cast<FFilePath*>(ValuePtr);
						FString VariableValue = FilePathValue->FilePath;
						VariableNames.Add(Property->GetName());
						VariableValues.Add(VariableValue);
					}
				}
			}
		}
	}
}
#endif


void UAssetCheckToolConfig::GetStringArrayVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<FArrayStruct>& VariableValues)
{
	if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
	{
		if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
		{
			for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;

				// Check for array of strings
				if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
				{
					if (FStrProperty* InnerStrProperty = CastField<FStrProperty>(ArrayProperty->Inner))
					{
						void* ValuePtr = ArrayProperty->ContainerPtrToValuePtr<void>(Object);
						TArray<FString>* StringArrayValue = static_cast<TArray<FString>*>(ValuePtr);
						VariableNames.Add(Property->GetName());
						FArrayStruct stringArray;
						stringArray.StringArray = *StringArrayValue;
						VariableValues.Add(stringArray);
					}
					// Check for array of FDirectoryPath
					else if (FStructProperty* InnerStructProperty = CastField<FStructProperty>(ArrayProperty->Inner))
					{
						if (InnerStructProperty->Struct->GetFName() == FName("DirectoryPath"))
						{
							void* ValuePtr = ArrayProperty->ContainerPtrToValuePtr<void>(Object);
							TArray<FDirectoryPath>* DirectoryPathArrayValue = static_cast<TArray<FDirectoryPath>*>(ValuePtr);
							VariableNames.Add(Property->GetName());
							FArrayStruct directoryPathArray;
							for (const FDirectoryPath& DirectoryPath : *DirectoryPathArrayValue)
							{
								directoryPathArray.StringArray.Add(DirectoryPath.Path);
							}
							VariableValues.Add(directoryPathArray);
						}
					}
				}
			}
		}
	}
}


void UAssetCheckToolConfig::GetStringClassMapVariableNamesAndValues(UObject* Object, TArray<FString>& VariableNames, TArray<FClassMapStruct>& Maps)
{
   if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
    {
        if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
        {
            for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
            {
                bool output = false;
                FProperty* Property = *PropertyIt;
                FClassMapStruct CurrentMap;

                if (Property->IsA<FMapProperty>())
                {
                    FMapProperty* MapProperty = CastField<FMapProperty>(Property);
                    void* ValuePtr = MapProperty->ContainerPtrToValuePtr<void>(Object);

                    // 检查 Key 是 FString 类型，Value 是 UClass* 类型
                    if (MapProperty->KeyProp->IsA<FStrProperty>() && MapProperty->ValueProp->IsA<FClassProperty>())
                    {
                        FScriptMapHelper MapHelper(MapProperty, ValuePtr);
                        for (int32 i = 0; i < MapHelper.Num(); ++i)
                        {
                            FStrProperty* KeyProp = CastField<FStrProperty>(MapProperty->KeyProp);
                            FClassProperty* ValueProp = CastField<FClassProperty>(MapProperty->ValueProp);

                            FString KeyString = KeyProp->GetPropertyValue(MapHelper.GetKeyPtr(i));
                            UClass* ValueClass = Cast<UClass>(ValueProp->GetObjectPropertyValue(MapHelper.GetValuePtr(i)));

                            if (ValueClass)
                            {
                                CurrentMap.ClassMap.Add(KeyString, ValueClass);
                            }
                        }
                        output = true;
                    }
                }

                if (output)
                {
                    VariableNames.Add(Property->GetName());
                    Maps.Add(CurrentMap);
                }
            }
        }
    }
}



void UAssetCheckToolConfig::GetStringArrayMapProperty(UObject* Object, TArray<FString>& VariableNames, TArray<FStringArrayMapStruct>& Maps)
{
    // 清空输出数组
    VariableNames.Empty();
    Maps.Empty();

    // 检查输入对象是否有效
    if (!Object)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid Object!"));
        return;
    }

    if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
    {
        if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
        {
            // 遍历所有属性
            for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
            {
                FProperty* Property = *PropertyIt;

                // 检查属性是否为 TMap<FString, FArrayStruct>
                if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
                {
                    // 检查 Key 是 FString 类型，Value 是 FArrayStruct 类型
                    if (MapProperty->KeyProp->IsA<FStrProperty>() && MapProperty->ValueProp->IsA<FStructProperty>())
                    {
                        FStructProperty* ValueProp = CastField<FStructProperty>(MapProperty->ValueProp);
                        if (ValueProp->Struct == FArrayStruct::StaticStruct())
                        {
                            // 获取属性值
                            void* ValuePtr = MapProperty->ContainerPtrToValuePtr<void>(Object);
                            FScriptMapHelper MapHelper(MapProperty, ValuePtr);

                            // 创建一个 FStringArrayMapStruct 来存储当前映射的值
                            FStringArrayMapStruct CurrentMap;

                            // 遍历映射
                            for (int32 i = 0; i < MapHelper.Num(); ++i)
                            {
                                // 获取 Key
                                FString KeyString = CastField<FStrProperty>(MapProperty->KeyProp)->GetPropertyValue(MapHelper.GetKeyPtr(i));

                                // 获取 Value
                                FArrayStruct* ValueStruct = reinterpret_cast<FArrayStruct*>(MapHelper.GetValuePtr(i));
                                if (ValueStruct)
                                {
                                    // 将 Key 和 Value 添加到 CurrentMap 中
                                    CurrentMap.StringArrayMap.Add(KeyString, *ValueStruct);
                                }
                            }

                            // 将属性名称和 CurrentMap 添加到输出数组中
                            VariableNames.Add(Property->GetName());
                            Maps.Add(CurrentMap);
                        }
                    }
                }
            }
        }
    }
}

void UAssetCheckToolConfig::GetAssetFilterRulesProperty(UObject* Object, TArray<FString>& VariableNames, TArray<FAssetFilterRuleStruct>& AssetFilterRules)
{
	// Clear the output arrays
	VariableNames.Empty();
	AssetFilterRules.Empty();

	// Check if the input object is valid
	if (!Object)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Object!"));
		return;
	}

	if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
	{
		if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
		{
			// Iterate over all properties
			for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;

				// Check if the property is of type FAssetFilterRuleStruct
				if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
				{
					if (StructProperty->Struct == FAssetFilterRuleStruct::StaticStruct())
					{
						// Get the property value
						FAssetFilterRuleStruct* ValuePtr = StructProperty->ContainerPtrToValuePtr<FAssetFilterRuleStruct>(Object);

						if (ValuePtr)
						{
							// Add the property name and value to the output arrays
							VariableNames.Add(Property->GetName());
							AssetFilterRules.Add(*ValuePtr);
						}
					}
				}
			}
		}
	}
}


void UAssetCheckToolConfig::SetStringVariableValue(UObject* Object, const FString& VariableName, const FString& Value)
#if ENGINE_MAJOR_VERSION >= 5
{//ue5
    if (!Object)
    {
        return;
    }

    // 获取对象的类
    UClass* ObjectClass = Object->GetClass();
    if (!ObjectClass)
    {
        return;
    }

    // 检查是否是蓝图生成的类
    UBlueprint* Blueprint = Cast<UBlueprint>(ObjectClass->ClassGeneratedBy);
    if (!Blueprint)
    {
        return;
    }

    // 获取蓝图生成的类
    UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass);
    if (!GeneratedClass)
    {
        return;
    }

    // 动态获取 FFilePath 和 FDirectoryPath 的 UScriptStruct
   // UScriptStruct* FilePathStruct = FindObject<UScriptStruct>(ANY_PACKAGE, TEXT("FFilePath"));
   // UScriptStruct* DirectoryPathStruct = FindObject<UScriptStruct>(ANY_PACKAGE, TEXT("FDirectoryPath"));
	UScriptStruct* FilePathStruct = FindObject<UScriptStruct>(nullptr, TEXT("/Script/CoreUObject.FilePath"));
	UScriptStruct* DirectoryPathStruct = FindObject<UScriptStruct>(nullptr, TEXT("/Script/CoreUObject.DirectoryPath"));

	if (FilePathStruct == nullptr || DirectoryPathStruct == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize one or both script structs."));
	}
    // 遍历类的属性
    for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
    {
        FProperty* Property = *PropertyIt;
        if (!Property)
        {
            continue;
        }

        // 检查属性名是否匹配
        if (Property->GetName() == VariableName)
        {
            // 处理字符串属性
            if (Property->IsA<FStrProperty>())
            {
                FStrProperty* StringProperty = CastField<FStrProperty>(Property);
                if (StringProperty)
                {
                    void* ValuePtr = StringProperty->ContainerPtrToValuePtr<void>(Object);
                    FString* StringValue = static_cast<FString*>(ValuePtr);
                    *StringValue = Value;
                }
            }
            // 处理结构体属性
            else if (Property->IsA<FStructProperty>())
            {
                FStructProperty* StructProperty = CastField<FStructProperty>(Property);
                if (StructProperty)
                {
                    // 处理 FFilePath 类型
                    if (FilePathStruct && StructProperty->Struct == FilePathStruct)
                    {
                        void* ValuePtr = StructProperty->ContainerPtrToValuePtr<void>(Object);
                        FFilePath* FilePathValue = static_cast<FFilePath*>(ValuePtr);
                        FilePathValue->FilePath = Value;
                    }
                    // 处理 FDirectoryPath 类型
                    else if (DirectoryPathStruct && StructProperty->Struct == DirectoryPathStruct)
                    {
                        void* ValuePtr = StructProperty->ContainerPtrToValuePtr<void>(Object);
                        FDirectoryPath* DirectoryPathValue = static_cast<FDirectoryPath*>(ValuePtr);
                        DirectoryPathValue->Path = Value;
                    }
                }
            }
            // 处理对象属性
            else if (Property->IsA<FObjectPropertyBase>())
            {
                FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property);
                if (ObjectProperty)
                {
                    void* ValuePtr = ObjectProperty->ContainerPtrToValuePtr<void>(Object);
                    UObject* LoadedObject = StaticLoadObject(UObject::StaticClass(), nullptr, *Value);
                    if (LoadedObject)
                    {
                        ObjectProperty->SetObjectPropertyValue(ValuePtr, LoadedObject);
                    }
                }
            }

            break;
        }
    }
}
#else
{//ue4
    if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
    {
        if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
        {
            for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
            {
                FProperty* Property = *PropertyIt;
                if (Property->GetName() == VariableName)
                {
                    if (Property->IsA<FStrProperty>())
                    {
                        FStrProperty* StringProperty = CastField<FStrProperty>(Property);
                        void* ValuePtr = StringProperty->ContainerPtrToValuePtr<void>(Object);
                        FString* StringValue = static_cast<FString*>(ValuePtr);
                        *StringValue = Value;
                    }
                    else if (Property->IsA<FStructProperty>())
                    {
                        FStructProperty* StructProperty = CastField<FStructProperty>(Property);
                        if (StructProperty->Struct == FFilePath::StaticStruct())
                        {
                            void* ValuePtr = StructProperty->ContainerPtrToValuePtr<void>(Object);
                            FFilePath* FilePathValue = static_cast<FFilePath*>(ValuePtr);
                            FilePathValue->FilePath = Value;
                        }
                        else if (StructProperty->Struct == FDirectoryPath::StaticStruct())
                        {
                            void* ValuePtr = StructProperty->ContainerPtrToValuePtr<void>(Object);
                            FDirectoryPath* DirectoryPathValue = static_cast<FDirectoryPath*>(ValuePtr);
                            DirectoryPathValue->Path = Value;
                        }
                    }
                    else if (Property->IsA<FObjectPropertyBase>())
                    {
                    	FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property);
                    	void* ValuePtr = ObjectProperty->ContainerPtrToValuePtr<void>(Object);
                    	UObject* LoadedObject = StaticLoadObject(UObject::StaticClass(), nullptr, *Value);
                    	ObjectProperty->SetObjectPropertyValue(ValuePtr,LoadedObject);
                    	//VariableValue = ObjectValue->GetName();

                    }


                    break;
                }
            }
        }
    }
}

#endif


void UAssetCheckToolConfig::SetStringMapVariableNamesAndValues(UObject* Object, const FString& VariableName, const FMapStruct& MapToSet)
{
    if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
    {
        if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
        {
            for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
            {
                FProperty* Property = *PropertyIt;
                if (Property->GetName() == VariableName)
                {
                    if (Property->IsA<FMapProperty>())
                    {
                        FMapProperty* MapProperty = CastField<FMapProperty>(Property);
                        void* ValuePtr = MapProperty->ContainerPtrToValuePtr<void>(Object);
                        if (MapProperty->KeyProp->IsA<FStrProperty>() && MapProperty->ValueProp->IsA<FStrProperty>())
                        {
                            FScriptMapHelper MapHelper(MapProperty, ValuePtr);
                            FStrProperty* KeyProp = CastField<FStrProperty>(MapProperty->KeyProp);
                            FStrProperty* ValueProp = CastField<FStrProperty>(MapProperty->ValueProp);

                            // Clear the existing map
                            MapHelper.EmptyValues();

                            // Add the new values
                            for (const auto& Pair : MapToSet.StringMap)
                            {
                                int32 KeyIndex = MapHelper.AddDefaultValue_Invalid_NeedsRehash();
                                KeyProp->SetPropertyValue(MapHelper.GetKeyPtr(KeyIndex), Pair.Key);
                                ValueProp->SetPropertyValue(MapHelper.GetValuePtr(KeyIndex), Pair.Value);

                                MapHelper.Rehash();
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
}


void UAssetCheckToolConfig::SetFloatVariableValue(UObject* Object, const FString& VariableName, float Value)
{
	if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
	{
		if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
		{
			for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				if (Property->GetName() == VariableName)
				{
					if (Property->IsA<FFloatProperty>())
					{
						FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property);
						void* ValuePtr = FloatProperty->ContainerPtrToValuePtr<void>(Object);
						float* FloatValue = static_cast<float*>(ValuePtr);
						*FloatValue = Value;
					}
					else if (Property->IsA<FIntProperty>())
					{
						FIntProperty* IntProperty = CastField<FIntProperty>(Property);
						void* ValuePtr = IntProperty->ContainerPtrToValuePtr<void>(Object);
						int32* IntValue = static_cast<int32*>(ValuePtr);
						*IntValue = static_cast<int32>(Value);
					}
					else if (Property->IsA<FBoolProperty>())
					{
						FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property);
						void* ValuePtr = BoolProperty->ContainerPtrToValuePtr<void>(Object);
						bool* BoolValue = static_cast<bool*>(ValuePtr);
						*BoolValue = (Value != 0.0f);
					}
					break;
				}
			}
		}
	}
}


void UAssetCheckToolConfig::SetColorVariableValue(UObject* Object, const FString& VariableName, const FLinearColor& Value)
{
	if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
	{
		if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
		{
			for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				if (Property->GetName() == VariableName)
				{

					if (Property->IsA<FStructProperty>())
					{
						FStructProperty* StructProperty = CastField<FStructProperty>(Property);
						if (StructProperty->Struct == TBaseStructure<FColor>::Get())
						{
							void* ValuePtr = StructProperty->ContainerPtrToValuePtr<void>(Object);
							FLinearColor* ColorValue = static_cast<FLinearColor*>(ValuePtr);
							*ColorValue = Value;
						}
					}
					break;
				}
			}
		}
	}
}


void UAssetCheckToolConfig::SetVectorVariableValue(UObject* Object, const FString& VariableName, const FVector& Value)
{
	if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
	{
		if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
		{
			for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				if (Property->GetName() == VariableName)
				{
					if (Property->IsA<FStructProperty>())
					{
						FStructProperty* StructProperty = CastField<FStructProperty>(Property);

						if (StructProperty->Struct == TBaseStructure<FVector>::Get())
						{
							void* ValuePtr = StructProperty->ContainerPtrToValuePtr<void>(Object);
							FVector* VectorValue = static_cast<FVector*>(ValuePtr);
							*VectorValue = Value;
						}
						else if (StructProperty->Struct == TBaseStructure<FVector2D>::Get())
						{
							void* ValuePtr = StructProperty->ContainerPtrToValuePtr<void>(Object);
							FVector2D* Vector2DValue = static_cast<FVector2D*>(ValuePtr);
							*Vector2DValue = FVector2D(Value.X, Value.Y);
						}
					}
					break;
				}
			}
		}
	}
}



void UAssetCheckToolConfig::SetStringArrayVariableValue(UObject* Object, const FString& VariableName, const TArray<FString>& StringArrayValue)
{
	if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
	{
		if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
		{
			for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				if (Property->GetName() == VariableName && Property->IsA<FArrayProperty>())
				{
					FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property);
					if (FStrProperty* InnerStrProperty = CastField<FStrProperty>(ArrayProperty->Inner))
					{
						void* ValuePtr = ArrayProperty->ContainerPtrToValuePtr<void>(Object);
						TArray<FString>* StringArrayPtr = static_cast<TArray<FString>*>(ValuePtr);
						if (StringArrayValue.Num() > 0)
						{
							*StringArrayPtr = StringArrayValue;
						}
						else
						{
							StringArrayPtr->Empty();
						}
					}
					// Check for array of FDirectoryPath
					else if (FStructProperty* InnerStructProperty = CastField<FStructProperty>(ArrayProperty->Inner))
					{
						if (InnerStructProperty->Struct->GetFName() == FName("DirectoryPath"))
						{
							void* ValuePtr = ArrayProperty->ContainerPtrToValuePtr<void>(Object);
							TArray<FDirectoryPath>* DirectoryPathArrayPtr = static_cast<TArray<FDirectoryPath>*>(ValuePtr);
							if (StringArrayValue.Num() > 0)
							{
								DirectoryPathArrayPtr->Empty();
								for (const FString& Path : StringArrayValue)
								{
									FDirectoryPath DirectoryPath;
									DirectoryPath.Path = Path;
									DirectoryPathArrayPtr->Add(DirectoryPath);
								}
							}
							else
							{
								DirectoryPathArrayPtr->Empty();
							}
						}
					}
					break;
				}
			}
		}
	}
}




void UAssetCheckToolConfig::SetStringClassMapVariableNamesAndValues(UObject* Object, const FString& VariableName, const FClassMapStruct& MapToSet)
{
    if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
    {
        if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
        {
            for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
            {
                FProperty* Property = *PropertyIt;
                if (Property->GetName() == VariableName)
                {
                    if (Property->IsA<FMapProperty>())
                    {
                        FMapProperty* MapProperty = CastField<FMapProperty>(Property);
                        void* ValuePtr = MapProperty->ContainerPtrToValuePtr<void>(Object);

                        // 检查 Key 是 FString 类型，Value 是 UClass* 类型
                        if (MapProperty->KeyProp->IsA<FStrProperty>() && MapProperty->ValueProp->IsA<FClassProperty>())
                        {
                            FScriptMapHelper MapHelper(MapProperty, ValuePtr);
                            FStrProperty* KeyProp = CastField<FStrProperty>(MapProperty->KeyProp);
                            FClassProperty* ValueProp = CastField<FClassProperty>(MapProperty->ValueProp);

                            // 清空现有的映射
                            MapHelper.EmptyValues();

                            // 添加新的值
                            for (const auto& Pair : MapToSet.ClassMap)
                            {
                                int32 KeyIndex = MapHelper.AddDefaultValue_Invalid_NeedsRehash();
                                KeyProp->SetPropertyValue(MapHelper.GetKeyPtr(KeyIndex), Pair.Key);
                                ValueProp->SetObjectPropertyValue(MapHelper.GetValuePtr(KeyIndex), Pair.Value);

                                MapHelper.Rehash();
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
}



void UAssetCheckToolConfig::SetStringArrayMapProperty(UObject* Object, const FString& VariableName, const FStringArrayMapStruct& InStringArrayMap)
{
    // 检查输入对象是否有效
    if (!Object)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid Object!"));
        return;
    }

    if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
    {
        if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
        {
            // 查找对应的属性
            for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
            {
                FProperty* Property = *PropertyIt;
                if (Property->GetName() == VariableName)
                {
                    if (Property->IsA<FMapProperty>())
                    {
                        FMapProperty* MapProperty = CastField<FMapProperty>(Property);
                        void* ValuePtr = MapProperty->ContainerPtrToValuePtr<void>(Object);

                        // 检查 Key 是 FString 类型，Value 是 FArrayStruct 类型
                        if (MapProperty->KeyProp->IsA<FStrProperty>() && MapProperty->ValueProp->IsA<FStructProperty>())
                        {
                            FScriptMapHelper MapHelper(MapProperty, ValuePtr);
                            FStrProperty* KeyProp = CastField<FStrProperty>(MapProperty->KeyProp);
                            FStructProperty* ValueProp = CastField<FStructProperty>(MapProperty->ValueProp);

                            // 清空现有的映射
                            MapHelper.EmptyValues();

                            // 添加新的值
                            for (const auto& Pair : InStringArrayMap.StringArrayMap)
                            {
                                int32 KeyIndex = MapHelper.AddDefaultValue_Invalid_NeedsRehash();
                                KeyProp->SetPropertyValue(MapHelper.GetKeyPtr(KeyIndex), Pair.Key);

                                // 复制 FArrayStruct 的值
                                FArrayStruct* ValueStruct = reinterpret_cast<FArrayStruct*>(MapHelper.GetValuePtr(KeyIndex));
                                if (ValueStruct)
                                {
                                    *ValueStruct = Pair.Value; // 直接赋值
                                }

                                MapHelper.Rehash();
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
}

void UAssetCheckToolConfig::SetAssetFilterRuleMapProperty(UObject* Object, const FString& VariableName, const FAssetFilterRuleStruct& InAssetFilterRule)
{
	if (!Object)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Object!"));
		return;
	}

	if (UBlueprint* Blueprint = Cast<UBlueprint>(Object->GetClass()->ClassGeneratedBy))
	{
		if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
		{
			// 查找对应的属性
			for (TFieldIterator<FProperty> PropertyIt(GeneratedClass, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				if (Property->GetName() == VariableName)
				{
                    UE_LOG(LogTemp, Log, TEXT("找到属性：%s"), *VariableName);
                    UE_LOG(LogTemp, Log, TEXT("属性类型：%s"), *(Property->GetClass()->GetName()));

                    // 检查属性是否为映射类型，并且值类型是否为 FAssetFilterRuleStruct
                    if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
                    {
                        UE_LOG(LogTemp, Log, TEXT("属性是FStructProperty类型"));
                        if (StructProperty->Struct == FAssetFilterRuleStruct::StaticStruct())
                        {
                        		void* ValuePtr = StructProperty->ContainerPtrToValuePtr<void>(Object);
                        		FAssetFilterRuleStruct* StringArrayPtr = static_cast<FAssetFilterRuleStruct*>(ValuePtr);

                        			*StringArrayPtr = InAssetFilterRule;

                        }
                    }
					break;
				}

			}
		}
	}

}

void UAssetCheckToolConfig::SetAssetNameRuleConfig(const FAssetNameRulesStruct& InConfigs)
{
	UAssetCheckToolConfig* StaticMeshConfig = GetMutableDefault<UAssetCheckToolConfig>();
	StaticMeshConfig->AssetNameRuleConfig = InConfigs;
	StaticMeshConfig->SaveConfig(CPF_Config, *StaticMeshConfig->GetDefaultConfigFilename());
	if (GConfig)
	{
		GConfig->Flush(false);
	}
#if ENGINE_MAJOR_VERSION >= 5
	StaticMeshConfig->TryUpdateDefaultConfigFile();
#else
	StaticMeshConfig->UpdateDefaultConfigFile();
#endif
}

const FAssetNameRulesStruct& UAssetCheckToolConfig::GetAssetNameRuleConfig()
{
	return GetMutableDefault<UAssetCheckToolConfig>()->AssetNameRuleConfig;
}


void UAssetCheckToolConfig::SetDirectoryRuleConfig(const FDirectoryRulesGroupStruct& InConfigs)
{
	UAssetCheckToolConfig* StaticMeshConfig = GetMutableDefault<UAssetCheckToolConfig>();
	StaticMeshConfig->DirectoryRuleConfig = InConfigs;
	StaticMeshConfig->SaveConfig(CPF_Config, *StaticMeshConfig->GetDefaultConfigFilename());
	if (GConfig)
	{
		GConfig->Flush(false);
	}
#if ENGINE_MAJOR_VERSION >= 5
	StaticMeshConfig->TryUpdateDefaultConfigFile();
#else
	StaticMeshConfig->UpdateDefaultConfigFile();
#endif
}

const FDirectoryRulesGroupStruct& UAssetCheckToolConfig::GetDirectoryRuleConfig()
{
	return GetMutableDefault<UAssetCheckToolConfig>()->DirectoryRuleConfig;
}

void UAssetCheckToolConfig::ExportMoveRuleSimpleConfig(const TArray<FMoveRuleSimple>& MoveRules)
{
	// 创建一个JSON数组
	TArray<TSharedPtr<FJsonValue>> JsonArray;

	// 遍历MoveRules数组，将每个元素转换为JSON对象并添加到JsonArray中
	for (const FMoveRuleSimple& MoveRule : MoveRules)
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

		// 使用FJsonObjectConverter将结构体转换为JSON对象
		if (FJsonObjectConverter::UStructToJsonObject(FMoveRuleSimple::StaticStruct(), &MoveRule, JsonObject.ToSharedRef(), 0,
													  0))
		{
			JsonArray.Add(MakeShareable(new FJsonValueObject(JsonObject)));
		}
	}

	// 创建一个JSON写入器
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);

	// 将JsonArray写入字符串
	FJsonSerializer::Serialize(JsonArray, Writer);

	// 将字符串保存到文件
	FString FilePath = FPaths::ProjectDir() / TEXT("MoveRulesConfig.json");
	FFileHelper::SaveStringToFile(OutputString, *FilePath);
}



#undef LOCTEXT_NAMESPACE