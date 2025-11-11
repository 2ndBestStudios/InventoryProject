// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Inv_WidgetUtils.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_WidgetUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// Allows us to calculate index location based on passed in parameters. FIntPoint can take X & Y 
	static int32 GetIndexFromPosition(const FIntPoint& Position, const int32 Columns);
	// Allows us to get x,y position from a given index 
	static FIntPoint GetPositionFromIndex(const int32 Index, const int32 Columns);
};
