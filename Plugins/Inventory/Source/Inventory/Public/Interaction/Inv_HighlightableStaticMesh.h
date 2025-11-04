// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inv_Highlightable.h"
#include "Components/StaticMeshComponent.h"
#include "Inv_HighlightableStaticMesh.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_HighlightableStaticMesh : public UStaticMeshComponent, public IInv_Highlightable
{
	GENERATED_BODY()

public:
	// Rewatch to understand implmentation 
	virtual void Highlight_Implementation() override;
	virtual void UnHighlight_Implementation() override;

private:

	// Create highlight material for static mesh
	UPROPERTY(EditAnywhere, Category="Inventory")
	TObjectPtr<UMaterialInterface> HighlightMaterial;
};
