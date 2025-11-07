// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Inv_PlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Interaction/Inv_Highlightable.h"
#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/HUD/Inv_HUDWidget.h"


AInv_PlayerController::AInv_PlayerController()
{
	// Line trace parameters 
	PrimaryActorTick.bCanEverTick = true;
	TraceLength = 500.0;
	ItemTraceChannel = ECC_GameTraceChannel1; 
}

void AInv_PlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Continually traces for items 
	TraceForItem();
}

void AInv_PlayerController::ToggleInventory()
{
	// Checks if inventory component is valid. Then calls toggle inventory function 
	if (!InventoryComponent.IsValid()) return;
	InventoryComponent->ToggleInventoryMenu(); 
}

void AInv_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	//Setup mapping context. Loop through array of contexts
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (IsValid(Subsystem))
	{
		for (UInputMappingContext* CurrentContext : DefaultInputMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}

	// Sets inventory component that was set in blueprint 
	InventoryComponent = FindComponentByClass<UInv_InventoryComponent>();

	//Create HUD
	CreateHUDWidget();
}

void AInv_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	//Cast input component from Actor to Enhanced Input
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	//Bind input action
	EnhancedInputComponent->BindAction(PrimaryInteractAction, ETriggerEvent::Started, this, &AInv_PlayerController::PrimaryInteract);
	EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &AInv_PlayerController::ToggleInventory);

}

void AInv_PlayerController::PrimaryInteract()
{
	if (!ThisActor.IsValid()) return;
	// Gets inventory component and checks if valid 
	UInv_ItemComponent* ItemComp = ThisActor->FindComponentByClass<UInv_ItemComponent>();
	if (!IsValid(ItemComp) || !InventoryComponent.IsValid()) return;
	// Calls tryadditem function on inventorycomponent which broadcasts delegate 
	InventoryComponent->TryAddItem(ItemComp); 
}

void AInv_PlayerController::CreateHUDWidget()
{
	// Prevents server from creating widget 
	if (!IsLocalController()) return;

	// Create HUD and add to viewport 
	HUDWidget = CreateWidget<UInv_HUDWidget>(this, HUDWidgetClass);
	if (IsValid(HUDWidget))
	{
		HUDWidget->AddToViewport();
	}
	
}

void AInv_PlayerController::TraceForItem()
{
	// Early return if GEngine isn't valid 
	if (!IsValid(GEngine) || !IsValid(GEngine->GameViewport)) return;

	// Get viewport size and find middle. Create empty vectors to hold information from ProjectScreenToWorld
	// Call ProjectScreenToWorld
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	const FVector2D ViewportCenter = ViewportSize / 2.f;
	FVector TraceStart;
	FVector Forward;
	if (!UGameplayStatics::DeprojectScreenToWorld(this, ViewportCenter, TraceStart, Forward)) return;

	// Calculate Trace End from filled out Vectors. Create empty hitresult that is filled by linetrace 
	const FVector TraceEnd = TraceStart + (Forward * TraceLength);
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ItemTraceChannel);

	// Rewatch section to understand again 
	LastActor = ThisActor;
	ThisActor = HitResult.GetActor();

	if (!ThisActor.IsValid())
	{
		if (IsValid(HUDWidget))
		{
			HUDWidget->HidePickupMessage(); 
		}
	}

	if (ThisActor == LastActor) return;

	if (ThisActor.IsValid())
	{
		if (UActorComponent* Highlightable = ThisActor->FindComponentByInterface(UInv_Highlightable::StaticClass()); IsValid(Highlightable))
		{
			IInv_Highlightable::Execute_Highlight(Highlightable);
		}
		
		UInv_ItemComponent* ItemComponent = ThisActor->FindComponentByClass<UInv_ItemComponent>();
		if (!IsValid(ItemComponent)) return;

		if (IsValid(HUDWidget))
		{
			HUDWidget->ShowPickupMessage(ItemComponent->GetPickupMessage());
		}
	}

	if (LastActor.IsValid())
	{
		if (UActorComponent* Highlightable = LastActor->FindComponentByInterface(UInv_Highlightable::StaticClass()); IsValid(Highlightable))
		{
			IInv_Highlightable::Execute_UnHighlight(Highlightable);
		}
	}
}
