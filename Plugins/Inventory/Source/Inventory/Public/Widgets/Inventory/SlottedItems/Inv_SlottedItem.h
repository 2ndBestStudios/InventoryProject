// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inv_SlottedItem.generated.h"

class UInv_InventoryItem;
class UImage;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_SlottedItem : public UUserWidget
{
	GENERATED_BODY()

public:

	// Getter and setter functions for variables 
	bool IsStackable() const { return bIsStackable; }
	void SetIsStackable(bool bStackable) { bIsStackable = bStackable; }
	UImage* GetImageIcon() const { return Image_Icon; }
	void SetGridIndex(int32 Index) { GridIndex = Index; }
	int32 GetGridIndex() const { return GridIndex; }
	void SetGridDimensions (const FIntPoint& Dimensions) { GridDimensions = Dimensions; }
	FIntPoint GetGridDimensions() const { return GridDimensions; }
	void SetInventoryItem(UInv_InventoryItem* Item); 
	UInv_InventoryItem* GetInventoryItem() const { return InventoryItem.Get(); }
	// Sets ImageIcon's brush based on passed in value  
	void SetImageBrush (const FSlateBrush& Brush) const; 
	
private:

	// Image of item
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;
	// Properties of item
	int32 GridIndex;
	FIntPoint GridDimensions;
	bool bIsStackable {false};
	TWeakObjectPtr<UInv_InventoryItem> InventoryItem;
};
