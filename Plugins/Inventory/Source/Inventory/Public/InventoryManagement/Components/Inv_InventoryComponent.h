// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inv_InventoryComponent.generated.h"

class UInv_InventoryBase; 

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class INVENTORY_API UInv_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Constructor 
	UInv_InventoryComponent();

	// Toggle inventory 
	void ToggleInventoryMenu(); 

protected:
	// Called on game start 
	virtual void BeginPlay() override;

private:
	// Get player reference 
	TWeakObjectPtr<APlayerController> OwningController; 

	// Creates inventory 
	void ConstructInventory();

	// Inventory class 
	UPROPERTY()
	TObjectPtr<UInv_InventoryBase> InventoryMenu;

	UPROPERTY(EditAnywhere, Category= "Inventory")
	TSubclassOf<UInv_InventoryBase> InventoryMenuClass;

	// Helper variables & functions 
	bool bInventoryMenuOpen;
	void OpenInventoryMenu();
	void CloseInventoryMenu();
};
