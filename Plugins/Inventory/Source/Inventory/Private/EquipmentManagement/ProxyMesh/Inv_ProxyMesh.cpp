// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentManagement/ProxyMesh/Inv_ProxyMesh.h"

#include "EquipmentManagement/Components/Inv_EquipmentComponent.h"


// Sets default values
AInv_ProxyMesh::AInv_ProxyMesh()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
	Mesh->SetupAttachment(RootComponent);
	
	EquipmentComponent = CreateDefaultSubobject<UInv_EquipmentComponent>("Equipment");
	EquipmentComponent->SetOwningSkeletalMesh(Mesh);
	EquipmentComponent->SetIsProxy(true);
}

// Called when the game starts or when spawned
void AInv_ProxyMesh::BeginPlay()
{
	Super::BeginPlay();
	
}


