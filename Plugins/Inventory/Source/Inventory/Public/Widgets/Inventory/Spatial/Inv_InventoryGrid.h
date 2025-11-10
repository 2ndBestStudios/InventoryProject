// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/Inv_GridTypes.h"
#include "Inv_InventoryGrid.generated.h"

class UInv_InventoryComponent;
class UCanvasPanel;
class UInv_GridSlot;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_InventoryGrid : public UUserWidget
{
	GENERATED_BODY()

public:
	// Setups inital values 
	virtual void NativeOnInitialized() override; 
	
	// Getter for item category 
	EInv_ItemCategory GetItemCategory() const { return ItemCategory; }

	// Callback function for the additem delegate from inventory component 
	UFUNCTION()
	void AddItem(UInv_InventoryItem* Item);  
	
private:

	// Gives a reference for the inventory component 
	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent;
	
	// Constructs item grid 
	void ConstructGrid(); 
	
	// Creates item category 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category="Inventory")
	EInv_ItemCategory ItemCategory;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_GridSlot> GridSlotClass;

	// Allows us to place the created widgets onto a canvas 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;
	
	UPROPERTY()
	TArray<TObjectPtr<UInv_GridSlot>> GridSlots;

	// Sets inventory grid size 
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 Rows;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 Columns;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float TileSize;

	// Handles matching item category to inventory item 
	bool MatchesCategory(const UInv_InventoryItem* Item) const; 
};
