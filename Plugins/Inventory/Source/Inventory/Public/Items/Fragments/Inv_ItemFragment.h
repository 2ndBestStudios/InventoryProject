#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Widgets/Composite/Inv_CompositeBase.h"
#include "Windows/WindowsApplication.h"

#include "Inv_ItemFragment.generated.h"

class UInv_CompositeBase;
class APlayerController;

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
	virtual void Manifest() {}
	
private:

	// Unique id for fragments 
	UPROPERTY(EditAnywhere, Category="Inventory", meta = (Categories = "FragmentTags"))
	FGameplayTag FragmentTag = FGameplayTag::EmptyTag;
	
};

// Item fragment specifically for assimilation into a widget 
USTRUCT(BlueprintType)
struct FInv_InventoryItemFragment : public FInv_ItemFragment
{
	GENERATED_BODY()

	virtual void Assimilate(UInv_CompositeBase* CompositeBase) const;

protected:
	bool MatchesWidgetTag(const UInv_CompositeBase* CompositeBase) const; 
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

USTRUCT(BlueprintType)
struct FInv_ImageFragment : public FInv_InventoryItemFragment
{
	GENERATED_BODY()

	// Getter 
	UTexture2D* GetIcon() const { return Icon; }

	virtual void Assimilate(UInv_CompositeBase* CompositeBase) const override;
	
private:

	// Provides icon for item
	UPROPERTY(EditAnywhere, Category="Inventory")
	TObjectPtr<UTexture2D> Icon{nullptr};
	
	// Can be adjusted for inspecting the item with widget 
	UPROPERTY(EditAnywhere, Category="Inventory")
	FVector2D IconDimensions{44.f, 44.f};
};

USTRUCT(BlueprintType)
struct FInv_TextFragment : public FInv_InventoryItemFragment
{
	GENERATED_BODY()

	FText GetText() const { return FragmentText; }
	void SetText(const FText& Text) { FragmentText = Text; }
	virtual void Assimilate(UInv_CompositeBase* CompositeBase) const override;

	
private:

	UPROPERTY(EditAnywhere, Category="Inventory")
	FText FragmentText; 
};

USTRUCT(BlueprintType)
struct FInv_LabeledNumberFragment : public FInv_InventoryItemFragment
{
	GENERATED_BODY()

	virtual void Assimilate(UInv_CompositeBase* CompositeBase) const override;
	virtual void Manifest() override;
	float GetValue() const { return Value; }

	// When manifesting for the first time this fragment will randomize 
	bool bRandomizeOnManifest{true};
	
private:

	UPROPERTY(EditAnywhere, Category="Inventory")
	FText Text_Label;

	UPROPERTY(VisibleAnywhere, Category="Inventory")
	float Value{0.f};

	UPROPERTY(EditAnywhere, Category="Inventory")
	float Min{0.f};

	UPROPERTY(EditAnywhere, Category="Inventory")
	float Max{0.f};

	UPROPERTY(editAnywhere, Category="Inventory")
	bool bCollapseLabel{false};

	UPROPERTY(EditAnywhere, Category="Inventory")
	bool bCollapseValue{false};

	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 MinFractionalDigits{1};

	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 MaxFractionalDigits{1};
};

USTRUCT(BlueprintType)
struct FInv_StackableFragment : public FInv_ItemFragment
{
	GENERATED_BODY()
	// Getter & Setter functions 
	int32 GetMaxStackSize() const { return MaxStackSize; }
	int32 GetStackCount() const { return StackCount; }
	void SetStackCount(int32 Count) { StackCount = Count; }

private:
	// MaxStackSize for item
	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 MaxStackSize{1};
	
	// How many stacks does this item have 
	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 StackCount{1}; 
};


// Consume Fragments

USTRUCT(BlueprintType)
struct FInv_ConsumeModifier : public FInv_LabeledNumberFragment
{
	GENERATED_BODY()

	virtual void OnConsume(APlayerController* PC) {}
};

USTRUCT(BlueprintType)
struct FInv_ConsumableFragment : public FInv_InventoryItemFragment
{
	GENERATED_BODY()

	virtual void OnConsume(APlayerController* PC);
	virtual void Assimilate(UInv_CompositeBase* CompositeBase) const override;
	virtual void Manifest() override;

private:

	UPROPERTY(EditAnywhere, Category="Inventory", meta= (ExcludeBaseStruct))
	TArray<TInstancedStruct<FInv_ConsumeModifier>> ConsumeModifiers;
};

USTRUCT(BlueprintType)
struct FInv_HealthPotionFragment : public FInv_ConsumeModifier
{
	GENERATED_BODY()
	
	virtual void OnConsume(APlayerController* PC) override;
};

USTRUCT(BlueprintType)
struct FInv_ManaPotionFragment : public FInv_ConsumeModifier
{
	GENERATED_BODY()
	
	virtual void OnConsume(APlayerController* PC) override;
};


// Equipment 

USTRUCT(BlueprintType)
struct FInv_EquipModifier : public FInv_LabeledNumberFragment
{
	GENERATED_BODY()
	
	virtual void OnEquip(APlayerController* PC) {}
	virtual void OnUnequip(APlayerController* PC) {}
	
};

USTRUCT(BlueprintType)
struct FInv_StrengthModifier : public FInv_EquipModifier
{
	GENERATED_BODY()
	
	virtual void OnEquip(APlayerController* PC) override;
	virtual void OnUnequip(APlayerController* PC) override;
	
};

USTRUCT(BlueprintType)
struct FInv_EquipmentFragment : public FInv_InventoryItemFragment
{
	GENERATED_BODY()
	
	bool bEquipped{false};
	void OnEquip(APlayerController* PC);
	void OnUnequip(APlayerController* PC);
	virtual void Assimilate(UInv_CompositeBase* CompositeBase) const override;
	virtual void Manifest() override;
	
private:
	
	UPROPERTY(EditAnywhere, Category="Inventory", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FInv_EquipModifier>> EquipModifiers;
	
};
