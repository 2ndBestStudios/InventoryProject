// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inv_EquipmentComponent.generated.h"


struct FGameplayTag;
struct FInv_ItemManifest;
struct FInv_EquipmentFragment;
class AInv_EquipActor;
class UInv_InventoryItem;
class UInv_InventoryComponent;
class APlayerController; 
class USkeletalMeshComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class INVENTORY_API UInv_EquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	void SetOwningSkeletalMesh(USkeletalMeshComponent* OwningMesh);
	
	void SetIsProxy(bool bProxy) {bIsProxy = bProxy;};

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
	
	AInv_EquipActor* SpawnEquippedActor(FInv_EquipmentFragment* EquipmentFragment, const FInv_ItemManifest& Manifest, USkeletalMeshComponent* AttachMesh); 
	
	UPROPERTY()
	TArray<TObjectPtr<AInv_EquipActor>> EquippedActors;
	
	AInv_EquipActor* FindEquippedActorByTag(const FGameplayTag& EquipmentTypeTag); 
	void RemoveEquippedActorByTag(const FGameplayTag& EquipmentTypeTag);
	
	void InitPlayerController(); 
	
	UFUNCTION()
	void OnPossessedPawnChange(APawn* OldPawn, APawn* NewPawn);
	
	bool bIsProxy{false};
};
