// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Inv_InventoryItem.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_InventoryItem : public UObject
{
	GENERATED_BODY()

public:
	// Needed for replicating subobjects 
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }

	// Sets the ItemManifest 
	void SetItemManifest(const FInv_ItemManifest Manifest);

	// Getter for ItemManifest. One returns a reference that cannot be changed. The other returns one that can be changed
	const FInv_ItemManifest& GetItemManifest() const { return ItemManifest.Get<FInv_ItemManifest>(); }
	FInv_ItemManifest& GetItemManifestMutable() { return ItemManifest.GetMutable<FInv_ItemManifest>(); }
	
	// Helper function to check StackableFragment 
	bool IsStackable() const;

	// Getter and setter functions 
	int32 GetTotalStackCount() const {return TotalStackCount;}
	void SetTotalStackCount(int32 Count) {TotalStackCount = Count;}

private:

	// Sets available item properties 
	UPROPERTY(VisibleAnywhere, meta=(BaseStruct = "/Script/Inventory.Inv_ItemManifest"), Replicated)
	FInstancedStruct ItemManifest;

	// Exposes TotalStackCount of item 
	UPROPERTY(Replicated)
	int32 TotalStackCount{0};
};

// Template function to GetFragment of an item by the GameplayTag returning the FragmentType  
template <typename FragmentType>
const FragmentType* GetFragment(const UInv_InventoryItem* Item, const FGameplayTag& Tag)
{
	// Checks if valid and then gets ItemManifest 
	if (!IsValid(Item)) return nullptr;
	const FInv_ItemManifest& Manifest = Item->GetItemManifest();
	
	// Calls template function on ItemManifest to get FragmentType 
	return Manifest.GetFragmentOfTypeWithTag<FragmentType>(Tag);
	
}