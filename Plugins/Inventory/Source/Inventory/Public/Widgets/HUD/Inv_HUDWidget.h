// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inv_HUDWidget.generated.h"

class UInv_InfoMessage;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_HUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Created when game starts 
	void NativeOnInitialized() override;

	// Create implementable events to use in BP 
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void ShowPickupMessage(const FString& Message);

	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void HidePickupMessage();

private:
	// Binds info message widget 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInv_InfoMessage> InfoMessage;

	// Displays OnNoRoom message 
	UFUNCTION()
	void OnNoRoom(); 
};
