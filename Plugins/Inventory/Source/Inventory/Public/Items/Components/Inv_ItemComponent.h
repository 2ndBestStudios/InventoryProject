// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Inv_ItemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class INVENTORY_API UInv_ItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInv_ItemComponent();

	// Getter for pickup message 
	FString GetPickupMessage() const {return PickupMessage;}

	// Needed for replication 
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// Returns ItemManifest 
	FInv_ItemManifest GetItemManifest() const {return ItemManifest;}
private:

	// Has reference to item manifest 
	UPROPERTY(EditAnywhere, Replicated, Category="Inventory")
	FInv_ItemManifest ItemManifest; 
	
	// Create pickup message 
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FString PickupMessage;
};
