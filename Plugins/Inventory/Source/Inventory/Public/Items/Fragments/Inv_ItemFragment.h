#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "Inv_ItemFragment.generated.h"

USTRUCT(BlueprintType)
struct FInv_ItemFragment
{
	GENERATED_BODY()

	// Come back and learn more about construct, copy, move, destruct 
	FInv_ItemFragment() {} 
	FInv_ItemFragment(const FInv_ItemFragment&) = default;
	FInv_ItemFragment& operator=(const FInv_ItemFragment&) = default;
	FInv_ItemFragment(FInv_ItemFragment&&) = default;
	FInv_ItemFragment& operator=(FInv_ItemFragment&&) = default;
	virtual ~FInv_ItemFragment() {}

	// Getter and setter for gameplay tags
	FGameplayTag GetFragmentTag() const { return FragmentTag; }
	void SetFragmentTag(FGameplayTag Tag) { FragmentTag = Tag; }
	
private:

	// Unique id for fragments 
	UPROPERTY(EditAnywhere, Category="Inventory")
	FGameplayTag FragmentTag = FGameplayTag::EmptyTag;
	
};

USTRUCT(BlueprintType)
struct FInv_GridFragment : public FInv_ItemFragment
{
	GENERATED_BODY()

	// Getter and setters for variables 
	FIntPoint GetGridSize() const { return GridSize; }
	void SetGridSize(const FIntPoint& Size) { GridSize = Size; }
	float GetGridPadding () const { return GridPadding; }
	void SetGridPadding(float Padding) { GridPadding = Padding; }
	
private:

	// Dictates how many spaces the item takes up 
	UPROPERTY(EditAnywhere, Category="Inventory")
	FIntPoint GridSize{1,1};

	// Allows any padding in the grid 
	UPROPERTY(EditAnywhere, Category="Inventory")
	float GridPadding{0.f};

	
};