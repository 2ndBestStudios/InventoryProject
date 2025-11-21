#include "Items/Fragments/Inv_ItemFragment.h"

#include "Widgets/Composite/Inv_CompositeBase.h"

void FInv_InventoryItemFragment::Assimilate(UInv_CompositeBase* CompositeBase) const
{
	if (!MatchesWidgetTag(CompositeBase)) return; 
	CompositeBase->Expand(); 
}

bool FInv_InventoryItemFragment::MatchesWidgetTag(const UInv_CompositeBase* CompositeBase) const
{
	return CompositeBase->GetFragmentTag().MatchesTagExact(GetFragmentTag());
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
