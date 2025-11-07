// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/Inv_InfoMessage.h"

#include "Components/TextBlock.h"

void UInv_InfoMessage::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Sets text to empty. Calls message hide bp event 
	Text_Message->SetText(FText::GetEmpty());
	MessageHide();
}

// Called from HUD Widget 
void UInv_InfoMessage::SetMessage(const FText& Message)
{
	// Sets text from HUD widget call
	Text_Message->SetText(Message);

	// Checks if a message is already active 
	if (!bIsMessageActive)
	{
		// Calls MessageShow BP Event 
		MessageShow(); 
	}
	// Sets to true
	bIsMessageActive = true;

	// Begins message timer
	GetWorld()->GetTimerManager().SetTimer(MessageTimer, [this]()
	{
		// Hides message. Disables active. Resets timer 
		MessageHide();
		bIsMessageActive = false;
	},MessageLifeTime, false);
}
