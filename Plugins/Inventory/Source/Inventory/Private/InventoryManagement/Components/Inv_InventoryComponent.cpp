// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagement/Components/Inv_InventoryComponent.h"

#include "Items/Components/Inv_ItemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Inventory/InventoryBase/Inv_InventoryBase.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_ItemFragment.h"

UInv_InventoryComponent::UInv_InventoryComponent() : InventoryList(this)
{
	PrimaryComponentTick.bCanEverTick = false;
	bInventoryMenuOpen = false;
	// Sets replication 
	SetIsReplicatedByDefault(true); 
	bReplicateUsingRegisteredSubObjectList = true;
}

void UInv_InventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Needed for replication 
	DOREPLIFETIME(ThisClass, InventoryList);
}

void UInv_InventoryComponent::Server_DropItem_Implementation(UInv_InventoryItem* Item, int32 StackCount)
{
	const int32 NewStackCount = Item->GetTotalStackCount() - StackCount;
	if (NewStackCount <= 0)
	{
		InventoryList.RemoveEntry(Item); 
	}
	else
	{
		Item->SetTotalStackCount(NewStackCount);
	}

	SpawnDroppedItem(Item, StackCount); 
}

void UInv_InventoryComponent::SpawnDroppedItem(UInv_InventoryItem* Item, int32 StackCount)
{
	// Spawn the dropped item in the level, creating it at a random angle 
	const APawn* OwningPawn = OwningController->GetPawn();
	
	FVector RotatedForward = OwningPawn->GetActorForwardVector();
	RotatedForward = RotatedForward.RotateAngleAxis(FMath::FRandRange(DropSpawnAngleMin, DropSpawnAngleMax), FVector::UpVector);

	FVector SpawnLocation = OwningPawn->GetActorLocation() + RotatedForward * FMath::FRandRange(DropSpawnDistanceMin, DropSpawnDistanceMax);
	SpawnLocation.Z -= RelativeSpawnElevation;
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	// Have the item manifest spawn the pickup actor
	FInv_ItemManifest& ItemManifest = Item->GetItemManifestMutable();
	if (FInv_StackableFragment* StackableFragment = ItemManifest.GetFragmentOfTypeMutable<FInv_StackableFragment>())
	{
		StackableFragment->SetStackCount(StackCount);
	}
	ItemManifest.SpawnPickUpActor(this, SpawnLocation, SpawnRotation); 
}

void UInv_InventoryComponent::ToggleInventoryMenu()
{
	// Toggles inventory menu 
	if (bInventoryMenuOpen)
	{
		CloseInventoryMenu();
	}
	else
	{
		OpenInventoryMenu();
	}
}

void UInv_InventoryComponent::AddRepSubObj(UObject* SubObj)
{
	// Needed for replication 
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && IsValid(SubObj))
	{
		AddReplicatedSubObject(SubObj);		
	}
}

void UInv_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Create inventory 
	ConstructInventory(); 
}

void UInv_InventoryComponent::TryAddItem(UInv_ItemComponent* ItemComponent)
{
	// Calls HasRoomForItem function on SpatialInventory which overrides the Parent function
	// Client first checks if there's even room in inventory before replicating any changes 
	FInv_SlotAvailabilityResult Result = InventoryMenu->HasRoomForItem(ItemComponent);

	// Sets InventoryItem on SlotAvailability result by checking the Array and passing in the ItemComponents GameplayTag
	UInv_InventoryItem* FoundItem = InventoryList.FindFirstItemByType(ItemComponent->GetItemManifest().GetItemType());
	Result.Item = FoundItem;

	// Checks if inventory is full 
	if (Result.TotalRoomToFill == 0)
	{
		// Broadcasts delegate when function is called 
		NoRoomInventory.Broadcast();
		return; 
	}

	// Only adds stacks to an item that already exists in the inventory 
	if (Result.Item.IsValid() && Result.bStackable)
	{
		OnStackChange.Broadcast(Result); 
		Server_AddStacksToItem(ItemComponent, Result.TotalRoomToFill, Result.Remainder);
	}
	// This item type doesn't exist in the inventory. Create a new one and update any slots 
	else if (Result.TotalRoomToFill > 0)
	{
		Server_AddNewItem(ItemComponent, Result.bStackable ? Result.TotalRoomToFill : 0);
	}
	
}

