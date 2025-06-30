// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetAction/CollisionCheckWidget.h"

#include "PaperSprite.h"
#include "PhysicsEngine/PhysicsAsset.h"

bool UCollisionCheckWidget::CheckFXCollision(UObject* Object)
{
	if( Object->IsA(UStaticMesh::StaticClass()) )
	{
		UStaticMesh* StaticMesh = Cast<UStaticMesh>(Object);
		if(!StaticMesh)return false;

		UBodySetup* BodySetup = StaticMesh->GetBodySetup();
		if (!BodySetup)return false;

		int32 Count = BodySetup->AggGeom.BoxElems.Num();
		Count += BodySetup->AggGeom.SphereElems.Num();
		Count += BodySetup->AggGeom.SphylElems.Num();
		Count += BodySetup->AggGeom.ConvexElems.Num();
		return Count > 0 ;
	}
	else if(Object->IsA(USkeletalMesh::StaticClass()))
	{
		USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(Object);
		if(!SkeletalMesh)return false;
		UPhysicsAsset* PhysicsAsset = SkeletalMesh->GetPhysicsAsset();
		if(!PhysicsAsset)return false;

		return PhysicsAsset->SkeletalBodySetups.Num()>0;
	}
	else if(Object->IsA(UPaperSprite::StaticClass()))
	{
		UPaperSprite* PaperSprite = Cast<UPaperSprite>(Object);
		if(!PaperSprite)return false;

		// 获取 UClass 对象
		UClass* SpriteClass = PaperSprite->GetClass();

		// 获取 CollisionGeometry 属性
		FProperty* CollisionGeometryProperty = SpriteClass->FindPropertyByName(TEXT("CollisionGeometry"));
		if (!CollisionGeometryProperty)return false;
		
		// 使用反射获取 CollisionGeometry
		void* CollisionGeometryPtr = CollisionGeometryProperty->ContainerPtrToValuePtr<void>(PaperSprite);
    
		// 确保获取到了 CollisionGeometry
		if (!CollisionGeometryPtr)return false;
		FSpriteGeometryCollection* CollisionGeometry = reinterpret_cast<FSpriteGeometryCollection*>(CollisionGeometryPtr);
				
		return  CollisionGeometry->Shapes.Num() > 0;
	}
	
	return false;
}

void UCollisionCheckWidget::ClearFXCollision(UObject* Object)
{
	
	if( Object->IsA(UStaticMesh::StaticClass()) )
	{
		UStaticMesh* StaticMesh = Cast<UStaticMesh>(Object);
		if(!StaticMesh)return;
		
		// Close the mesh editor to prevent crashing. Reopen it after the mesh has been built.
		UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
		bool bStaticMeshIsEdited = false;
		if (AssetEditorSubsystem->FindEditorForAsset(StaticMesh, false))
		{
			AssetEditorSubsystem->CloseAllEditorsForAsset(StaticMesh);
			bStaticMeshIsEdited = true;
		}

		StaticMesh->GetBodySetup()->Modify();

		// Remove simple collisions
		StaticMesh->GetBodySetup()->RemoveSimpleCollision();

		// refresh collision change back to static mesh components
		//RefreshCollisionChange(*StaticMesh);

		// Request re-building of mesh with new collision shapes
		StaticMesh->PostEditChange();

		// Reopen MeshEditor on this mesh if the MeshEditor was previously opened in it
		if (bStaticMeshIsEdited)
		{
			AssetEditorSubsystem->OpenEditorForAsset(StaticMesh);
		}
	}
	else if(Object->IsA(USkeletalMesh::StaticClass()))
	{
		USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(Object);
		if(!SkeletalMesh)return ;
		UPhysicsAsset* PhysicsAsset = SkeletalMesh->GetPhysicsAsset();
		if(!PhysicsAsset)return ;
		
		
		PhysicsAsset->ConstraintSetup.Empty();
		PhysicsAsset->SkeletalBodySetups.Empty();
		
		PhysicsAsset->UpdateBodySetupIndexMap();
		PhysicsAsset->UpdateBoundsBodiesArray();
		
		PhysicsAsset->PostEditChange();

		PhysicsAsset->MarkPackageDirty();
	}
	else if(Object->IsA(UPaperSprite::StaticClass()))
	{
		UPaperSprite* PaperSprite = Cast<UPaperSprite>(Object);
		if(!PaperSprite)return ;

		// 获取 UClass 对象
		UClass* SpriteClass = PaperSprite->GetClass();

		// 获取 CollisionGeometry 属性
		FProperty* CollisionGeometryProperty = SpriteClass->FindPropertyByName(TEXT("CollisionGeometry"));
		if (!CollisionGeometryProperty)return ;
		
		// 使用反射获取 CollisionGeometry
		void* CollisionGeometryPtr = CollisionGeometryProperty->ContainerPtrToValuePtr<void>(PaperSprite);
    
		// 确保获取到了 CollisionGeometry
		if (!CollisionGeometryPtr)return ;
		FSpriteGeometryCollection* CollisionGeometry = reinterpret_cast<FSpriteGeometryCollection*>(CollisionGeometryPtr);
				
		CollisionGeometry->Shapes.Empty();
		PaperSprite->MarkPackageDirty();
	}
}
