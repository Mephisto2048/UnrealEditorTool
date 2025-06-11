// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidget/PropertyMatrix.h"

#include "EditorAssetLibrary.h"
#include "IPropertyTable.h"

void SPropertyMatrixTab::Construct(const FArguments& InArgs)
{
	TArray<TSharedPtr<FAssetData>> StoredAssetData = InArgs._AssetDataToStore;
	TArray<UObject*> LoadedObjects;
	for (const TSharedPtr<FAssetData>& AssetDataPtr : StoredAssetData)
	{
		if (!AssetDataPtr.IsValid())  // 检查智能指针是否有效
			continue;

		const FAssetData& AssetData = *AssetDataPtr;  // 解引用获取 FAssetData
		UObject* LoadedObject = UEditorAssetLibrary::LoadAsset(AssetData.GetObjectPathString());  // 加载资产

		if (LoadedObject)  // 检查是否加载成功
		{
			LoadedObjects.Add(LoadedObject);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load asset: %s"), *AssetData.GetObjectPathString());
		}
	}
		
	FPropertyEditorModule& PropertyEditorModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

	PropertyTable = PropertyEditorModule.CreatePropertyTable();
	PropertyTable->SetObjects(LoadedObjects);
	
	ChildSlot
	[
		PropertyEditorModule.CreatePropertyTableWidget(PropertyTable.ToSharedRef())
		/*SNew(SScrollBox)
		//1.title
		+SScrollBox::Slot()
		[
			PropertyEditorModule.CreatePropertyTableWidget(PropertyTable.ToSharedRef())
		]*/
		
	];
}
