// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/Inv_GridTypes.h"
#include "Widgets/Inventory/SlottedItems/Inv_SlottedItem.h"
#include "Inv_InventoryGrid.generated.h"

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

	// Updates the background of the GridSlot 
	void UpdateGridSlots(UInv_InventoryItem* NewItem, const int32 Index); 
	
	// Gives a reference for the inventory component 
	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent;
	
	// Creates item category 
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
	
};
