#pragma once

#include "CoreMinimal.h"
#include "Types/Inv_GridTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "GameplayTagContainer.h"
#include "Inv_ItemManifest.generated.h"

// The Item manifest contains all the necessary data for creating a new Inventory Item

class UInv_InventoryItem;
struct FInv_ItemFragment;

USTRUCT(BlueprintType)
struct INVENTORY_API FInv_ItemManifest
{
	GENERATED_BODY()

	// Creates a newobject 
	UInv_InventoryItem* Manifest(UObject* NewOuter);
	// Returns item category 
	EInv_ItemCategory GetItemCategory() const { return ItemCategory; }
	
private:

	// Holds an array of instanced struct item fragments 
	UPROPERTY(EditAnywhere, Category="Inventory", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FInv_ItemFragment>> Fragments;
	
	// Assigns item category 
	UPROPERTY(EditAnywhere, Category = "Inventory")
	EInv_ItemCategory ItemCategory{EInv_ItemCategory::None};

	// Gameplay tag accessibility is added to item manifest 
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag ItemType; 
};

