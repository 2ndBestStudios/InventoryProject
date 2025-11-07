// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "Widgets/Inventory/InventoryBase/Inv_InventoryBase.h"

UInv_InventoryComponent::UInv_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UInv_InventoryComponent::ToggleInventoryMenu()
{
	// Toggles inventory menu 
	if (bInventoryMenuOpen)
	{
		CloseInventoryMenu();
	}
	else
	{
		OpenInventoryMenu();
	}
}

void UInv_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Create inventory 
	ConstructInventory(); 
}

void UInv_InventoryComponent::TryAddItem(UInv_ItemComponent* ItemComponent)
{
	// Broadcasts delegate when function is called 
	NoRoomInventory.Broadcast(); 
}

void UInv_InventoryComponent::ConstructInventory()
{
	// Get Player controller by casting the result of the actor function GetOwner
	// Check if it's valid and make sure that if it's not valid there is an early return 
	OwningController = Cast<APlayerController>(GetOwner());
	checkf(OwningController.IsValid(), TEXT("Inventory component should have a Player Controller as Owner."));
	if (!OwningController.IsValid()) return;

	// Create widget based on InventoryBaseWidget. Since OwningController is weak get actual. Based on InventoryClass 
	InventoryMenu = CreateWidget<UInv_InventoryBase>(OwningController.Get(), InventoryMenuClass);
	InventoryMenu->AddToViewport();

	// After creating set inventory to collapsed 
	CloseInventoryMenu(); 
}

void UInv_InventoryComponent::OpenInventoryMenu()
{
	// Check if inventory menu is valid, then set visibility and toggle variable 
	if (!IsValid(InventoryMenu)) return;

	InventoryMenu->SetVisibility(ESlateVisibility::Visible);
	bInventoryMenuOpen = true;

	// Check if owning controller is valid, then set input mode to UI 
	if (!OwningController.IsValid()) return;

	FInputModeGameAndUI InputMode;
	OwningController->SetInputMode(InputMode);
	OwningController->SetShowMouseCursor(true);
	
}

void UInv_InventoryComponent::CloseInventoryMenu()
{
	// Check if inventory menu is valid, then collapse and toggle variable 
	if (!IsValid(InventoryMenu)) return;

	InventoryMenu->SetVisibility(ESlateVisibility::Collapsed);
	bInventoryMenuOpen = false;

	// Check if owning controller is valid, then reenable input to game  
	if (!OwningController.IsValid()) return;

	FInputModeGameOnly InputMode;
	OwningController->SetInputMode(InputMode);
	OwningController->SetShowMouseCursor(false);
}



