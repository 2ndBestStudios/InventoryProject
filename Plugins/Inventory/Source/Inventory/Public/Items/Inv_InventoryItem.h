// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Inv_InventoryItem.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_InventoryItem : public UObject
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	// Needed for replicating subobjects 
	virtual bool IsSupportedForNetworking() const override { return true; }

	// Sets the item manifest 
	void SetItemManifest(const FInv_ItemManifest Manifest);

	// Getter for item manifest. One returns a reference that cannot be changed. The other returns one that can be changed
	const FInv_ItemManifest& GetItemManifest() const { return ItemManifest.Get<FInv_ItemManifest>(); }
	FInv_ItemManifest& GetItemManifestMutable() { return ItemManifest.GetMutable<FInv_ItemManifest>(); }

private:

	// Sets available item properties 
	UPROPERTY(VisibleAnywhere, meta=(BaseStruct = "/Script/Inventory.Inv_ItemManifest"), Replicated)
	FInstancedStruct ItemManifest; 
};
