// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Inv_PlayerController.generated.h"

class UInv_InventoryComponent;
class UInv_HUDWidget;
class UInputMappingContext;
class UInputAction;

UCLASS()
class INVENTORY_API AInv_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Constructor 
	AInv_PlayerController();

	// Tick 
	virtual void Tick( float DeltaTime );

	// Toggles inventory. Can be called from blueprint 
	UFUNCTION(BlueprintCallable)
	void ToggleInventory();
	
protected:
	// Called in beginning of game 
	virtual void BeginPlay() override;

	// Setups any input 
	virtual void SetupInputComponent() override;

private:
	// Functionality for interact 
	void PrimaryInteract();

	// Creates HUD 
	void CreateHUDWidget();

	// Line Trace for items 
	void TraceForItem();

	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent;
	
	UPROPERTY(EditDefaultsOnly, Category= "Inventory")
	TArray<TObjectPtr<UInputMappingContext>> DefaultInputMappingContexts;

	UPROPERTY(editdefaultsOnly, Category= "Inventory")
	TObjectPtr<UInputAction> PrimaryInteractAction;

	UPROPERTY(editdefaultsOnly, Category= "Inventory")
	TObjectPtr<UInputAction> ToggleInventoryAction;

	UPROPERTY(EditDefaultsOnly, Category= "Inventory")
	TSubclassOf<UInv_HUDWidget> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UInv_HUDWidget> HUDWidget;

	UPROPERTY(EditDefaultsOnly, Category= "Inventory")
	double TraceLength;

	UPROPERTY(EditDefaultsOnly, Category= "Inventory")
	TEnumAsByte<ECollisionChannel> ItemTraceChannel;

	TWeakObjectPtr<AActor> ThisActor;
	TWeakObjectPtr<AActor> LastActor;
};
