// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Inv_EquipActor.generated.h"

UCLASS()
class INVENTORY_API AInv_EquipActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInv_EquipActor();
	
	FGameplayTag GetEquipmentTag() const {return EquipmentType;}
	void SetEquipmentType (FGameplayTag Type) { EquipmentType = Type; }
	
private:
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag EquipmentType;
};
