// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Inv_InventoryItem.h"

#include "Net/UnrealNetwork.h"

void UInv_InventoryItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Needed for replication 
	DOREPLIFETIME(ThisClass, ItemManifest); 
}

void UInv_InventoryItem::SetItemManifest(const FInv_ItemManifest Manifest)
{
	// Creates instanced struct of ItemManifest 
	ItemManifest = FInstancedStruct::Make<FInv_ItemManifest>(Manifest); 
}
