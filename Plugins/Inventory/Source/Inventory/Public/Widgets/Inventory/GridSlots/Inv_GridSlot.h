// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VisualizeTexture.h"
#include "Blueprint/UserWidget.h"
#include "Inv_GridSlot.generated.h"

class UInv_InventoryItem;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGridSlotEvent, int32, GridIndex, const FPointerEvent&, MouseEvent);

UENUM(BlueprintType)
enum class EInv_GridSlotState : uint8
{
	Unoccupied,
	Occupied,
	Selected,
	GrayedOut 
};

UCLASS()
class INVENTORY_API UInv_GridSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	// Override function that captures when the mouse enters a widget 
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	// Override function that captures when the mouse leaves a widget 
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	// Override function that captures when the mouse is clicked on a widget 
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	// Gets the GridSlotState
	EInv_GridSlotState GetGridSlotState() const {return GridSlotState;}
	
	// Setters and Getters for below GridSlot variables 
	void SetTileIndex(int32 Index) {TileIndex = Index;}
	int32 GetTileIndex() const {return TileIndex;}
	
	TWeakObjectPtr<UInv_InventoryItem> GetInventoryItem() const {return InventoryItem;}
	void SetInventoryItem(UInv_InventoryItem* Item);

	int32 GetStackCount() const {return StackCount;}
	void SetStackCount(int32 Count) {StackCount = Count;}
	
	int32 GetUpperLeftIndex() const {return UpperLeftIndex;}
	void SetUpperLeftIndex(int32 Index) {UpperLeftIndex = Index;}

	bool IsAvailable() const {return bAvailable;}
	void SetAvailable(bool bIsAvailable) {bAvailable = bIsAvailable;}
	
	// Sets the GridSlot status 
	void SetOccupiedTexture();
	void SetUnoccupiedTexture();
	void SetSelectedTexture();
	void SetGrayedOutTexture();

	FGridSlotEvent GridSlotClicked;
	FGridSlotEvent GridSlotHovered;
	FGridSlotEvent GridSlotUnhovered;
	
private:
	// Grid slot remembers its own index 
	int32 TileIndex{INDEX_NONE};
	
	int32 StackCount{0};
	int32 UpperLeftIndex{INDEX_NONE};
	TWeakObjectPtr<UInv_InventoryItem> InventoryItem;
	bool bAvailable{true}; 

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_GridSlot;

	UPROPERTY(EditAnywhere, Category="Inventory")
	FSlateBrush Brush_Unoccupied;

	UPROPERTY(EditAnywhere, Category="Inventory")
	FSlateBrush Brush_Occupied;

	UPROPERTY(EditAnywhere, Category="Inventory")
	FSlateBrush Brush_Selected;

	UPROPERTY(EditAnywhere, Category="Inventory")
	FSlateBrush Brush_GrayedOut;

	EInv_GridSlotState GridSlotState;


};
