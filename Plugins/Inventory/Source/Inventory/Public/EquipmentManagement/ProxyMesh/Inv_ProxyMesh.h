// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Inv_ProxyMesh.generated.h"

class UInv_EquipmentComponent;

UCLASS()
class INVENTORY_API AInv_ProxyMesh : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInv_ProxyMesh();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	
	// This is the mesh on the player-controlled character 
	TWeakObjectPtr<USkeletalMeshComponent> SourceMesh; 
	
	UPROPERTY(visibleAnywhere)
	TObjectPtr<UInv_EquipmentComponent> EquipmentComponent;
	
	// This is the proxy mesh we will see in the Inventory Menu 
	UPROPERTY(visibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> Mesh; 
};
