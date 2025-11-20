// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryManagement/FastArray/Inv_FastArray.h"
#include "Inv_InventoryComponent.generated.h"

class UInv_ItemComponent;
class UInv_InventoryItem;
class UInv_InventoryBase;

// Delegates
// Called when item is changed 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryItemChange, UInv_InventoryItem*, Item);
// Called if no room in inventory 
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNoRoomInInventory);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStackChange, const FInv_SlotAvailabilityResult&, Result);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class INVENTORY_API UInv_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Constructor 
	UInv_InventoryComponent();

	// Handles Replication 
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	// Trys to add item to inventory 
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Inventory") 
	void TryAddItem(UInv_ItemComponent* ItemComponent);

	// Server functions for adding new items and stacks 
	UFUNCTION(Server, Reliable)
	void Server_AddNewItem(UInv_ItemComponent* ItemComponent, int32 StackCount);

	UFUNCTION(Server, Reliable)
	void Server_AddStacksToItem(UInv_ItemComponent* ItemComponent, int32 StackCount, int32 Remainder);

	UFUNCTION(Server, Reliable)
	void Server_DropItem(UInv_InventoryItem* Item, int32 StackCount);

	void SpawnDroppedItem(UInv_InventoryItem* Item, int32 StackCount);
	
	// Toggle inventory 
	void ToggleInventoryMenu();

	// Required for replication 
	void AddRepSubObj(UObject* SubObj);

	FInventoryItemChange OnItemAdded;
	FInventoryItemChange OnItemRemoved;
	FNoRoomInInventory NoRoomInventory;
	FStackChange OnStackChange;
protected:
	// Called on game start 
	virtual void BeginPlay() override;

private:
	// Get player reference 
	TWeakObjectPtr<APlayerController> OwningController; 
	
	// Creates inventory 
	void ConstructInventory();

	// Includes Fast Array to access inventory 
	UPROPERTY(Replicated)
	FInv_InventoryFastArray InventoryList; 

	// Inventory class 
	UPROPERTY()
	TObjectPtr<UInv_InventoryBase> InventoryMenu;

	UPROPERTY(EditAnywhere, Category= "Inventory")
	TSubclassOf<UInv_InventoryBase> InventoryMenuClass;

	// Helper variables & functions 
	bool bInventoryMenuOpen;
	void OpenInventoryMenu();
	void CloseInventoryMenu();

	UPROPERTY(EditAnywhere, Category= "Inventory")
	float DropSpawnAngleMin = -85.f;

	UPROPERTY(EditAnywhere, Category= "Inventory")
	float DropSpawnAngleMax = 85.f;

	UPROPERTY(EditAnywhere, Category= "Inventory")
	float DropSpawnDistanceMin = 10.f;

	UPROPERTY(EditAnywhere, Category= "Inventory")
	float DropSpawnDistanceMax = 50.f;

	UPROPERTY(EditAnywhere, Category= "Inventory")
	float RelativeSpawnElevation = -70.f;
};
