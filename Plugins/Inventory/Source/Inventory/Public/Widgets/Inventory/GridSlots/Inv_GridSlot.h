// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inv_GridSlot.generated.h"

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
	// Set and get tile index 
	void SetTileIndex(int32 Index) {TileIndex = Index;}
	int32 GetTileIndex() const {return TileIndex;}
	// Gets the GridSlotState
	EInv_GridSlotState GetGridSlotState() const {return GridSlotState;}
	// Sets the GridSlot status 
	void SetOccupiedTexture();
	void SetUnoccupiedTexture();
	void SetSelectedTexture();
	void SetGrayedOutTexture();
private:
	// Grid slot remembers its own index 
	int32 TileIndex; 

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
