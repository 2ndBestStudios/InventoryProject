// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inv_ItemPopUp.generated.h"

class USizeBox;
class UButton;
class USlider;
class UTextBlock;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FPopUpMenuSplit, int32, SplitAmount, int32, Index);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPopUpMenuDrop, int32, Index);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPopUpMenuConsume, int32, Index);

/**
 * The ItemPopUp widget shows up when right-clicking on an item in the inventory grid 
 */
UCLASS()
class INVENTORY_API UInv_ItemPopUp : public UUserWidget
{
	GENERATED_BODY()

public:
	// Override Functions 
	virtual void NativeOnInitialized() override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	// Delegates 
	FPopUpMenuSplit OnSplit;
	FPopUpMenuDrop OnDrop;
	FPopUpMenuConsume OnConsume;

	// Slider values 
	int32 GetSplitAmount() const;
	void SetSliderParameters(const float Max, const float Value) const;

	// Collapse unused properties of widget depending on item
	void CollapseSplitButton() const;
	void CollapseConsumeButton() const;

	// Box size of widget
	FVector2D GetBoxSize() const;

	// GridIndex of item in reference 
	void SetGridIndex(int32 Index) { GridIndex = Index; }
	int32 GetGridIndex() const { return GridIndex; }

private:

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Split;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Drop;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Consume;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USlider> Slider_Split;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_SplitAmount;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;

	int32 GridIndex{INDEX_NONE}; 

	// Functions bound to delegate 
	UFUNCTION()
	void SplitButtonClicked();

	UFUNCTION()
	void DropButtonClicked();

	UFUNCTION()
	void ConsumeButtonClicked();

	UFUNCTION()
	void SliderValueChanged(float Value);
	
	
};
