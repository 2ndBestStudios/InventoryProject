// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Items/Fragments/Inv_FragmentTags.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/GridSlots/Inv_GridSlot.h"
#include "Widgets/Utils/Inv_WidgetUtils.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Widgets/Inventory/SlottedItems/Inv_SlottedItem.h"

void UInv_InventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ConstructGrid();

	// Fills out inventory component reference and subscribes to callback 
	InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	InventoryComponent->OnItemAdded.AddDynamic(this, &ThisClass::AddItem); 
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_ItemComponent* ItemComponent)
{
	// Calls manifest version 
	return HasRoomForItem(ItemComponent->GetItemManifest());
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_InventoryItem* Item)
{
	// Calls manifest version 
	return HasRoomForItem(Item->GetItemManifest());
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const FInv_ItemManifest& Manifest)
{
	FInv_SlotAvailabilityResult Result;
 	Result.TotalRoomToFill = 1;

	FInv_SlotAvailability SlotAvailability;
	SlotAvailability.AmountToFill = 1;
	SlotAvailability.Index = 0;

	Result.SlotAvailabilities.Add(MoveTemp(SlotAvailability));
	
 	return Result; 
}

void UInv_InventoryGrid::AddItem(UInv_InventoryItem* Item)
{
	// Checks if categories match 
	if (!MatchesCategory(Item)) return;

	// Checks for Room for Items from Server broadcast 
	FInv_SlotAvailabilityResult Result = HasRoomForItem(Item);

	// Allows us to start performing some actions based on the item 
	AddItemToIndices(Result, Item);
}

void UInv_InventoryGrid::AddItemToIndices(const FInv_SlotAvailabilityResult& Result, UInv_InventoryItem* NewItem)
{
	// Loops through SlotAvailabilities from SlotAvailabilityResult. For each available slot it checks index, stackable, and amount to fill 
	for (const auto& Availability : Result.SlotAvailabilities)
	{
		AddItemAtIndex(NewItem, Availability.Index, Result.bStackable, Availability.AmountToFill);
	}
}

void UInv_InventoryGrid::AddItemAtIndex(UInv_InventoryItem* Item, const int32 Index, const bool bStackable,
	const int32 StackAmount)
{
	// Uses NewItem (InventoryItem) to get its ItemManifest, which then returns the GridFragment that was set
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(Item, FragmentTags::GridFragment);
	const FInv_ImageFragment* ImageFragment = GetFragment<FInv_ImageFragment>(Item, FragmentTags::IconFragment);
	if (!GridFragment || !ImageFragment) return;
	// Calls below function 
	UInv_SlottedItem* SlottedItem = CreateSlottedItem(Item, bStackable, StackAmount, GridFragment, ImageFragment, Index);
	// Add Item to canvas 
	AddSlottedItemToCanvas(Index, GridFragment, SlottedItem);
	// Adds Item to Map for data manipulation later 
	SlottedItems.Add(Index,SlottedItem);
}

UInv_SlottedItem* UInv_InventoryGrid::CreateSlottedItem(UInv_InventoryItem* Item, const bool bStackable,
	const int32 StackAmount, const FInv_GridFragment* GridFragment, const FInv_ImageFragment* ImageFragment,
	const int32 Index)
{
	// Creates a GridSlotItem Widget based on SlottedItem with owner set by PlayerController 
	UInv_SlottedItem* SlottedItem = CreateWidget<UInv_SlottedItem>(GetOwningPlayer(), SlottedItemClass);
	// Sets the inventory item for the GridSlot 
	SlottedItem->SetInventoryItem(Item);
	// Creates GridSlotItem image 
	SetSlottedItemImage(SlottedItem,GridFragment,ImageFragment);
	// Sets index of GridSlotItem 
	SlottedItem->SetGridIndex(Index);
	// Returns the Slotted Item
	return SlottedItem;
}

void UInv_InventoryGrid::SetSlottedItemImage(const UInv_SlottedItem* SlottedItem, const FInv_GridFragment* GridFragment,
                                             const FInv_ImageFragment* ImageFragment) const
{
	// Create an empty brush that is then set to the ImageFragment. Set brush value to draw as image 
	FSlateBrush Brush;
	Brush.SetResourceObject(ImageFragment->GetIcon());
	Brush.DrawAs = ESlateBrushDrawType::Image;
	// Calls below function
	Brush.ImageSize = GetDrawSize(GridFragment);
	// Calls the individual GridSlot item to set its own ImageBrush 
	SlottedItem->SetImageBrush(Brush);
}

FVector2D UInv_InventoryGrid::GetDrawSize(const FInv_GridFragment* GridFragment) const 
{
	// Takes TileSize of Grid, subtracts from padding of fragment, and multiples by 2 for either side 
	const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2;
	return GridFragment->GetGridSize() * IconTileWidth;
}

void UInv_InventoryGrid::AddSlottedItemToCanvas(const int32 Index, const FInv_GridFragment* GridFragment,
	UInv_SlottedItem* SlottedItem) const
{
	// Adds Child 
	CanvasPanel->AddChild(SlottedItem);
	// Sets the slot of the canvas panel, then draws based on GridFragmentSize 
	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(SlottedItem);
	CanvasSlot->SetSize(GetDrawSize(GridFragment));
	// Calculates DrawPosition based on Index and Column #. Then incorporates padding 
	const FVector2D DrawPos = UInv_WidgetUtils::GetPositionFromIndex(Index, Columns) * TileSize;
	const FVector2D DrawPosWithPadding = DrawPos + FVector2D(GridFragment->GetGridPadding());
	// Sets position 
	CanvasSlot->SetPosition(DrawPosWithPadding);
}

void UInv_InventoryGrid::ConstructGrid()
{
	// Assigns array size to prevent it from dynamically adjusting 
	GridSlots.Reserve(Rows * Columns);

	// loops through all rows and all columns 
	for (int32 j = 0; j < Rows; j++)
	{
		for (int32 i = 0; i < Columns; i++)
		{
			// creates a widget for each loop passthrough which is then added to canvas 
			UInv_GridSlot* GridSlot = CreateWidget<UInv_GridSlot>(this, GridSlotClass);
			CanvasPanel->AddChild(GridSlot);
			
			// helper function is called which creates index from row and column point
			// which is then passed through gridslot function to set its index
			const FIntPoint TilePosition(i, j); 
			GridSlot->SetTileIndex(UInv_WidgetUtils::GetIndexFromPosition(TilePosition, Columns));

			// sets the slot for the individual grid slot. then sets size and position of slot 
			UCanvasPanelSlot* GridCanvasPanelSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridSlot);
			GridCanvasPanelSlot->SetSize(FVector2D(TileSize));
			GridCanvasPanelSlot->SetPosition(FVector2D(TilePosition * TileSize));

			// add each created GridSlot widget to the GridSlot array 
			GridSlots.Add(GridSlot); 
		}
	}
}

bool UInv_InventoryGrid::MatchesCategory(const UInv_InventoryItem* Item) const
{
	// Checks item manfiest to item category 
	return Item->GetItemManifest().GetItemCategory() == ItemCategory; 
}
