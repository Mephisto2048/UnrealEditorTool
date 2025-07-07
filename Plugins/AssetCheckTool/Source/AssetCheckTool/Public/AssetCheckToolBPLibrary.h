// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "FoliageType.h"
#include "Editor/FoliageEdit/Private/SFoliageEdit.h"
#include "Editor/FoliageEdit/Private/FoliageEdMode.h"
#include "EditorModes.h"
#include "InstancedFoliageActor.h"
#include "EditorModeManager.h"
#include "Materials/MaterialInstance.h"
#include "EngineUtils.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#include "StaticMeshResources.h"
#include "EditorFramework/AssetImportData.h"
#include "Misc/Paths.h"

#include "Engine/StaticMesh.h"
#include "Modules/ModuleManager.h"
#include "EditorReimportHandler.h"
#include "StaticMeshOperations.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "GameFramework/Actor.h"
#include "Editor/EditorEngine.h"
#include "EngineGlobals.h"
#include "EditorViewportClient.h"
#include "SceneView.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodySetup.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "AssetToolsModule.h"
#include "AssetExportTask.h"
#include "Engine/AssetManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMeshActor.h"
#include "Editor/UnrealEd/Public/ObjectTools.h"
#include "UObject/SavePackage.h"
#include "PaperSprite.h"


#include "Engine/LODActor.h"
#include "HAL/FileManager.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Editor/UnrealEd/Public/PackageTools.h"
#include "FileHelpers.h"
#include "RenderingThread.h"
#include "RenderResource.h"
#include "Rendering/StaticMeshVertexBuffer.h"
#include "Math/NumericLimits.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/DataTable.h"
#include "Containers/UnrealString.h"
#include "SLevelViewport.h"
#include "LevelEditor.h"
#include "Engine/Texture2D.h"
#include "IAssetTools.h"
#include "IAssetTypeActions.h"
#include "ImageUtils.h"
#include "FoliageType_InstancedStaticMesh.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "Editor/UnrealEd/Public/AssetSelection.h"
#include "Editor/UnrealEd/Public/EditorDirectories.h"
#include "MaterialEditor/MaterialEditorInstanceConstant.h"
#include "InstancedFoliage.h"
#include "EditorModeManager.h"
#include "FoliageEditModule.h"
#include "LevelEditor.h"
#include "LevelEditorActions.h"
#include "Kismet/GameplayStatics.h"

#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"
#include "Blueprint/UserWidget.h"
#include "EditorUtilitySubsystem.h"
#include "Internationalization/Text.h"
#include "Internationalization/Internationalization.h"
#include "DrawDebugHelpers.h"
#include "Landscape.h"
#include "LandscapeStreamingProxy.h"
#include "EditorLevelUtils.h"
#include "CTGTracker.inl"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/SkeletalMesh.h"
#include "Rendering/SkeletalMeshRenderData.h"

#include "SourceControlOperations.h"
#include "ISourceControlModule.h"
#include "SourceControlHelpers.h"

#include "UObject/UObjectIterator.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include <regex>
#include "Editor/UnrealEd/Public/UnrealEd.h"

#include "Misc/Optional.h"
#include <iomanip>
#include "AssetCheckToolConfig.h"
#include "MoveRule.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "MaterialEditorUtilities.h"
#include "DebugUtil.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialExpressionObjectPositionWS.h"

#if ENGINE_MAJOR_VERSION >= 5
#include "AssetRegistry/AssetRegistryModule.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/StaticMesh.h"
#include "EditorModeTools.h"
#include "Subsystems/EditorActorSubsystem.h"
#else
#include "AssetTools/Private/AssetTools.h"
#include "AssetRegistryModule.h"
#include "AssetData.h"
#include "Engine/StaticMeshConfig.h"
#endif



#include "AssetCheckToolBPLibrary.generated.h"


