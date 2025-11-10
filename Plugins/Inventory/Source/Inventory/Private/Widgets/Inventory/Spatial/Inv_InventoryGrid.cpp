// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Widgets/Inventory/GridSlots/Inv_GridSlot.h"
#include "Widgets/Utils/Inv_WidgetUtils.h"

void UInv_InventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ConstructGrid();

	// Fills out inventory component reference and subscribes to callback 
	InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	InventoryComponent->OnItemAdded.AddDynamic(this, &ThisClass::AddItem); 
}

void UInv_InventoryGrid::AddItem(UInv_InventoryItem* Item)
{
	// Checks if categories match 
	if (!MatchesCategory(Item)) return;

	UE_LOG(LogTemp, Warning, TEXT("InventoryGrid::AddItem")); 
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
