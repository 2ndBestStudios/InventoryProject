// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "Widgets/Inventory/InventoryBase/Inv_InventoryBase.h"

// Component constructor 
UInv_InventoryComponent::UInv_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

// Called in beginning of game 
void UInv_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Create inventory 
	ConstructInventory(); 
}

// Creates inventory 
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
}



