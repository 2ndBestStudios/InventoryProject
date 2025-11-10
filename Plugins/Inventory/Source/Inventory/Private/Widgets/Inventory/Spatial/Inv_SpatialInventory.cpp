// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_SpatialInventory.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"

void UInv_SpatialInventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Binds the BP Widget callback delegate to the C++ function 
	Button_Equippables->OnClicked.AddDynamic(this, &ThisClass::UInv_SpatialInventory::ShowEquippables);
	Button_Consumables->OnClicked.AddDynamic(this, &ThisClass::UInv_SpatialInventory::ShowConsumables);
	Button_Craftables->OnClicked.AddDynamic(this, &ThisClass::UInv_SpatialInventory::ShowCraftables);

	ShowEquippables(); 
}

FInv_SlotAvailabilityResult UInv_SpatialInventory::HasRoomForItem(UInv_ItemComponent* ItemComponent) const
{
	// Temp functionality for testing 
	FInv_SlotAvailabilityResult Result;
	Result.TotalRoomToFill = 1; 
	return Result; 
}

void UInv_SpatialInventory::ShowEquippables()
{
	SetActiveGrid(Grid_Equippables, Button_Equippables);
}

void UInv_SpatialInventory::ShowConsumables()
{
	SetActiveGrid(Grid_Consumables, Button_Consumables);
}

void UInv_SpatialInventory::ShowCraftables()
{
	SetActiveGrid(Grid_Craftables, Button_Craftables);
}

void UInv_SpatialInventory::DisableButton(UButton* Button)
{
	// Enables all buttons to be used 
	Button_Equippables->SetIsEnabled(true);
	Button_Consumables->SetIsEnabled(true);
	Button_Craftables->SetIsEnabled(true);
	// Disables button just clicked 
	Button->SetIsEnabled(false);
}

void UInv_SpatialInventory::SetActiveGrid(UInv_InventoryGrid* InventoryGrid, UButton* Button)
{
	// Disables button just clicked on from delegate. Enables other buttons 
	DisableButton(Button);

	// Switches active widget to given inventory type 
	Switcher->SetActiveWidget(InventoryGrid); 
}
