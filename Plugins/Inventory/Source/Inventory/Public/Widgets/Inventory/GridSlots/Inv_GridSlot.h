// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VisualizeTexture.h"
#include "Blueprint/UserWidget.h"
#include "Inv_GridSlot.generated.h"

class UInv_InventoryItem;
class UImage;

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
private:
	// Grid slot remembers its own index 
	int32 TileIndex;

	int32 StackCount;
	int32 UpperLeftIndex{INDEX_NONE};
	TWeakObjectPtr<UInv_InventoryItem> InventoryItem;
	bool bAvailable; 

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
