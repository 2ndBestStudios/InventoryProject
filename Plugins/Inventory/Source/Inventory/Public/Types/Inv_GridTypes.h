#pragma once

#include "Inv_GridTypes.generated.h"

// Defines the different item categories we can have 
UENUM(BlueprintType)
enum class EInv_ItemCategory : uint8
{
	Equippable,
	Consumable,
	Craftable,
	None 
};
