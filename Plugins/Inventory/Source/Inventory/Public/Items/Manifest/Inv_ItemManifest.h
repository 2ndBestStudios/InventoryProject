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

	// Creates a NewObject
	UInv_InventoryItem* Manifest(UObject* NewOuter);
	
	// Returns ItemCategory 
	EInv_ItemCategory GetItemCategory() const { return ItemCategory; }
	
	// Returns GameplayTag
	FGameplayTag GetItemType() const { return ItemType; }
	
	// Template to get fragments by comparing with tags
	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	const T* GetFragmentOfTypeWithTag(const FGameplayTag& FragmentTag) const;
	
	// Simpler template to get fragment 
	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	const T* GetFragmentOfType() const;

	// Mutable template to get fragment
	template<typename T> requires std::derived_from<T, FInv_ItemFragment>
	T* GetFragmentOfTypeMutable();
	
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

template<typename T>
requires std::derived_from<T, FInv_ItemFragment>
const T* FInv_ItemManifest::GetFragmentOfTypeWithTag(const FGameplayTag& FragmentTag) const 
{
	// Loops through entire Instanced Struct Array of Fragments 
	for (const TInstancedStruct<FInv_ItemFragment>& Fragment : Fragments)
	{
		// Returns the pointer for each fragment of the type fragment type searching for 
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			// Checks each fragment pointer and calls GetFragmentTag on ItemFragment, which then calls helper function for tags 
			if (!FragmentPtr->GetFragmentTag().MatchesTagExact(FragmentTag)) continue; 
			return FragmentPtr;
		}
	}
	
	return nullptr;
}

template <typename T> requires std::derived_from<T, FInv_ItemFragment>
const T* FInv_ItemManifest::GetFragmentOfType() const
{
	// Loops through entire Instanced Struct Array of Fragments 
	for (const TInstancedStruct<FInv_ItemFragment>& Fragment : Fragments)
	{
		// Returns the pointer for each fragment of the type fragment type searching for 
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			// Checks each fragment pointer and calls GetFragmentTag on ItemFragment, which then calls helper function for tags 
			return FragmentPtr;
		}
	}
	
	return nullptr;
}

template <typename T> requires std::derived_from<T, FInv_ItemFragment>
T* FInv_ItemManifest::GetFragmentOfTypeMutable()
{
	// Loops through entire Instanced Struct Array of Fragments 
	for (TInstancedStruct<FInv_ItemFragment>& Fragment : Fragments)
	{
		// Returns the pointer for each fragment of the type fragment type searching for 
		if (T* FragmentPtr = Fragment.GetMutablePtr<T>())
		{
			// Checks each fragment pointer and calls GetFragmentTag on ItemFragment, which then calls helper function for tags 
			return FragmentPtr;
		}
	}
	
	return nullptr;
}
