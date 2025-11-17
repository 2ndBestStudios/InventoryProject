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

UENUM(BlueprintType)
enum class EInv_TileQuadrant : uint8
{
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight,
	None
};

USTRUCT(BlueprintType)
struct FInv_TileParameters
{
	GENERATED_BODY()

	// Lets us know where we are on the Grid 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Inventory")
	FIntPoint TileCoordinates{};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Inventory")
	int32 TileIndex{INDEX_NONE};

	// Lets us know where we are on the actual GridSlot 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Inventory")
	EInv_TileQuadrant TileQuadrant{EInv_TileQuadrant::None};
};

inline bool operator==(const FInv_TileParameters A, const FInv_TileParameters B)
{
	return A.TileCoordinates == B.TileCoordinates && A.TileIndex == B.TileIndex && A.TileQuadrant == B.TileQuadrant;
}
