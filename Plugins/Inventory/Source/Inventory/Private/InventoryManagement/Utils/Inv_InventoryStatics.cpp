// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagement/Utils/Inv_InventoryStatics.h"

#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Types/Inv_GridTypes.h"

UInv_InventoryComponent* UInv_InventoryStatics::GetInventoryComponent(const APlayerController* PlayerController)
{
	// Checks for valid PlayerController 
	if (!IsValid(PlayerController)) return nullptr;

	// Returns InventoryComponent by finding it through PlayerController 
	UInv_InventoryComponent* InventoryComponent = PlayerController->FindComponentByClass<UInv_InventoryComponent>();
	return InventoryComponent;
}

EInv_ItemCategory UInv_InventoryStatics::GetItemCategoryFromItemComp(UInv_ItemComponent* ItemComp)
{
	// Checks if ItemComponent is valid and then returns 
	if (!IsValid(ItemComp)) return EInv_ItemCategory::None;
	return ItemComp->GetItemManifest().GetItemCategory(); 
}