void UInv_InventoryComponent::Server_AddNewItem_Implementation(UInv_ItemComponent* ItemComponent, int32 StackCount)
{
	// Calls Fast Array function to add entry based on item component
	// This is then replicated 
	UInv_InventoryItem* NewItem = InventoryList.AddEntry(ItemComponent);

	NewItem->SetTotalStackCount(StackCount);

	// Handles broadcasting the delegate for listen and standalone
	// Needed because for these modes the player is the host, not the client 
	if (GetOwner()->GetNetMode() == NM_ListenServer || GetOwner()->GetNetMode() == NM_Standalone)
	{
		// Broadcasts to the InventoryGrid the actual item to do changes with it 
		OnItemAdded.Broadcast(NewItem);
	}
	
	ItemComponent->PickedUp();
}


void UInv_InventoryComponent::Server_AddStacksToItem_Implementation(UInv_ItemComponent* ItemComponent, int32 StackCount,
	int32 Remainder)
{
	// Retrieves GameplayTag of ItemComponent by checking manifest. Then checks InventoryList array for matching item
	const FGameplayTag& ItemType = IsValid(ItemComponent) ? ItemComponent->GetItemManifest().GetItemType() : FGameplayTag::EmptyTag;
	UInv_InventoryItem* Item = InventoryList.FindFirstItemByType(ItemType);
	if (!IsValid(Item)) return;

	// Sets TotalStackCount of item by checking what it's total is and adding it to the passed in StackCount 
	Item->SetTotalStackCount(Item->GetTotalStackCount() + StackCount);

	// If there is no more remainder, the item is picked up and the owner is destroyed 
	if (Remainder == 0)
	{
		ItemComponent->PickedUp();
	}
	// If there is still a remainder, we will get the ItemComponent's stackable fragment and set the Item's StackCount to the remainder 
	else if (FInv_StackableFragment* StackableFragment = ItemComponent->GetItemManifest().GetFragmentOfTypeMutable<FInv_StackableFragment>())
	{
		StackableFragment->SetStackCount(Remainder);
	}
}

void UInv_InventoryComponent::ConstructInventory()
{
	// Get Player controller by casting the result of the actor function GetOwner
	// Check if it's valid and make sure that if it's not valid there is an early return 
	OwningController = Cast<APlayerController>(GetOwner());
	checkf(OwningController.IsValid(), TEXT("Inventory component should have a Player Controller as Owner."));
	if (!OwningController.IsValid()) return;

	// Create widget based on InventoryBaseWidget. Since OwningController is weak get actual. Based on InventoryClass 
	InventoryMenu = CreateWidget<UInv_InventoryBase>(OwningController.Get(), InventoryMenuClass);
	InventoryMenu->AddToViewport();

	// After creating set inventory to collapsed 
	CloseInventoryMenu(); 
}

void UInv_InventoryComponent::OpenInventoryMenu()
{
	// Check if inventory menu is valid, then set visibility and toggle variable 
	if (!IsValid(InventoryMenu)) return;

	InventoryMenu->SetVisibility(ESlateVisibility::Visible);
	bInventoryMenuOpen = true;

	// Check if owning controller is valid, then set input mode to UI 
	if (!OwningController.IsValid()) return;

	FInputModeGameAndUI InputMode;
	OwningController->SetInputMode(InputMode);
	OwningController->SetShowMouseCursor(true);
	
}

void UInv_InventoryComponent::CloseInventoryMenu()
{
	// Check if inventory menu is valid, then collapse and toggle variable 
	if (!IsValid(InventoryMenu)) return;

	InventoryMenu->SetVisibility(ESlateVisibility::Collapsed);
	bInventoryMenuOpen = false;

	// Check if owning controller is valid, then reenable input to game  
	if (!OwningController.IsValid()) return;

	FInputModeGameOnly InputMode;
	OwningController->SetInputMode(InputMode);
	OwningController->SetShowMouseCursor(false);
}



