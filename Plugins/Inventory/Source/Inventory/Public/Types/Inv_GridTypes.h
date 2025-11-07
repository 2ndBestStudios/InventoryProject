#pragma once

#include "Inv_GridTypes.generated.h"

class UInv_InventoryItem;
// Defines the different item categories we can have 
UENUM(BlueprintType)
enum class EInv_ItemCategory : uint8
{
	Equippable,
	Consumable,
	Craftable,
	None 
};

// Checks individual slot
USTRUCT()
struct FInv_SlotAvailability
{
	GENERATED_BODY()

	FInv_SlotAvailability() {}
	FInv_SlotAvailability(int32 ItemIndex, int32 Room, bool bHasItem) : Index(ItemIndex), AmountToFill(Room), bItemAtIndex(bHasItem) {}
	
	// Checks slot index, how much to fill, and if there's already an item there 
	int32 Index{INDEX_NONE};
	int32 AmountToFill{0};
	bool bItemAtIndex{false}; 
};

// Checks overall availability of slots 
USTRUCT()
struct FInv_SlotAvailabilityResult
{
	GENERATED_BODY()

	FInv_SlotAvailabilityResult() {}

	// Tells what the item is, how big, if there's any remaining, and if it's stackable 
	TWeakObjectPtr<UInv_InventoryItem> Item;
	int32 TotalRoomToFill{0}; 
	int32 Remainder{0}; 
	bool bStackable{false};
	// Holds array of individual slot availabilities 
	TArray<FInv_SlotAvailability> SlotAvailabilities; 
};