/*
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu.
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/

UCLASS()
class  UBRAssetToolTagAssetUserData : public UAssetUserData {
	GENERATED_BODY()
public:
	UBRAssetToolTagAssetUserData(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	{}

	UPROPERTY(EditAnywhere, Category = "AssetCheckTool")
	TArray<FString> TagArray;

	void AddTag(const FString& Tag) {
		TagArray.Add(Tag);
	}

	TArray<FString> GetAllTags() const
	{
		return TagArray;
	}

	void RemoveTag(const FString& Tag) {
		TagArray.Remove(Tag);
	}

	void SetTags(const TArray<FString>& NewTags)
	{
		TagArray = NewTags;
	}

	bool HasTag(const FString& Tag) const
	{
		return TagArray.Contains(Tag);
	}
};


// 在全局作用域声明枚举
UENUM(BlueprintType)
enum class EFoliageMobilityType : uint8
{
	Static UMETA(DisplayName = "Static"),
	Stationary UMETA(DisplayName = "Stationary"),
	Movable UMETA(DisplayName = "Movable")
};


USTRUCT(BlueprintType)
struct FMyMaterialBasePropertStruct
{
	GENERATED_BODY()
	/** Enables override of the blend mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		bool OverrideBlendMode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		TEnumAsByte<EBlendMode> BlendMode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		TEnumAsByte<EBlendMode> BlendModeBase;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		bool OverrideTwoSided;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		bool TwoSided;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		bool TwoSidedBase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		bool OverrideUseHQIBL;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		bool UseHQIBL;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		bool UseHQIBLBase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		bool OverrideSubsurfaceProfile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		USubsurfaceProfile* SubsurfaceProfile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		USubsurfaceProfile* SubsurfaceProfileBase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		bool OverrideOpacityMaskClipValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		float OpacityMaskClipValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		float OpacityMaskClipValueBase;
};




USTRUCT(BlueprintType)
struct FTextureGroup_ACT
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetCheckTool/texture", meta = (DisplayName = "贴图组"))
		TEnumAsByte<TextureGroup> TextureGroup;
};


USTRUCT(BlueprintType)
struct FTextureCompressionSetting_ACT
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetCheckTool/texture", meta = (DisplayName = "贴图压缩设置"))
		TEnumAsByte<TextureCompressionSettings> TextureCompressionSetting;
};


USTRUCT(BlueprintType)
struct FPathFilter
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetCheckTool/Path", meta = (DisplayName = "资源路径", RelativeToGameContentDir, ContentDir))
		TArray<FDirectoryPath> Directories;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetCheckTool/Path", meta = (DisplayName = "忽略子文件夹"))
		TArray<FString> IgnoreSubFolders;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetCheckTool/Path", meta = (DisplayName = "仅处理文件名"))
		TArray<FString> ProcessFileNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssetCheckTool/Path", meta = (DisplayName = "忽略文件名"))
		TArray<FString> IgnoreFileNames;
};

// 定义一个结构体来存储键值对 用于列表
USTRUCT(BlueprintType)
struct FKeyValue {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "KeyValue")
	FString Key;

	UPROPERTY(BlueprintReadWrite, Category = "KeyValue")
	FString Value; // 这里的 Value 是一个字符串
};

// 定义一个结构体来存储每一行的 KeyValue 数组
USTRUCT(BlueprintType)
struct FKeyValueRow {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "KeyValueRow")
	TArray<FKeyValue> KeyValueArray; // 每一行的 KeyValue 数组
	// 重载 == 操作符
	bool operator==(const FKeyValueRow& Other) const {
		// 比较两个 FKeyValueRow 是否相等，这里简单地比较它们的 KeyValueArray 长度
		// 可以根据需要进行更复杂的比较
		return KeyValueArray.Num() == Other.KeyValueArray.Num();
	}

};

UCLASS()
class UAssetCheckToolBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()


	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Get AutoCompute LOD ScreenSize", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void GetAutoComputeLODScreenSize(UStaticMesh* StaticMesh, bool& AutoComputeLODScreenSize);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set LODScreenSize", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void SetLODScreenSize(UStaticMesh * StaticMesh, int32 LODIndex,float Value);


	/**
	 * 获取纹理指定纹理组的MaxInGameSize
	 * @param Texture 纹理
	 * @param OptionalGroup 纹理组
	 * @return 纹理的MaxInGameSize
	 */
	UFUNCTION(BlueprintPure)
		static int GetMaxInGameSizeWithSpecialGroup(UTexture* Texture, TEnumAsByte<TextureGroup> OptionalGroup);

	/**
	 * 获取纹理的MaxInGameSize
	 * @param Texture 纹理
	 * @return 纹理的MaxInGameSize
	 */
	UFUNCTION(BlueprintPure)
	static int GetMaxInGameSize(UTexture* Texture);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetMaxInGameSize"), Category = "AssetCheckTool")
	static bool GetMaxInGameSizev2(UTexture* Texture, int32& OutWidth, int32& OutHeight);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetMipMapNum"), Category = "AssetCheckTool")
	static int GetMipMapNum(UTexture* Texture);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set AutoComputeLODScreenSize", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void SetAutoComputeLODScreenSize(UStaticMesh* StaticMesh, bool Enable);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get MaterialSlotName", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void GetMaterialSlotName(UStaticMesh* StaticMesh, int32 MatIndex, FString& SlotName);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set MaximumSize", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void SetMaximumSize(UTexture* Texture, float value);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetTextureResourceSize", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static int32 GetTextureResourceSize(UTexture* Texture);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetTextureMaxLODSize", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static int32 GetTextureMaxLODSize(const UTexture* Texture);


	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Get MaterialIndex With Section", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void GetMaterialIndexWithScetion(UStaticMesh* StaticMesh, int32 InLODIndex, int32 SectionIndex, int32& MaterialIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Get Customized Collision", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void GetCustomizedCollision(UStaticMesh* StaticMesh, bool& CustomizedCollision, FString& CollisionMesh);

	UFUNCTION(BlueprintCallable, Category = "Material")
		static UMaterialInterface* GetMaterialParent(UMaterialInstance* MaterialInstance);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get ModelTriangleCount", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static int32 GetModelTriangleCount(UStaticMesh* StaticMesh, int32 LODIndex);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get SkeletalMeshTriangleCount", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static int32 GetSkeletalMeshTriangleCount(USkeletalMesh* SkeletalMesh, int32 LODIndex);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get All LOD TriangleCounts", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static TArray<int32> GetAllLODTriangleCounts(UStaticMesh* StaticMesh);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get All LOD TriangleCounts", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static TArray<int32> GetAllLODTriangleCountsSK(USkeletalMesh* SkeletalMesh);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Foliage Cull Distance", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void SetFoliageCullDistance(UFoliageType_InstancedStaticMesh* FoliageTypeInput, float MinCullDistance, float MaxCullDistance);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get GenerateLightmapUVs", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void GetGenerateLightmapUVs(UStaticMesh* StaticMesh, bool& GenerateLightmapUVs);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Selected FoliageType", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void GetSelectedFoliageTypes(TArray<UObject*>& FoliageTypeFiles, TArray<AInstancedFoliageActor*>& InstancedFoliageActors, TArray<UFoliageType*>& FoliageTypes);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Tag To Actors", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void AddTagToActors(const FName& Tag, bool& bSuccess, const TArray<AActor*>& Actors);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Tag To Actor", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void AddTagToActor(const FName& Tag, bool& bSuccess, AActor* Actor);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Actor Tag", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void SetActorTag(const FName& Tag, int32 Index, AActor* Actor);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Level", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void GetLevel(AActor* Actor,FString& LevelName);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Remove Tag From Actors", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void RemoveTagFromActors(const FName& Tag, bool& bSuccess, const TArray<AActor*>& Actors);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get StaticMesh Source FilePath", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static FString GetStaticMeshSourceFilePath(UStaticMesh* StaticMesh);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get StaticMesh Source FilePath", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static FString GetSkeletalMeshSourceFilePath(USkeletalMesh* SkeletalMesh);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Find StaticMesh WithSameSource", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void FindStaticMeshWithSameSource(UStaticMesh* Mesh, const FString& AdditionalSearchPath, int32 mode, const FString& FilterString, TArray<UStaticMesh*>& OutMeshes);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Reimport Static Mesh", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void ReimportStaticMesh(UStaticMesh* StaticMesh);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get In Viewport Actor", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void GetInViewportActor(TArray<AActor*>& OutActorList, float MaxDistance);

	template <typename T>
	static void SortNumListTemplate(const TArray<T>& NumList, TArray<T>& SortedList, TArray<int32>& IDList, bool bReverse);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Sort int List", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void SortNumListInt(const TArray<int32>& NumList, TArray<int32>& SortedList, TArray<int32>& IDList, bool bReverse);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Sort float List", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void SortNumListFloat(const TArray<float>& NumList, TArray<float>& SortedList, TArray<int32>& IDList, bool bReverse);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Find Instanced StaticMesh Components", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void FindInstancedStaticMeshComponents(UStaticMesh* StaticMesh, AInstancedFoliageActor* InstancedFoliageActor, TArray<UInstancedStaticMeshComponent*>& ResultComponents, bool& bFound);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Collision Profile", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static bool SetCollisionProfile(UStaticMesh* StaticMesh, const FString& CollisionProfileName);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Save Array Text", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static bool SaveArrayText(FString SaveDirector, FString FileName, TArray<FString> SaveText, bool AllowOverWriting );

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Save Array Text plus", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static bool SaveArrayTextPlus(FString SaveDirector, FString FileName, TArray<FString> SaveText, bool AllowOverWriting, FString Delimiter);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Check UVSet Empty", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<int32> CheckUVSetEmpty(UStaticMesh* StaticMesh, int32 LODIndex);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Check UVSet Empty", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static TArray<int32> CheckUVSetEmptySkeletal(USkeletalMesh* SkeletalMesh, int32 LODIndex);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Export StaticMeshes As FBX", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void ExportStaticMeshesAsFBX(const TArray<UStaticMesh*>& StaticMeshes, const FString& OutputFolder);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get All Assets By Class", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<UObject*> GetAllAssetsByClass(TSubclassOf<UObject> AssetClass);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get All Asset Names By Class", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<FString> GetAllAssetNamesByClass(TSubclassOf<UObject> AssetClass, bool bExcludeRedirectors);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get All Actors By Class", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<AActor*> GetAllActorsByClass(TSubclassOf<AActor> ActorClass);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Check And RemoveUVSets", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void CheckAndRemoveUVSets(UStaticMesh* Mesh, int32 MaxUVChannelCount, int32 LODIndex);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Find Same Source Actors With Distance", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<FString> FindSameSourceActorsWithDistance(TArray<AStaticMeshActor*> StaticMeshActors, float DistanceThreshold);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Find Same Source Actors With Distance Ratio", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static TArray<AStaticMeshActor*> FindSameSourceActorsWithDistanceRatio(AStaticMeshActor* InputActor, float Ratio);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Actors From Name", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<AStaticMeshActor*> GetActorsFromName(const FString& ActorsString);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get StaticMesh LOD Source ImportFilename", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static FString GetStaticMeshLODSourceImportFilename(UStaticMesh* StaticMesh, int32 LODIndex);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Import StaticMesh LOD", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void ImportStaticMeshLOD(UStaticMesh* StaticMesh, int32 LODIndex, const FString& FilePath);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get All Assets By Class With Filter", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<UObject*> GetAllAssetsByClassWithFilter(TSubclassOf<UObject> AssetClass, const TArray<FString>& SpecifiedPaths, const TArray<FString>& FilterEndWith, bool EndWithOut);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get All Asset Names By Class With Filter", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<FString> GetAllAssetNamesByClassWithFilter(
			const TArray<TSubclassOf<UObject>>& AssetClasses, // 多个类的过滤
			const TArray<FString>& SpecifiedPaths,
			const TArray<FString>& FilterEndWith,
			bool EndWithOut,
			bool bUseClassFilter,
			bool bUseCast);//有bug 待修

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get All Asset Names By Path With Filter", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<FString> GetAllAssetNamesByPathWithFilter(
			const TArray<FString>& SpecifiedPaths,
			const TArray<FString>& FilterContains,
			bool bExcludeContains,
			bool bIncludeSubfolders,
			UClass* AssetClass);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get All Asset Names By Path With Filter Plus", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static TArray<FString> GetAllAssetNamesByPathWithFilterPlus(
	const TArray<FString>& SpecifiedPaths,
	const TArray<FString>& FilterContains,
	const TArray<FString>& ExcludeContains,
	bool bIncludeSubfolders,
	UClass* AssetClass,
	bool bIncludeRedirectors, // 新增参数：是否包含重定向文件
	bool bWithClass = true);  // 默认值为true

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get All Assets By Class With Filter Plus", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<UObject*> GetAllAssetsByClassWithFilterPlus(
			TSubclassOf<UObject> AssetClass,
			const TArray<FString>& SpecifiedPaths,
			const TArray<FString>& WithFilters,
			const TArray<FString>& WithoutFilters);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetAllAssetNamesByPathWithFilters", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static TArray<FString> GetAllAssetNamesByPathWithFilters(
	const TArray<FString>& SpecifiedPaths,
	const TArray<FString>& FilterContains,
	const TArray<FString>& ExcludeContains,
	bool bIncludeSubfolders,
	const TArray<UClass*>& AssetClasses, // Changed to TArray<UClass*>
	bool bIncludeRedirectors);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Is Asset Valid By Filter", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static bool IsAssetValidByFilter(
			UObject* Asset,
			const TArray<FDirectoryPath>& DirectoryPaths ,
			const TArray<FString>& IgnoreSubPaths,
			const TArray<FString>& OnlyNames,
			const TArray<FString>& IgnoreNames);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Is Asset Valid By Ignore Filter", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static bool IsAssetValidByIgnoreFilter(
			UObject* Asset,
			const TArray<FString>& IgnoreSubPaths,
			const TArray<FString>& IgnoreNames);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Is Asset Valid By Ignore Filter Plus", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static bool IsAssetValidByIgnoreFilterPlus(
		UObject* Asset,
		const TArray<FString>& IgnoreSubPaths,
		const TArray<FString>& IgnoreNames,
		const TArray<FString>& IncludeNames);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Is Asset Valid By Ignore Filter", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static bool IsAssetPathValidByIgnoreFilter(
		const FString& AssetPath,
		const TArray<FString>& IgnoreSubPaths,
		const TArray<FString>& IgnoreNames);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Is Asset Valid By Ignore Filter Plus", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static bool IsAssetPathValidByIgnoreFilterPlus(
	const FString& AssetPath,
	const TArray<FString>& IgnoreSubPaths,
	const TArray<FString>& IgnoreNames,
	const TArray<FString>& IncludeNames);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Is Asset Valid By Filters", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static bool IsAssetValidByFilters(
	const FString& AssetPath,
	const TArray<FString>& IgnoreSubPaths,
	const TArray<FString>& IgnoreNames,
	const TArray<FString>& IncludeNames, // 新增的参数
	const TArray<UClass*>& AllowedClasses) ;

	UFUNCTION(BlueprintCallable, Category = "AssetCheckTool")
	static void RecursiveAddSubFolders(const FString& Path, FARFilter& AssetFilter);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Mark Object Dirty", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void MarkObjectDirty(UObject* object, bool Enable);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Save Assets", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void SaveAssets(const TArray<UObject*>& Assets);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Save Assets", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void SaveAssetsSoft(const TArray<TSoftObjectPtr<UObject>>& Assets);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Is Mesh Closed", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static bool IsMeshClosed(UStaticMesh* Mesh);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "check UV Overlap", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static bool checkUVOverlap(UStaticMesh* Mesh, int32 LODIndex, int32 UVIndex,float tolerance);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "CheckUVOverlapForSkeletalMesh", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static bool CheckUVOverlapForSkeletalMesh(USkeletalMesh* Mesh, int32 LODIndex, int32 UVIndex, float tolerance);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Are Segments Intersecting", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static bool AreSegmentsIntersecting(const FVector2D& A1, const FVector2D& A2, const FVector2D& B1, const FVector2D& B2, float tolerance);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Check UV Bounds", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static bool CheckUVBounds(UStaticMesh* StaticMesh, int32 LODIndex, int32 UVIndex);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Check UV Bounds", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static bool CheckUVBoundsSkeletal(USkeletalMesh* SkeletalMesh, int32 LODIndex, int32 UVIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Split String", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<FString> SplitString(const FString& SourceString, const FString& Delimiter);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Are Mesh Sections Closed", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<bool> AreMeshSectionsClosed(UStaticMesh* Mesh);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Are Mesh Sections Closed", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<bool> AreMeshSectionsClosedwithOutputEdge(UStaticMesh* Mesh, TArray<FVector>& EdgeStartArray, TArray<FVector>& EdgeEndArray);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Vector Min Or Max", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static FVector VectorMinOrMax(FVector Vector1, FVector Vector2, bool bMax);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get UVset Num", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static int32 GetNumTexCoords(UStaticMesh* StaticMesh, int32 LODIndex);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get UVset Num", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static int32 GetNumTexCoordsForSkeletalMesh(USkeletalMesh* SkeletalMesh, int32 LODIndex);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Material Instance Switch Parameter Value", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void SetMaterialInstanceSwitchParameterValue(UMaterialInstance* Instance, FName ParameterName, bool SwitchValue, bool bOverride);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Material Instance Switch Parameter Value", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void GetMaterialInstanceSwitchParameterValue(UMaterialInstance* Instance, FName ParameterName, bool& SwitchValue, bool& IsOverride, bool bOveriddenOnly, bool bCheckParent);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Is BoundingBox Visible In Viewport", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static bool IsBoundingBoxVisibleInViewport(const FVector& Position, const FVector& BoxExtent,float MaxDistance);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Visible FoliageActors In Viewport", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<AInstancedFoliageActor*> GetVisibleFoliageActorsInViewport();


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Find FoliageTypes For StaticMesh", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<UFoliageType*> FindFoliageTypesForStaticMesh(UStaticMesh* TargetStaticMesh);








	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set FoliageType Culling Distance", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void SetFoliageTypeCullingDistance(UFoliageType* FoliageType, float distance);



	UFUNCTION(BlueprintCallable,  BlueprintPure, meta = (DisplayName = "Get FoliageType Culling Distance", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void GetFoliageCullDistance(UFoliageType_InstancedStaticMesh* FoliageTypeInput, float& MinCullDistance, float& MaxCullDistance);








	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Save ObjectArray To File", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static bool SaveObjectArrayToFile(const TArray<UObject*>& ObjectArray, const FString& FilePath, UClass* ObjectType);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Load ObjectArray From File", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<UObject*> LoadObjectArrayFromFile(const FString& FilePath, TSubclassOf<UObject> ObjectType);



	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Parse And Print KeyValue Pairs", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void ParseAndPrintKeyValuePairs(const FString& InputString);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get DataTable Property Names", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<FString> GetDataTablePropertyNames(UDataTable* DataTable);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set MIC Scalar Param without Updata", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static bool SetMICScalarParam_withoutUpdata(UMaterialInstanceConstant* Material, FString ParamName, float Value);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set MIC Vector Param without Updata", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static bool SetMICVectorParam_withoutUpdata(UMaterialInstanceConstant* Material, FString ParamName, FLinearColor Value);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set MIC Texture Param without Updata", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static bool SetMICTextureParam_withoutUpdata(UMaterialInstanceConstant* Material, FString ParamName, UTexture2D* Texture);

	//UFNCTION(BlueprintCallable, meta = (DisplayName = "Set All Foliage Culling Mask BR", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	//	static void SetAllFoliageCullingMaskBR(bool DC0, bool DC1, bool DC2, bool SC0, bool SC1, bool SC2, bool SC3);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Find References In Scene", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void FindReferencesInScene(UStaticMesh* TargetMesh, TArray<AActor*>& OutActorArray, TArray<AActor*>& OutFoliageActorArray);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Find References In Viewport", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void FindReferencesInViewport(UStaticMesh* TargetMesh, TArray<AActor*>& OutActorArray, TArray<AActor*>& OutFoliageActorArray, float MaxDistance);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Find References Num In Scene", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void FindReferencesNumInScene(UStaticMesh* TargetMesh, int32& OutActorCount, int32& OutFoliageActorCount, int32& OutInstanceCount);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set View Mode", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void SetViewMode(EViewModeIndex viewmode);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Viewport Info", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void GetViewportInfos(FVector& OutLocation, FRotator& OutRotation, float& OutFOV);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Viewport FOV", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void SetViewportFOV(float FOV);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get FoliageTpye", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void GetFoliageType(UStaticMesh* TargetMesh, TArray<UFoliageType*>& OutFoliageTypes, bool FindSameNameFoliage);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Calculate AverageVertexColor And Material Per Section", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void CalculateAverageVertexColorAndMaterialPerSection(UStaticMesh* Mesh, int32 LODIndex, TArray<FColor>& AverageColors, TArray<UMaterialInterface*>& Materials);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Calculate AverageVertexColor And Material Per Section With Area", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void CalculateAverageVertexColorAndMaterialPerSectionWithArea(UStaticMesh* Mesh, int32 LODIndex, TArray<FColor>& AverageColors, TArray<UMaterialInterface*>& Materials);
	//UFUNCTION(BlueprintCallable, meta = (DisplayName = "SelectAsset In ContentFolder", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	//static void SelectAssetInContentFolder(UStaticMesh* Mesh);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Convert Foliage To Actor", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void ConvertFoliageToActor(AInstancedFoliageActor* InstancedFoliageActor);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Convert Actors To Foliage by folder", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void ConvertActorsToFoliageByFolder(FName FolderName);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Convert Foliage To Actor With Array", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void ConvertFoliageToActorWithArray(AInstancedFoliageActor* InstancedFoliageActor, TArray<UObject*> MeshesToConvert, TArray<UFoliageType*> FoliageTypes, bool bDestroy,TArray<AActor*>& NewActors);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Convert Actors To Foliage", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void ConvertActorsToFoliage(TArray<AActor*> ActorsToConvert, TArray<UFoliageType*> FoliageTypes, bool bConvertAll);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Convert Actors To Foliage With Tag", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void ConvertActorsToFoliageWithTag(TArray<AActor*> ActorsToConvert,  TArray<FString>& OutLevelPaths);



	UFUNCTION(BlueprintCallable,BlueprintPure, meta = (DisplayName = "Get StaticMesh Collision Preset", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetStaticMeshCollisionPreset(UStaticMesh* TargetStaticMesh, FString& CollisionPreset);


	UFUNCTION(BlueprintCallable,BlueprintPure, meta = (DisplayName = "GetSkeletalMeshCollisionPreset", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetSkeletalMeshCollisionPreset(USkeletalMesh* TargetSkeletalMesh, FString& CollisionPreset);

	UFUNCTION(BlueprintCallable,BlueprintPure, meta = (DisplayName = "GetPaperSpriteCollisionPreset", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetPaperSpriteCollisionPreset(UPaperSprite* TargetPaperSprite, FString& CollisionPreset);

	UFUNCTION(BlueprintCallable,BlueprintPure, meta = (DisplayName = "Get Tag From StaticMesh", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static TArray<FString> GetTagFromStaticMesh(UStaticMesh* StaticMesh);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Tag To StaticMeshs", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void AddTagToObjects(TArray<UObject*> Objects, const FString& Tag);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Remove Tag To StaticMeshs", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void RemoveTagFromObjects(TArray<UObject*> Objects, const FString& Tag);

	UFUNCTION(BlueprintCallable,BlueprintPure, meta = (DisplayName = "Has Tag In StaticMeshs", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static bool HasTagInStaticMesh(UStaticMesh* StaticMesh, const FString& Tag);

	UFUNCTION(BlueprintCallable,BlueprintPure, meta = (DisplayName = "Get LOD Status Of Actors In CurrentView", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetLODStatusOfActorsInCurrentView(const TArray<AActor*>& Actors, TArray<UStaticMesh*>& StaticMeshArray, TArray<int32>& LODStatusArray);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Update Material BR", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void UpdateMaterialBR(UMaterialInstanceConstant* Material);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Switch To Foliage Mode", ToolTips = "EM_Default,EM_Foliage,EM_Landscape,..."), Category = "AssetCheckTool")
	static void SwitchFoliageMode();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Split String To Strings",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void SplitStringToStrings(FString InputString, FString SplitString,  TArray<FString>& OutStrings, bool InCullEmpty = false);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Split Text To Strings",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static  void SplitTextToStrings(FText InputString, FString SplitString,  TArray<FString>& OutStrings, bool InCullEmpty = false);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Read CSV Text", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void ReadCSVToString(FString CSVPath, int32 SpacesNume,TArray<FString>& CSVLines_String,  TArray<FText>& CSVLines_Text,TArray<FString>& CSVLinesSplitBySpaces);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "InBound",Keywords = "AssetCheckTool sample test testing"),Category = "AssetCheckTool")
	static void InBound(AActor* Actor, AActor* BoundActor, bool& Inbound, bool OnlyXY);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Run EditorUtilityWidget By Path",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void RunEditorUtilityWidgetByPath(FString path);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Confirm Dialog", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static bool ConfirmDialog(FString message);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Move Viewport Cameras To Actor",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void MoveViewportCamerasToActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Browser To Objects",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void BrowserToObjects( TArray<UObject*> InObjectsToSync, bool bFocusContentBrowser = true );

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Sort To Int Array",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static TArray<int32> SortToIntArray(const TArray<FString>& strs);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Intersecting BBox Actors",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static TArray<AActor*> GetIntersectingBBoxActors(AActor* InputActor, TArray<AActor*> OtherActors);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Filtered Actors From Scene",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static TArray<AActor*> GetFilteredActorsFromScene(TArray<FString> ExcludeStrings, TArray<FString> IncludeStrings);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Are Actors Colliding",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static bool AreActorsColliding(UObject* Object1, AActor* Actor2);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Triangles Intersect",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static bool TrianglesIntersect(const FVector& V1_1, const FVector& V1_2, const FVector& V1_3, const FVector& V2_1, const FVector& V2_2, const FVector& V2_3);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Check Collision With Actors",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static bool CheckCollisionWithActors(UObject* Object, const TArray<AActor*>& Actors);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Ray Intersects Triangle",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static bool RayIntersectsTriangle(const FVector& RayOrigin, const FVector& RayDirection, const FVector& V0, const FVector& V1, const FVector& V2, FVector& OutIntersectionPoint);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Raycast Point And Distance",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static bool GetRaycastPointAndDistance(const FVector& Start, const FVector& End, float& OutHitDistance, FString& OutHitActorName, FVector& OutHitLocation, FVector& OutHitNormal, const TArray<AActor*>& Actors, const TArray<AActor*>& IgnoreActors)
	;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Scatter Points On Actor Bottom",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static TArray<FVector> ScatterPointsOnActorBottom(UObject* WorldContextObject, AActor* TargetActor, float PointSpacing, float RayLength, bool bDebugDraw);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Lowest Points In Actor Sections",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static TArray<FVector> GetLowestPointsInActorSections(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Shortest Raycast Distance",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static bool GetShortestRaycastDistance(const TArray<FVector>& Points, const FVector& Direction, float Distance,const TArray<AActor*>& Actors, const TArray<AActor*>& IgnoreActors, FVector& OutOriginalPoint, FVector& OutShortestPoint, float& OutShortestDistance, FString& OutHitActorName, FVector& OutHitLocation, FVector& OutHitNormal);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Generate Random Point In Triangle",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static  FVector GenerateRandomPointInTriangle(const FVector& A, const FVector& B, const FVector& C);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Scatter Points On Actor Surface",Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void ScatterPointsOnActorSurface(AActor* Actor, float MinDistance, int32 MaxAttempts, TArray<FVector>& OutPoints, TArray<FVector>& OutNormals);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set RenderTarget2D Size", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void SetRenderTargetSize(UTextureRenderTarget2D* RenderTarget, int32 NewWidth, int32 NewHeight);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Detail Mode", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void SetDetailMode(UStaticMeshComponent* StaticMeshComponent, EDetailMode NewDetailMode);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Material Base Property Overrides", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static FMyMaterialBasePropertStruct GetMaterialBasePropertyOverrides(UMaterialInstanceConstant* MaterialInstance);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Remove Material Base Property Overrides", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void RemoveMaterialBasePropertyOverrides(
	UMaterialInstanceConstant* MaterialInstance,
	bool bDisableBlendModeOverride,
	bool bDisableTwoSidedOverride,
	bool bDisableShadingModelOverride,
	bool bDisableOpacityMaskClipValueOverride);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ContainsExcludedKeyword", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static bool ContainsExcludedKeyword(const FString& AssetName, const TArray<FString>& ExcludeKeywords, bool bInvertFilter);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Find References Object With Class", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static TArray<FAssetData> FindReferencesObject(
	const TSoftObjectPtr<UObject>& TargetObjectPtr,
	UClass* ReferenceClass,
	TArray<FString>& OutPackageNames,
	TArray<TSoftObjectPtr<UObject>>& OutSoftObjects,
	const TArray<FString>& ExcludeKeywords,
	bool bInvertFilter);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Find References Object With Class And Depth", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static TArray<FAssetData> FindReferencesObjectWithDepth(
		const TSoftObjectPtr<UObject>& TargetObjectPtr,
		UClass* ReferenceClass,
		TArray<FString>& OutPackageNames,
		TArray<TSoftObjectPtr<UObject>>& OutSoftObjects,
		const TArray<FString>& ExcludeKeywords,
		bool bInvertFilter,
		int32 Depth);

	UFUNCTION(BlueprintCallable,  meta = (DisplayName = "Find References Package With Class", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<FString> FindReferencers(
			const FString& PackageName,
			const TArray<UClass*>& ReferenceClasses,
			const TArray<FString>& ExcludeKeywords,
			bool WithOutClassCheck,
			bool bInvertFilter);



	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Find Dependencies Package With Class", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static  TArray<FString> FindDependenciesWithClass(
		const FString& PackageName,
		const TArray<UClass*>& DependencyClasses,
		const TArray<FString>& ExcludeKeywords,
		bool WithOutClassCheck,
		bool bInvertFilter);

	UFUNCTION(BlueprintCallable,  meta = (DisplayName = "Find Packages Dependencies", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<FAssetData> FindDependencies(
			const FString& PackageName,
			const TArray<FString>& ExcludeKeywords,
			bool bInvertFilter);


	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Format Float", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static FString FormatFloat(float Value);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "FormatNumber", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static FString FormatNumber(float Number, int32 TotalLength, int32 DecimalPlaces);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Remove Spaces", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static FString RemoveSpaces(const FString& Original);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Overried MaterialInstance Texture Parameters", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void GetOverriedMaterialInstanceTextureParameters(
			UMaterialInstance* MaterialInstance,
			TArray<FString>& ChangedParameterNames,
			TArray<FString>& ChangedParameterValues);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Overried MaterialInstance Scalar Parameters", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void GetOverriedMaterialInstanceScalarParameters(
			UMaterialInstance* MaterialInstance,
			TArray<FString>& ChangedParameterNames,
			TArray<FString>& ChangedParameterValues);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Overried MaterialInstance Vector Parameters", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void GetOverriedMaterialInstanceVectorParameters(
			UMaterialInstance* MaterialInstance,
			TArray<FString>& ChangedParameterNames,
			TArray<FString>& ChangedParameterValues);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Overried MaterialInstance Static Switch Parameters", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void GetOverriedMaterialInstanceStaticSwitchParameters(
			UMaterialInstance* MaterialInstance,
			TArray<FString>& ChangedParameterNames,
			TArray<FString>& ChangedParameterValues);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get MaterialInstance Static Switch Parameters", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void GetMaterialInstanceStaticSwitchParameters(
		UMaterialInstance* MaterialInstance,
		TArray<FString>& ParameterNames,
		TArray<FString>& ParameterValues);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetMaterialInstanceStaticSwitchParametersNew", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void GetMaterialInstanceStaticSwitchParametersNew(
	UMaterialInstance* MaterialInstance,
	TArray<FString>& ParameterNames,
	TArray<FString>& ParameterValues);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get User Text Input", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static FString GetUserTextInput(const FString& PromptMessage);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Sync Assets", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void SyncAssets(const TArray<FString> PackageNames);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Object PathNames", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<FString> GetObjectPathNames(const TArray<UObject*>& Objects);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Check Assets Is Current", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void CheckAssetsIsCurrent(const TArray<FString> PathNames, TArray<FString>& OutdatedAssets, TArray<FString>& CurrentAssets);



	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Find Reference FoliageType", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<TSoftObjectPtr<UObject>> FindReferenceFoliageType(
			const TSoftObjectPtr<UObject>& TargetObjectPtr,
			TArray<FString>& OutPackageNames
		);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Get PackageNames", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static TArray<FString> GetPackageNamesFromAssetDataArray(const TArray<FAssetData>& AssetDataArray);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Move Files", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void MoveFolderFiles(const FString& SourceDirectory, const FString& DestinationDirectory);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "FixUp Redirectors In Folder", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void FixUpRedirectorsInFolder(const FString& Path);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "MoveAssetAndCleanupRedirector", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static bool MoveAssetAndCleanupRedirector(const FString& SourcePath, const FString& TargetPath);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "FixUpRedirectorForAsset", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void FixUpRedirectorForAsset(const FString& AssetPath);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Open Editor For Asset", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void OpenEditorForAsset(UObject* Asset);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Select Asset In Browser", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static void SelectAssetInBrowser(UObject* Asset);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Select Assets In Browser", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
	static void SelectAssetsInBrowser(const TArray<UObject*>& Assets);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Update Texture", Keywords = "AssetCheckTool sample test testing"), Category = "AssetCheckTool")
		static bool UpdateTexture(UTexture2D* Texture);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Texture Source Path"), Category = "AssetCheckTool")
	static 	FString GetTextureSourcePath(UTexture* Texture);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetActorsWithStaticMesh"), Category = "AssetCheckTool")
	static 	TArray<AActor*> GetActorsWithStaticMesh(UStaticMesh* Mesh);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "FindActorsByStaticMesh"), Category = "AssetCheckTool")
	static TArray<AActor*> FindActorsByStaticMesh(UStaticMesh* Mesh,const TArray<UClass*>& AllowedClasses);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetSelectedPathInContentBrowser"), Category = "AssetCheckTool")
	static 	TArray<FString> GetSelectedPathsInContentBrowser();


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetAssetSourceControlStatus"), Category = "AssetCheckTool")
	static void GetAssetSourceControlStatus(
	const FString& PathName,
	bool bUpdateStatus,
	bool& OutIsCurrent,
	bool& OutIsCheckedOutByOthers,
	FString& OutCheckedOutByOtherUserName,
	bool& OutIsCheckedOutBySelf,
	bool& OutIsAdded,
	bool& OutIsUnknown);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetAssetFileDiskPath"), Category = "AssetCheckTool")
	static FString GetAssetFileDiskPath(const FString& AssetPath);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UpdateAssetSourceControlStatus"), Category = "AssetCheckTool")
	static void UpdateAssetSourceControlStatus(
		const TArray<FString>& PathNames);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "DeleteEmptyFolder"), Category = "AssetCheckTool")
	static bool DeleteEmptyFolder(const FString& FolderPath);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ConvertResourcePath"), Category = "AssetCheckTool")
	static FString ConvertResourcePath(const FString& ResourcePath);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetMetaData"), Category = "AssetCheckTool")
	static FString GetMetaData(const FString& ObjectPath, const FString& Key);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetEnumNames"), Category = "AssetCheckTool")
	static void GetEnumNames(const UEnum* InEnum, TArray<FString>& OutNames);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetEnumTips"), Category = "AssetCheckTool")
	static void GetEnumTips(const UEnum* InEnum, TArray<FString>& OutNames);






	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SortByElementAtIndex"), Category = "AssetCheckTool")
	static TArray<FString> SortByElementAtIndex(const TArray<FString>& StringArray, const FString& SplitString, int32 Id);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ParseStringArrayToListStruct"), Category = "AssetCheckTool")
	static TArray<FKeyValueRow> ParseStringArray(const TArray<FString>& StringArray, const FString& Delimiter) ;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SortKeyValueRowsById"), Category = "AssetCheckTool")
	static TArray<FKeyValueRow> SortKeyValueRowsByColumn(
	TArray<FKeyValueRow> KeyValueRows,
	int32 ColumnIndex,
	bool bDescending,
	TArray<int32>& SortedRowIDs);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "FindReferencingPackages"), Category = "AssetCheckTool")
	static TArray<FString> FindReferencingPackages(const FString& AssetPath);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "FindAssetsReferencingPackages"), Category = "AssetCheckTool")
	static TArray<FString> FindAssetsReferencingPackages(const TArray<FString>& AssetPaths);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetAssetData"), Category = "AssetCheckTool")
	static bool GetAssetData(const FString& PathName, FAssetData& OutAssetData);



	UFUNCTION(BlueprintCallable, meta = (DisplayName = "NavigateToFolder"), Category = "AssetCheckTool")
	static void NavigateToFolder(const FString& FolderPath);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetSubFolders"), Category = "AssetCheckTool")
	static TArray<FString> GetSubFolders(const FString& Path, bool bRecursive);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetSubFoldersByArray"), Category = "AssetCheckTool")
	static TArray<FString> GetSubFoldersByArray(const TArray<FString>& Paths, bool bRecursive);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "MoveAsset"), Category = "AssetCheckTool")
	static bool MoveAsset(const FString& SourcePath, const FString& TargetPath);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "MoveAssets"), Category = "AssetCheckTool")
	static void MoveAssets(const TArray<FString>& SourcePaths, const TArray<FString>& TargetPaths);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetClassPackage"), Category = "AssetCheckTool")
	static UPackage* GetClassPackage(TSubclassOf<UObject> InClass);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetCurrentDirectory"), Category = "AssetCheckTool")
	static FString GetCurrentDirectory();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "CompareStrings"), Category = "AssetCheckTool")
	static void CompareStrings(const FString& String1, const FString& String2, bool bIgnoreCase, int32& OutLastMatchIndex, FString& OutMatchingSubstring);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "RenameAssetsWithDialog"), Category = "AssetCheckTool")
	static EAssetRenameResult RenameAssetsWithDialog(const TArray<FAssetRenameData>& AssetsAndNames, bool bAutoCheckout);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "RenameAssets"), Category = "AssetCheckTool")
	static bool RenameAssets(const TArray<FAssetRenameData>& AssetsAndNames);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "RenameAssetsWithRetries"), Category = "AssetCheckTool")
	static bool RenameAssetsWithRetries(const TArray<FAssetRenameData>& AssetsAndNames, int32 MaxRetries);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "RenameAssetsWithDialogWithRetries"), Category = "AssetCheckTool")
	static EAssetRenameResult RenameAssetsWithDialogWithRetries(const TArray<FAssetRenameData>& AssetsAndNames, bool bAutoCheckout, int32 MaxRetries);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "CompareStringsSimilarity"), Category = "AssetCheckTool")
	static float CompareStringsSimilarity(const FString& StringA, const FString& StringB, bool bCaseSensitive);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ComputeLevenshteinDistance"), Category = "AssetCheckTool")
	static int32 ComputeLevenshteinDistance(const FString& S1, const FString& S2);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetAllLevelActorsEditable"), Category = "AssetCheckTool")
	static TArray<AActor*> GetAllLevelActorsEditable();



	UFUNCTION(BlueprintCallable, meta = (DisplayName = "RenameDirectory"), Category = "AssetCheckTool")
	static bool RenameDirectory(const FString& OriginalPath, const FString& TargetPath);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ConvertVirtualToAbsolutePath"), Category = "AssetCheckTool")
	static FString ConvertVirtualToAbsolutePath(const FString& VirtualPath);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "RefreshFolder"), Category = "AssetCheckTool")
	static void RefreshFolder(const FString& FolderPath);
	static TArray<FMoveAction> MoveAction;


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "RegexMatch"), Category = "AssetCheckTool")
	static bool RegexMatch(const FString& InputString, const FString& Pattern, TArray<FString>& OutMatches, bool bCaseSensitive);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "RegexMatchWithScore"), Category = "AssetCheckTool")//1.PrefixWeight  2.CalculateJaccard 3.NumericMatch
	static bool RegexMatchWithScore(const FString& InputString, const FString& Pattern,bool bCaseSensitive, float& MatchScore, const FString& Delimiter = "_",const float& math1Wight = 0.7f,const float& math2Wight = 0.2f,const float& math3Wight = 0.1f);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "CheckNumericMatch"), Category = "AssetCheckTool")
	static float CheckNumericMatch(const FString& Str1, const FString& Str2);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "JaroWinklerDistance"), Category = "AssetCheckTool")
	static float JaroWinklerDistance(const FString& StringA, const FString& StringB);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "CalculateJaccardSimilarity"), Category = "AssetCheckTool")
	static float CalculateJaccardSimilarity(const FString& StringA, const FString& StringB);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SmithWatermanAlignment"), Category = "AssetCheckTool")
	static float SmithWatermanAlignment(const FString& StringA, const FString& StringB);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PrefixWeightedSimilarity"), Category = "AssetCheckTool")
	static float PrefixWeightedSimilarity(const FString& StringA, const FString& StringB);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetFuzzyMatchScore"), Category = "AssetCheckTool")
	static float GetFuzzyMatchScore(const FString& InputString, const FString& TargetString);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GenerateNGrams"), Category = "AssetCheckTool")
	static TSet<FString> GenerateNGrams(const FString& Input, int32 N);



	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetUniqueFilename"), Category = "AssetCheckTool")
	static FString GetUniqueFilename(const FString& InputPath);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "FindFilePaths"), Category = "AssetCheckTool")
	static TArray<FString> FindFilePaths(const FString& DirectoryPath, bool bRecursive, const FString& FileExtension);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "FindDirectoryInternal"), Category = "AssetCheckTool")
		static void FindDirectoryInternal(const FString& DirectoryPath, bool bRecursive, const FString& FileExtension, TArray<FString>& OutFiles);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "RegexMatchAndReplace"), Category = "AssetCheckTool")
	static void RegexMatchAndReplace(
	const FString& InputPath,
	const FString& Pattern,
	FString& OutputPath,
	float math1Wight,
	float math2Wight,
	float math3Wight);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "RenameFolder"), Category = "AssetCheckTool")
	static bool RenameFolders(const FString& DestPath, const FString& SourcePath);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetSelectedAssetByPath"), Category = "AssetCheckTool")
	static void SetSelectedAssetByPath(const TArray<FString> AssetPathArray);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetLightMapResolution"), Category = "AssetCheckTool")
	static float GetLightMapResolution(UStaticMesh* StaticMesh);



	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetVisibleUVSetCount"), Category = "AssetCheckTool")
	static int32 GetVisibleUVSetCount(UStaticMesh* StaticMesh, int32 LODIndex, float Tolerance = 0.01f);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetVisibleUVSetCountForSkeletalMesh"), Category = "AssetCheckTool")
	static int32 GetVisibleUVSetCountForSkeletalMesh(USkeletalMesh* SkeletalMesh, int32 LODIndex, float Tolerance);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ReplaceTextureInMaterial"), Category = "AssetCheckTool")
	static int32 ReplaceTexturesInMaterial(UMaterialInterface* Material, const TArray<FString>& PathsToReplace, UTexture* DefaultTexture);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "JumpToMaterialNode"), Category = "AssetCheckTool")
	static void JumpToMaterialNode(UMaterialInterface* Material, const FString& NodeName);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "OpenMaterialEditorAtNode"), Category = "AssetCheckTool")
	static bool OpenMaterialEditorAtNode(UMaterialInterface* Material, const FString& NodeName);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ExecuteSearchInMaterialEditor"), Category = "AssetCheckTool")
	static void ExecuteSearchInMaterialEditor(const FString& SearchText);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "CreateFoliageTypeForStaticMesh"), Category = "AssetCheckTool")
	static UFoliageType_InstancedStaticMesh* CreateFoliageTypeForStaticMesh(UStaticMesh* StaticMesh, FString& OutPackagePath);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetMobility"), Category = "AssetCheckTool")
	static void SetMobility(UFoliageType* FoliageType,EComponentMobility::Type Mobility);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetLightmapType"), Category = "AssetCheckTool")
	static void SetLightmapType(UFoliageType* FoliageType, ELightmapType LightmapType);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetComponnentLightmapType"), Category = "AssetCheckTool")
	static void SetComponnentLightmapType(UPrimitiveComponent* Component, ELightmapType LightmapType);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetLightmapTypetest"), Category = "AssetCheckTool")
	static void  SetLightmapTypetest(UFoliageType_InstancedStaticMesh* FoliageTypeInput);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ReParentMaterialInstances"), Category = "AssetCheckTool")
	static void ReParentMaterialInstances(const TArray<UMaterialInstanceConstant*>& MaterialInstances, UMaterialInterface* NewParentMaterial);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "TransferMaterialParameters"), Category = "AssetCheckTool")
	static bool TransferMaterialParameters(
	FString SourcePath,
	FString TargetPath,
	const TArray<FString>& ExcludeKeywords);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LoadMatInstByPath"), Category = "AssetCheckTool")
	static UMaterialInstanceConstant* LoadMatInstByPath(const FString& Path);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetAssetClassByPath"), Category = "AssetCheckTool")
	static UClass* GetAssetClassByPath(const FString& AssetPath);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetAssetClassByPath"), Category = "AssetCheckTool")
	static AInstancedFoliageActor* GetPartitionFoliageActor(AActor* StaticMeshActor);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetLevelFoliageActor"), Category = "AssetCheckTool")
	static AInstancedFoliageActor* GetLevelFoliageActor(FString levelname);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetActorByActorFullName"), Category = "AssetCheckTool")
	static AActor* GetActorByActorFullName(const FString& InName);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetComponentByComponentFullName"), Category = "AssetCheckTool")
	static UActorComponent* GetComponentByComponentFullName(const FString& InName);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "FindMaterialWorldPositionExpression"), Category = "AssetCheckTool")
	static int32 FindMaterialWorldPositionExpression(const FString& MaterialPath, FString& OutDetails);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "RecursiveGetAssetsDependencies"), Category = "AssetCheckTool")
	static TArray<FString> RecursiveGetAssetsDependencies(const TArray<FString>& PackageNames,FString& OutPackagePath);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "RecursiveGetAssetDependencies"), Category = "AssetCheckTool")
	static TArray<FString> RecursiveGetAssetDependencies(const FString& PackageName);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetParentClass"), Category = "AssetCheckTool")
	static UClass* GetParentClass(UObject* Object);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetParentClassChain"), Category = "AssetCheckTool")
	static TArray<UClass*> GetParentClassChain(UObject* Object);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "CancelMaterialInstanceOverride"), Category = "AssetCheckTool")
	static void CancelMaterialInstanceOverride(UMaterialInstanceConstant* MaterialInst,const FString& ParameterName);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SplitStringByDelimiter"), Category = "AssetCheckTool")
	static TArray<FString> SplitStringByDelimiter(const FString& FileContent, const FString& Delimiter = TEXT("\n"));

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "RecursiveGetAssetDependenciesWithClass"), Category = "AssetCheckTool")
	static TArray<FString> RecursiveGetAssetDependenciesWithClass(
	const FString& PackageName,
	const TArray<UClass*>& FilterClasses,
	const TArray<FString>& ExcludeKeywords);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "InsertSkeletalMeshLODs"), Category = "AssetCheckTool")
	static void InsertSkeletalMeshLODs(USkeletalMesh* SkeletalMesh,USkeletalMesh* LOD0);

	static bool SetCustomLOD(USkeletalMesh* DestinationSkeletalMesh, USkeletalMesh* SourceSkeletalMesh, const int32 LodIndex, const int32 SrcLodIndex,const FString& SourceDataFilename,const bool Save);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ReplaceSKMReferences"), Category = "AssetCheckTool")
	static void ReplaceSKMReferences(UObject* Source, UObject* Dest);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "FixUpRedirectorInAssetsFolder"), Category = "AssetCheckTool")
	static void FixUpRedirectorInAssetsFolder(TArray<UObject*> Assets);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "IsNaniteEnabled"), Category = "AssetCheckTool")
	static bool IsNaniteEnabled(UStaticMesh* StaticMesh);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ReloadAsset"), Category = "AssetCheckTool")
	static void ReloadAsset(UObject* Object);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetSameSourceActors"), Category = "AssetCheckTool")
	static void GetSameSourceActors(AActor* SourceActor, TArray<AActor*>& SameSourceActors, bool bIncludeSelf);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "FixAllGroupActors"), Category = "AssetCheckTool")
	static void FixAllGroupActors();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "FindRelatedGroupActors"), Category = "AssetCheckTool")
	static TArray<AGroupActor*> FindRelatedGroupActors(const TArray<AActor*>& SelectedActors);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "FixGroupActors"), Category = "AssetCheckTool")
	static void FixGroupActors(const TArray<AGroupActor*>& GroupActors);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "RemoveActorsFromGroupsAndDestroy"), Category = "AssetCheckTool")
	static void RemoveActorsFromGroupsAndDestroy(const TArray<AActor*>& ActorsToRemove);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "IsGroupActorEmpty"), Category = "AssetCheckTool")
	static bool IsGroupActorEmpty(AGroupActor* GroupActor);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "CheckFXCollision"), Category = "AssetCheckTool")
	static bool CheckFXCollision(UObject* Object);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ClearFXCollision"), Category = "AssetCheckTool")
	static void ClearFXCollision(UObject* Object);
private:
	static void RecursiveGetDependencies(const FName& PackageName, TSet<FName>& AllDependencies, TSet<FString>& ExternalObjectsPaths, TSet<FName>& ExcludedDependencies, const TFunction<bool(FName)>& ShouldExcludeFromDependenciesSearch);
};
