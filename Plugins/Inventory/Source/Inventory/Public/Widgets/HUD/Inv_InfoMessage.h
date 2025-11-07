// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inv_InfoMessage.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_InfoMessage : public UUserWidget
{
	GENERATED_BODY()

public:
	// Called on game initialization 
	virtual void NativeOnInitialized() override;

	// BP events to handle showing and hiding message. Functionality handled in bp 
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void MessageShow();

	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void MessageHide();

	// Sets initial message 
	void SetMessage(const FText& Message);
	
private:

	// Message properties 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Message;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float MessageLifeTime{3.f};

	FTimerHandle MessageTimer;
	bool bIsMessageActive{false}; 
};
