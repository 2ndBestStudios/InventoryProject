#include "Items/Fragments/Inv_ItemFragment.h"

#include "EquipmentManagement/EquipActor/Inv_EquipActor.h"
#include "Widgets/Composite/Inv_CompositeBase.h"
#include "Widgets/Composite/Inv_Leaf_Image.h"
#include "Widgets/Composite/Inv_Leaf_LabeledValue.h"
#include "Widgets/Composite/Inv_Leaf_Text.h"

void FInv_InventoryItemFragment::Assimilate(UInv_CompositeBase* CompositeBase) const
{
	if (!MatchesWidgetTag(CompositeBase)) return; 
	CompositeBase->Expand(); 
}

bool FInv_InventoryItemFragment::MatchesWidgetTag(const UInv_CompositeBase* CompositeBase) const
{
	return CompositeBase->GetFragmentTag().MatchesTagExact(GetFragmentTag());
}

void FInv_ImageFragment::Assimilate(UInv_CompositeBase* CompositeBase) const
{
	FInv_InventoryItemFragment::Assimilate(CompositeBase);
	if (!MatchesWidgetTag(CompositeBase)) return;

	UInv_Leaf_Image* Image = Cast<UInv_Leaf_Image>(CompositeBase);
	if (!IsValid(Image)) return;

	Image->SetImage(Icon);
	Image->SetBoxSize(IconDimensions);
	Image->SetImageSize(IconDimensions);
}

void FInv_TextFragment::Assimilate(UInv_CompositeBase* CompositeBase) const
{
	FInv_InventoryItemFragment::Assimilate(CompositeBase);
	if (!MatchesWidgetTag(CompositeBase)) return;

	UInv_Leaf_Text* Text = Cast<UInv_Leaf_Text>(CompositeBase);
	if (!IsValid(Text)) return;

	Text->SetText(FragmentText); 
}

void FInv_LabeledNumberFragment::Assimilate(UInv_CompositeBase* CompositeBase) const
{
	FInv_InventoryItemFragment::Assimilate(CompositeBase);

	if (!MatchesWidgetTag(CompositeBase)) return;

	UInv_Leaf_LabeledValue* LabeledValue = Cast<UInv_Leaf_LabeledValue>(CompositeBase);
	if (!IsValid(LabeledValue)) return;

	LabeledValue->SetText_Label(Text_Label, bCollapseLabel);

	FNumberFormattingOptions Options;
	Options.MinimumFractionalDigits = MinFractionalDigits;
	Options.MaximumFractionalDigits = MaxFractionalDigits;
	
	LabeledValue->SetText_Value(FText::AsNumber(Value, &Options), bCollapseValue); 
}

void FInv_LabeledNumberFragment::Manifest()
{
	FInv_InventoryItemFragment::Manifest();

	if (bRandomizeOnManifest)
	{
		Value = FMath::FRandRange(Min, Max);
	}
	bRandomizeOnManifest = false;
}

void FInv_ConsumableFragment::OnConsume(APlayerController* PC)
{
	for (auto& Modifier : ConsumeModifiers)
	{
		auto& ModRef = Modifier.GetMutable<>();
		ModRef.OnConsume(PC); 
	}
}

void FInv_ConsumableFragment::Assimilate(UInv_CompositeBase* CompositeBase) const
{
	FInv_InventoryItemFragment::Assimilate(CompositeBase);
	for (const auto& Modifier : ConsumeModifiers)
	{
		const auto& ModRef = Modifier.Get();
		ModRef.Assimilate(CompositeBase);
	}
}

void FInv_ConsumableFragment::Manifest()
{
	FInv_InventoryItemFragment::Manifest();

	for (auto& Modifier : ConsumeModifiers)
	{
		auto& ModRef = Modifier.GetMutable<>();
		ModRef.Manifest(); 
	}
}

void FInv_HealthPotionFragment::OnConsume(APlayerController* PC)
{
	// Get a stats component from the PC or get the AbilitySystemComponent and apply a gameplay effect

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Health potion consumed!")));
}

void FInv_ManaPotionFragment::OnConsume(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Mana potion consumed!")));
}

void FInv_StrengthModifier::OnEquip(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Strength increased!")));
}

void FInv_StrengthModifier::OnUnequip(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Item unequipped. Strength decreased.")));
}

void FInv_EquipmentFragment::OnEquip(APlayerController* PC)
{
	if (bEquipped) return;
	bEquipped = true;
	for (auto& Modifier : EquipModifiers)
	{
		auto& ModRef = Modifier.GetMutable<>();
		ModRef.OnEquip(PC);
	}
}

void FInv_EquipmentFragment::OnUnequip(APlayerController* PC)
{
	if (!bEquipped) return; 
	bEquipped = false;
	for (auto& Modifier : EquipModifiers)
	{
		auto& ModRef = Modifier.GetMutable<>();
		ModRef.OnUnequip(PC);
	}
}

void FInv_EquipmentFragment::Assimilate(UInv_CompositeBase* CompositeBase) const
{
	FInv_InventoryItemFragment::Assimilate(CompositeBase);
	for (const auto& Modifier : EquipModifiers)
	{
		const auto& ModRef = Modifier.Get<>();
		ModRef.Assimilate(CompositeBase);
	}
}

void FInv_EquipmentFragment::Manifest()
{
	FInv_InventoryItemFragment::Manifest();
	for (auto& Modifier : EquipModifiers)
	{
		auto& ModRef = Modifier.GetMutable<>();
		ModRef.Manifest(); 
	}
}

AInv_EquipActor* FInv_EquipmentFragment::SpawnAttachedActor(USkeletalMeshComponent* AttachedMesh) const
{
	if (!IsValid(EquipActorClass) || !IsValid(AttachedMesh)) return nullptr;
	
	AInv_EquipActor* SpawnedActor = AttachedMesh->GetWorld()->SpawnActor<AInv_EquipActor>(EquipActorClass); 
	SpawnedActor->AttachToComponent(AttachedMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketAttachPoint); 
	
	return SpawnedActor;
}

void FInv_EquipmentFragment::DestroyAttachedActor() const
{
	if (EquippedActor.IsValid())
	{
		EquippedActor->Destroy();
	}
}

void FInv_EquipmentFragment::SetEquippedActor(AInv_EquipActor* EquipActor)
{
	EquippedActor = EquipActor;
}
