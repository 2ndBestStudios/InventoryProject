// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inv_SlottedItem.generated.h"

class UInv_InventoryItem;
class UImage;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSlottedItemClicked, int32, GridIndex, const FPointerEvent&, MouseEvent);

/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_SlottedItem : public UUserWidget
{
	GENERATED_BODY()

public:
	// Override function for capturing MouseButton clicks 
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	
	// Delegate 
	FSlottedItemClicked OnSlottedItemClicked; 
	
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

	// Updates the StackCount 
	void UpdateStackCount(int32 StackCount); 
	
private:

	// Image of Item
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;

	// Number indicating ItemStackCount
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_StackCount;
	
	// Properties of Item
	int32 GridIndex;
	FIntPoint GridDimensions;
	bool bIsStackable {false};
	TWeakObjectPtr<UInv_InventoryItem> InventoryItem;
};
