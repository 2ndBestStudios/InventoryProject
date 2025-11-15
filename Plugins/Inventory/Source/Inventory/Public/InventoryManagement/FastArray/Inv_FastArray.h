#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "Inv_FastArray.generated.h"

struct FGameplayTag;
class UInv_InventoryComponent;
class UInv_InventoryItem;
class UInv_ItemComponent;

// A single entry in the inventory 
USTRUCT(BlueprintType)
struct FInv_InventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FInv_InventoryEntry() {} 

private:
	// allows private access to these files 
	friend struct FInv_InventoryFastArray; 
	friend UInv_InventoryComponent;

	// pointer to actual object 
	UPROPERTY()
	TObjectPtr<UInv_InventoryItem> Item = nullptr;
};

// A list of inventory items. The actual Fast array serializer
USTRUCT(BlueprintType)
struct FInv_InventoryFastArray : public FFastArraySerializer
{
	GENERATED_BODY()

	FInv_InventoryFastArray() : OwnerComponent(nullptr) {}
	FInv_InventoryFastArray(UActorComponent* InOwnerComponent) : OwnerComponent(InOwnerComponent) {}

	TArray<UInv_InventoryItem*> GetAllItems() const; 

	// FFastArraySerializer contract
	// Called before items are removed from array. Good for cleaning up UI 
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	// Called after new items are added to the array. Good for creating UI 
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	// End of FFastArraySerializer contract

	// Required for serialization 
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FInv_InventoryEntry, FInv_InventoryFastArray>(Entries, DeltaParams, *this); 
	}

	UInv_InventoryItem* AddEntry(UInv_ItemComponent* ItemComponent);
	UInv_InventoryItem* AddEntry(UInv_InventoryItem* Item);
	void RemoveEntry(UInv_InventoryItem* Item);

	UInv_InventoryItem* FindFirstItemByType(const FGameplayTag& ItemType);
	
private:
	friend UInv_InventoryComponent;
	
	// Replicated list of items 
	UPROPERTY()
	TArray<FInv_InventoryEntry> Entries;

	// Pointer to the component that owns this array 
	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent; 
};

// Required boilerplate 
template<>
struct TStructOpsTypeTraits<FInv_InventoryFastArray> : TStructOpsTypeTraitsBase2<FInv_InventoryFastArray>
{
	enum { WithNetDeltaSerializer = true};
};