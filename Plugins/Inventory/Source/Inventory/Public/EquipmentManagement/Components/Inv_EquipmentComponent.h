// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inv_EquipmentComponent.generated.h"

class UInv_InventoryItem;
class UInv_InventoryComponent;
class APlayerController; 
class USkeletalMeshComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class INVENTORY_API UInv_EquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent;
	TWeakObjectPtr<APlayerController> OwningPlayerController;
	TWeakObjectPtr<USkeletalMeshComponent> OwningSkeletalMeshComponent;
	
	UFUNCTION()
	void OnItemEquipped(UInv_InventoryItem* EquippedItem); 
	
	UFUNCTION()
	void OnItemUnequipped(UInv_InventoryItem* UnequippedItem); 
	
	void InitInventoryComponent();
};
