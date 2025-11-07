#include "InventoryManagement/FastArray/Inv_FastArray.h"

#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"

TArray<UInv_InventoryItem*> FInv_InventoryFastArray::GetAllItems() const
{
	// temporary array is created to hold items. array size is preallocated 
	TArray<UInv_InventoryItem*> Result;
	Result.Reserve(Entries.Num());
	// iterates over all actual items 
	for (const auto& Entry : Entries)
	{
		// checks validity. adds to result array 
		if (!IsValid(Entry.Item)) continue;
		Result.Add(Entry.Item);
	}
	return Result; 
}

void FInv_InventoryFastArray::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	// cast owner component which owns array to inventory component 
	UInv_InventoryComponent* IC = Cast<UInv_InventoryComponent>(OwnerComponent);
	if (!IsValid(IC)) return;

	// iterates through removed indices which are entries being removed 
	for (int32 Index : RemovedIndices)
	{
		// for each removed index a delegate is called to the inventory component passing the item being removed
		IC->OnItemRemoved.Broadcast(Entries[Index].Item);
	}
}

void FInv_InventoryFastArray::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	// cast owner component which owns array to inventory component 
	UInv_InventoryComponent* IC = Cast<UInv_InventoryComponent>(OwnerComponent);
	if (!IsValid(IC)) return;

	for (int32 Index : AddedIndices)
	{
		// for each added index a delegate is called to the inventory component passing the item being added 
		IC->OnItemAdded.Broadcast(Entries[Index].Item);
	}
}

UInv_InventoryItem* FInv_InventoryFastArray::AddEntry(UInv_ItemComponent* ItemComponent)
{
	// ensures it is only called on valid component and only on the server. originates from authoritative source 
	check(OwnerComponent);
	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());
	// cast owner component which owns array to inventory component 
	UInv_InventoryComponent* IC = Cast<UInv_InventoryComponent>(OwnerComponent);
	if (!IsValid(IC)) return nullptr;

	// New entry is added to entries 
	FInv_InventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	// Item is assigned to entry by checking itemcomponent, then getting manifest, then calling manifest 
	NewEntry.Item = ItemComponent->GetItemManifest().Manifest(OwningActor);

	// Required for replication 
	IC->AddRepSubObj(NewEntry.Item);
	MarkItemDirty(NewEntry);

	return NewEntry.Item;
}

UInv_InventoryItem* FInv_InventoryFastArray::AddEntry(UInv_InventoryItem* Item)
{
	// ensures it is only called on valid component and only on the server. originates from authoritative source 
	check (OwnerComponent);
	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());

	// new entry is added to entries which the item is then stored in 
	FInv_InventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Item = Item;

	// tells replication that the item entry has changed and needs to be synced 
	MarkItemDirty(NewEntry);
	return Item; 
}

void FInv_InventoryFastArray::RemoveEntry(UInv_InventoryItem* Item)
{
	// iterates over entries using an iterator 
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		// finds entry whose item matches the one being removed 
		FInv_InventoryEntry& Entry = *EntryIt;
		if (Entry.Item == Item)
		{
			// removes entry and tells replication that entire array structure has changed 
			EntryIt.RemoveCurrent();
			MarkArrayDirty(); 
		}
	}
}
