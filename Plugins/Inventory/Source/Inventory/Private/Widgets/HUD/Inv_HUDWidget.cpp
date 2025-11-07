// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/Inv_HUDWidget.h"

#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Widgets/HUD/Inv_InfoMessage.h"

void UInv_HUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Subscribes to inventory component delegate 
	UInv_InventoryComponent* InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	if (IsValid(InventoryComponent))
	{
		// Binds NoRoomInventory delegate to function OnNoRoom 
		InventoryComponent->NoRoomInventory.AddDynamic(this, &ThisClass::OnNoRoom); 
	}
}

void UInv_HUDWidget::OnNoRoom()
{
	// Checks if InfoMessage is valid and sets message 
	if (!IsValid(InfoMessage)) return;
	InfoMessage->SetMessage(FText::FromString("No Room In Inventory")); 
}
