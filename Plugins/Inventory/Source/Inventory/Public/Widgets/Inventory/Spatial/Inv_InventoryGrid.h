// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Types/Inv_GridTypes.h"
#include "Widgets/Inventory/GridSlots/Inv_GridSlot.h"
#include "Widgets/Inventory/SlottedItems/Inv_SlottedItem.h"
#include "Inv_InventoryGrid.generated.h"

class UInv_HoverItem;
struct FInv_ImageFragment;
struct FInv_GridFragment;
class UInv_SlottedItem;
struct FInv_ItemManifest;
class UInv_ItemComponent;
class UInv_InventoryItem;
class UInv_InventoryComponent;
class UCanvasPanel;
class UInv_GridSlot;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_InventoryGrid : public UUserWidget
{
	GENERATED_BODY()

public:
	// Setups inital values 
	virtual void NativeOnInitialized() override; 
	
	// Getter for item category 
	EInv_ItemCategory GetItemCategory() const { return ItemCategory; }

	// Overload for HasRoomForItem
	FInv_SlotAvailabilityResult HasRoomForItem(const UInv_ItemComponent* ItemComponent);

	// Callback function for the additem delegate from inventory component 
	UFUNCTION()
	void AddItem(UInv_InventoryItem* Item);  
	
private:
	// Constructs item grid 
	void ConstructGrid(); 

	// Handles matching item category to inventory item 
	bool MatchesCategory(const UInv_InventoryItem* Item) const;
	
	// Overload for checking HasRoomForItem
	FInv_SlotAvailabilityResult HasRoomForItem(const UInv_InventoryItem* Item);
	FInv_SlotAvailabilityResult HasRoomForItem(const FInv_ItemManifest& Manifest);

	// Function to start performing actions on our item 
	void AddItemToIndices(const FInv_SlotAvailabilityResult& Result, UInv_InventoryItem* NewItem);
	
	// Adds an item to inventory index 
	void AddItemAtIndex(UInv_InventoryItem* Item, const int32 Index, const bool bStackable, const int32 StackAmount);
	
	// Creates individual GridSlot item 
	UInv_SlottedItem* CreateSlottedItem(UInv_InventoryItem* Item, const bool bStackable, const int32 StackAmount, const FInv_GridFragment* GridFragment, const FInv_ImageFragment* ImageFragment, const int32 Index);
	
	// Sets ItemImage for GridSlot item based on GridFragment and ImageFragment 
	void SetSlottedItemImage(const UInv_SlottedItem* SlottedItem, const FInv_GridFragment* GridFragment, const FInv_ImageFragment* ImageFragment) const;
	
	// Gets the DrawSize of the image based on GridSlot values 
	FVector2D GetDrawSize(const FInv_GridFragment* GridFragment) const ;

	// Function for adding items to canvas panel 
	void AddSlottedItemToCanvas(const int32 Index, const FInv_GridFragment* GridFragment, UInv_SlottedItem* SlottedItem) const;

	// Updates the properties of a GridSlot 
	void UpdateGridSlots(UInv_InventoryItem* NewItem, const int32 Index, bool bStackableItem, const int32 StackAmount);

	// Checks if a current index is already filled 
	bool IsIndexClaimed(const TSet<int32>& CheckedIndices, const int32 Index) const;

	// Backbone of Entire Project : Checks if there is room at index 
	bool HasRoomAtIndex(const UInv_GridSlot* GridSlot,
						const FIntPoint& Dimensions,
						const TSet<int32>& CheckedIndices,
						TSet<int32>& OutTentativelyClaimedIndices,
						const FGameplayTag& ItemType,
						const int32 MaxStackSize);

	// Checks SlotConstraints 
	bool CheckSlotConstraints (const UInv_GridSlot* GridSLot,
								const UInv_GridSlot* SubGridSlot,
								const TSet<int32>& CheckedIndices,
								TSet<int32>& OutTentativelyClaimedIndices,
								const FGameplayTag& ItemType,
								const int32 MaxStackSize) const;

	// Checks GridSlot for InventoryItem 
	bool HasValidItem(const UInv_GridSlot* GridSlot) const;

	// Checks if GridSlot & SubGridSlot match 
	bool IsUpperLeftSlot(const UInv_GridSlot* GridSlot, const UInv_GridSlot* SubGridSlot) const;

	// Checks if new item matches existing item 
	bool DoesItemTypeMatch(const UInv_InventoryItem* SubItem, const FGameplayTag& ItemType) const;

	// Check if item is in GridBounds 
	bool IsInGridBounds(const int32 StartIndex, const FIntPoint& ItemDimensions) const;

	// Checks for right click 
	bool IsRightClicked(const FPointerEvent& MouseEvent) const;

	// Checks for left click 
	bool IsLeftClicked(const FPointerEvent& MouseEvent) const;

	// Called whenever an inventory item is clicked & picked up 
	void PickUp(UInv_InventoryItem* ClickedInventoryItem, const int32 GridIndex);

	// Assigns HoverItem
	void AssignHoverItem(UInv_InventoryItem* InventoryItem);

	// Overload for assigning HoverItem. Allows us to set previous index and update stack count 
	void AssignHoverItem(UInv_InventoryItem* InventoryItem, const int32 GridIndex, const int32 PreviousGridIndex);

	// Removes item from grid 
	void RemoveItemFromGrid(UInv_InventoryItem* InventoryItem, const int32 GridIndex);

	// Determine how much space is in a given GridSlot 
	int32 DetermineFillAmountForSlot(const bool bStackable, const int32 MaxStackSize, const int32 AmountToFill, const UInv_GridSlot* GridSlot) const;

	// Retrieve the StackAmount from a GridSlot 
	int32 GetStackAmount(const UInv_GridSlot* GridSlot) const;

	// Function that responds to on StackChange delegate 
	UFUNCTION()
	void AddStacks(const FInv_SlotAvailabilityResult& Result);

	// Function that responds to the SlottedItemClicked delegate 
	UFUNCTION()
	void OnSlottedItemClicked(int32 GridIndex, const FPointerEvent& MouseEvent); 

	// Checks item manifest for item dimensions 
	FIntPoint GetItemDimensions(const FInv_ItemManifest& Manifest) const;
	
	// Gives a reference for the InventoryComponent 
	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent;
	
	// Creates ItemCategory 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category="Inventory")
	EInv_ItemCategory ItemCategory;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_GridSlot> GridSlotClass;

	// Allows us to place the created widgets onto a canvas 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;

	// Class used for drawing GridSlot widget 
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_SlottedItem> SlottedItemClass;

	// Map of all SlottedItems in inventory 
	UPROPERTY()
	TMap<int32, TObjectPtr<UInv_SlottedItem>> SlottedItems; 
	
	UPROPERTY()
	TArray<TObjectPtr<UInv_GridSlot>> GridSlots;

	// Sets inventory grid size 
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 Rows;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 Columns;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float TileSize;


	// Contains HoverItemClass
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_HoverItem> HoverItemClass;

	// Reference to HoverItem
	UPROPERTY()
	TObjectPtr<UInv_HoverItem> HoverItem; 
	
};
