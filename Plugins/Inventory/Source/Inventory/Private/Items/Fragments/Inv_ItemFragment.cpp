#include "Items/Fragments/Inv_ItemFragment.h"

#include "Widgets/Composite/Inv_CompositeBase.h"
#include "Widgets/Composite/Inv_Leaf_Image.h"
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

void FInv_HealthPotionFragment::OnConsume(APlayerController* PC)
{
	// Get a stats component from the PC or get the AbilitySystemComponent and apply a gameplay effect

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Health potion consumed!")));
}

void FInv_ManaPotionFragment::OnConsume(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Mana potion consumed!")));
}
