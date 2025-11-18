// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"

#include "Inventory.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Items/Fragments/Inv_FragmentTags.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/GridSlots/Inv_GridSlot.h"
#include "Widgets/Utils/Inv_WidgetUtils.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"
#include "Widgets/Inventory/SlottedItems/Inv_SlottedItem.h"
#include "Widgets/ItemPopUp/Inv_ItemPopUp.h"

void UInv_InventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ConstructGrid();

	// Fills out inventory component reference and subscribes to callback 
	InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	InventoryComponent->OnItemAdded.AddDynamic(this, &ThisClass::AddItem);
	InventoryComponent->OnStackChange.AddDynamic(this, &ThisClass::AddStacks); 
}

void UInv_InventoryGrid::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Get mouse position relative to where in the canvas panel it would be
	// First get canvas position in viewport 
	const FVector2D CanvasPosition = UInv_WidgetUtils::GetWidgetPosition(CanvasPanel);
	
	// Get mouse position in viewport 
	const FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());

	if (CursorExitedCanvas(CanvasPosition, UInv_WidgetUtils::GetWidgetSize(CanvasPanel), MousePosition))
	{
		return;
	}

	UpdateTileParameters(CanvasPosition, MousePosition);
}

bool UInv_InventoryGrid::CursorExitedCanvas(const FVector2D& CanvasPosition, const FVector2D& BoundarySize,
	const FVector2D& Location)
{
	bLastMouseWithinCanvas = bMouseWithinCanvas;

	bMouseWithinCanvas = UInv_WidgetUtils::IsWithinBounds(CanvasPosition, BoundarySize, Location);
	if (!bMouseWithinCanvas && bLastMouseWithinCanvas)
	{
		UnHighlightSlots(LastHighlightedIndex, LastHighlightedDimensions);
		return true;
	}
	return false; 
}

void UInv_InventoryGrid::UpdateTileParameters(const FVector2D& CanvasPosition, const FVector2D& MousePosition)
{
	// If mouse not in canvas panel, return
	if (!bMouseWithinCanvas) return; 
	
	// Calculate tile quadrant, tile index, and coordinates
	const FIntPoint HoveredTileCoordinates = CalculateHoveredCoordinates(CanvasPosition, MousePosition);

	// Fill out TileParameters 
	LastTileParameters = TileParameters;
	TileParameters.TileCoordinates = HoveredTileCoordinates;
	TileParameters.TileIndex = UInv_WidgetUtils::GetIndexFromPosition(HoveredTileCoordinates, Columns);
	TileParameters.TileQuadrant = CalculateTileQuadrant(CanvasPosition, MousePosition);
	
	// Handle highlight/unhighlight of GridSlots
	OnTileParametersUpdated(TileParameters); 
	
}

FIntPoint UInv_InventoryGrid::CalculateHoveredCoordinates(const FVector2D& CanvasPosition,
	const FVector2D& MousePosition) const
{
	// Floor to Int is 64 bit int. Static Cast converts to 32
	return FIntPoint{
		static_cast<int32>(FMath::FloorToInt((MousePosition.X - CanvasPosition.X)/ TileSize)),
		static_cast<int32>(FMath::FloorToInt((MousePosition.Y - CanvasPosition.Y)/ TileSize))
	};
}

EInv_TileQuadrant UInv_InventoryGrid::CalculateTileQuadrant(const FVector2D CanvasPosition,
	const FVector2D MousePosition) const
{
	// Calculate relative position within the current tile
	const float TileLocalX = FMath::Fmod(MousePosition.X - CanvasPosition.X, TileSize);
	const float TileLocalY = FMath::Fmod(MousePosition.Y - CanvasPosition.Y, TileSize);

	// Determine which quadrant the mouse is in
	const bool bIsTop = TileLocalY < TileSize / 2.f;  // Top if Y is in the upper half
	const bool bIsLeft = TileLocalX < TileSize / 2.f; // Left is X is in the left half

	EInv_TileQuadrant HoveredTileQuadrant{EInv_TileQuadrant::None};
	if (bIsTop && bIsLeft) HoveredTileQuadrant = EInv_TileQuadrant::TopLeft;
	else if (bIsTop && !bIsLeft) HoveredTileQuadrant = EInv_TileQuadrant::TopRight;
	else if (!bIsTop && bIsLeft) HoveredTileQuadrant = EInv_TileQuadrant::BottomLeft;
	else if (!bIsTop && !bIsLeft) HoveredTileQuadrant = EInv_TileQuadrant::BottomRight;

	return HoveredTileQuadrant;
	
}

void UInv_InventoryGrid::OnTileParametersUpdated(const FInv_TileParameters& Parameters)
{
	if (!IsValid(HoverItem)) return;

	// Get Hover Item's dimensions
	const FIntPoint Dimensions = HoverItem->GetGridDimensions(); 
	
	// Calculate the starting coordinate for highlighting
	const FIntPoint StartingCoordinate = CalculateStartingCoordinate(Parameters.TileCoordinates, Dimensions, Parameters.TileQuadrant);

	ItemDropIndex = UInv_WidgetUtils::GetIndexFromPosition(StartingCoordinate, Columns); 

	CurrentSpaceQueryResult = CheckHoverPosition(StartingCoordinate, Dimensions);

	if (CurrentSpaceQueryResult.bHasSpace)
	{
		HighlightSlots(ItemDropIndex, Dimensions);
		return;
	}
	UnHighlightSlots(LastHighlightedIndex, LastHighlightedDimensions);

	if (CurrentSpaceQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentSpaceQueryResult.UpperLeftIndex))
	{
		const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(CurrentSpaceQueryResult.ValidItem.Get(), FragmentTags::GridFragment);
		if (!GridFragment) return;

		ChangeHoverType(CurrentSpaceQueryResult.UpperLeftIndex, GridFragment->GetGridSize(), EInv_GridSlotState::GrayedOut);
	}
}

FIntPoint UInv_InventoryGrid::CalculateStartingCoordinate(const FIntPoint& Coordinate, const FIntPoint& Dimensions,
	const EInv_TileQuadrant Quadrant) const
{
	// If the item has an even width it'll add extra padding when moving the item over 
	const int32 HasEvenWidth = Dimensions.X % 2 == 0 ? 1 : 0;
	const int32 HasEvenHeight = Dimensions.Y % 2 == 0 ? 1 : 0;

	FIntPoint StartingCoordinate;
	switch (Quadrant)
	{
		case EInv_TileQuadrant::TopLeft:
			StartingCoordinate.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X);
			StartingCoordinate.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y);
		break;
		case EInv_TileQuadrant::TopRight:
			StartingCoordinate.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X) + HasEvenWidth;
			StartingCoordinate.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y);
		break;
		case EInv_TileQuadrant::BottomLeft:
			StartingCoordinate.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X);
			StartingCoordinate.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y) + HasEvenHeight;
		break;
		case EInv_TileQuadrant::BottomRight:
			StartingCoordinate.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X) + HasEvenWidth;
			StartingCoordinate.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y) + HasEvenHeight;
		break;
		default:
			UE_LOG(LogInventory, Error, TEXT("Invalid quadrant"))
			return FIntPoint(-1,-1); 
	}
	return StartingCoordinate;
}

FInv_SpaceQueryResult UInv_InventoryGrid::CheckHoverPosition(const FIntPoint& Position,
	const FIntPoint& Dimensions) 
{
	FInv_SpaceQueryResult Result;
	
	// Is it in the grid bounds?
	if (!IsInGridBounds(UInv_WidgetUtils::GetIndexFromPosition(Position, Columns), Dimensions)) return Result;

	Result.bHasSpace = true; 
	
	// If more than one of the indices is occupied with the same item, we need to see if they all have the same upper left index
	TSet<int32> OccupiedUpperLeftIndices;
	
	UInv_InventoryStatics::ForEach2D(GridSlots, UInv_WidgetUtils::GetIndexFromPosition(Position, Columns), Dimensions, Columns, [&](const UInv_GridSlot* GridSlot)
	{
		if (GridSlot->GetInventoryItem().IsValid())
		{
			OccupiedUpperLeftIndices.Add(GridSlot->GetUpperLeftIndex());
			Result.bHasSpace = false; 
		}
	}); 
	
	// If so, is there only one item in the way? (can we stop?)
	if (OccupiedUpperLeftIndices.Num() == 1) // Single item at position - It's valid for swapping / combining 
	{
		const int32 Index = *OccupiedUpperLeftIndices.CreateConstIterator();
		Result.ValidItem = GridSlots[Index]->GetInventoryItem();
		Result.UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex();
	}

	return Result;
}

void UInv_InventoryGrid::HighlightSlots(const int32 Index, const FIntPoint& Dimensions)
{
	if (!bMouseWithinCanvas) return;

	UnHighlightSlots(LastHighlightedIndex, LastHighlightedDimensions); 
	
	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [&](UInv_GridSlot* GridSlot)
	{
		GridSlot->SetOccupiedTexture(); 
	});
	LastHighlightedDimensions = Dimensions;
	LastHighlightedIndex = Index;
}

void UInv_InventoryGrid::UnHighlightSlots(const int32 Index, const FIntPoint& Dimensions)
{
	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [&](UInv_GridSlot* GridSlot)
	{
		if (GridSlot->IsAvailable())
		{
			GridSlot->SetUnoccupiedTexture();
		}
		else
		{
			GridSlot->SetOccupiedTexture();
		}
	});
}

void UInv_InventoryGrid::ChangeHoverType(const int32 Index, const FIntPoint& Dimensions,
	EInv_GridSlotState GridSlotState)
{
	UnHighlightSlots(LastHighlightedIndex, LastHighlightedDimensions);

	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [State = GridSlotState](UInv_GridSlot* GridSlot)
	{
		switch (State)
		{
		case EInv_GridSlotState::Occupied:
			GridSlot->SetOccupiedTexture();
			break;
		case EInv_GridSlotState::Unoccupied:
			GridSlot->SetUnoccupiedTexture();
			break;
		case EInv_GridSlotState::Selected:
			GridSlot->SetSelectedTexture();
			break;
		case EInv_GridSlotState::GrayedOut:
			GridSlot->SetGrayedOutTexture();
			break;
		}
	});

	LastHighlightedIndex = Index;
	LastHighlightedDimensions = Dimensions;
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_ItemComponent* ItemComponent)
{
	// Calls manifest version 
	return HasRoomForItem(ItemComponent->GetItemManifest());
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_InventoryItem* Item)
{
	// Calls manifest version 
	return HasRoomForItem(Item->GetItemManifest());
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const FInv_ItemManifest& Manifest)
{
	FInv_SlotAvailabilityResult Result;

	// Determine if the item is stackable
	const FInv_StackableFragment* StackableFragment = Manifest.GetFragmentOfType<FInv_StackableFragment>();
	Result.bStackable = StackableFragment != nullptr;
	
	// Determine how many stacks to add
	const int32 MaxStackSize = StackableFragment ? StackableFragment->GetMaxStackSize() : 1; 
	int32 AmountToFill = StackableFragment ? StackableFragment->GetStackCount() : 1;

	TSet<int32> CheckedIndices;
	
	// For each GridSlot:
	for (const auto& GridSlot : GridSlots)
	{
		// If we don't have anymore to fill, break out of the entire loop 
		if (AmountToFill == 0) break; 
		
		// If an index is already filled, loop through next index 
		if (IsIndexClaimed(CheckedIndices, GridSlot->GetTileIndex())) continue; 

		// Is the item in GridBounds
		if (!IsInGridBounds(GridSlot->GetTileIndex(), GetItemDimensions(Manifest))) continue; 
		
		TSet<int32> TentativelyClaimedIndices;
		// Can the item fit here? (i.e. is it out of GridBounds)
		if (!HasRoomAtIndex(GridSlot, GetItemDimensions(Manifest), CheckedIndices, TentativelyClaimedIndices, Manifest.GetItemType(), MaxStackSize))
		{
			continue;
		}
		
		// How much to fill?
		const int32 AmountToFillInSlot = DetermineFillAmountForSlot(Result.bStackable, MaxStackSize, AmountToFill, GridSlot);
		if (AmountToFillInSlot == 0) continue;

		CheckedIndices.Append(TentativelyClaimedIndices);
		
		// Update the amount left to fill
		Result.TotalRoomToFill += AmountToFillInSlot;
		// Creates a new SlotAvailability, using conditional operators to set the constructor variables 
		Result.SlotAvailabilities.Emplace(
			FInv_SlotAvailability{
				HasValidItem(GridSlot) ? GridSlot->GetUpperLeftIndex() : GridSlot->GetTileIndex(),
				Result.bStackable ? AmountToFillInSlot : 0,
				HasValidItem(GridSlot)
			}
		);
		// Keep a running total of how much room is left to fill 
		AmountToFill -= AmountToFillInSlot;

		// How much is the remainder? 
		Result.Remainder = AmountToFill;

		// Early return prevents checking the rest of the inventory when its not needed 
		if (AmountToFill == 0) return Result; 
	}
	
	
 	return Result; 
}

bool UInv_InventoryGrid::HasRoomAtIndex(const UInv_GridSlot* GridSlot,
		const FIntPoint& Dimensions,
		const TSet<int32>& CheckedIndices,
		TSet<int32>& OutTentativelyClaimedIndices,
		const FGameplayTag& ItemType,
		const int32 MaxStackSize)
{
	// Is there room at this index? (i.e. is there other items in the way?)
	// If we have room at index, loop through GridSlot indices by dimensions and columns, 
	bool bHasRoomAtIndex = true;

	UInv_InventoryStatics::ForEach2D(GridSlots,GridSlot->GetTileIndex(), Dimensions, Columns, [&](const UInv_GridSlot* SubGridSlot)
	{
		// If the slot is free, we can add to TentativelyClaimedIndices. If not, return false. 
		if (CheckSlotConstraints(GridSlot, SubGridSlot, CheckedIndices, OutTentativelyClaimedIndices, ItemType, MaxStackSize))
		{
			OutTentativelyClaimedIndices.Add(SubGridSlot->GetTileIndex());
		}
		else
		{
			bHasRoomAtIndex = false;
		}
	});
	
	return bHasRoomAtIndex; 
}

void UInv_InventoryGrid::AddItem(UInv_InventoryItem* Item)
{
	// Checks if categories match 
	if (!MatchesCategory(Item)) return;

	// Checks for Room for Items from Server broadcast 
	FInv_SlotAvailabilityResult Result = HasRoomForItem(Item);

	// Allows us to start performing some actions based on the item 
	AddItemToIndices(Result, Item);
}

void UInv_InventoryGrid::AddItemToIndices(const FInv_SlotAvailabilityResult& Result, UInv_InventoryItem* NewItem)
{
	// Loops through SlotAvailabilities from SlotAvailabilityResult. For each available slot it checks index, stackable, and amount to fill
	// Then updates the background of the GridSlot 
	for (const auto& Availability : Result.SlotAvailabilities)
	{
		AddItemAtIndex(NewItem, Availability.Index, Result.bStackable, Availability.AmountToFill);
		UpdateGridSlots(NewItem, Availability.Index, Result.bStackable, Availability.AmountToFill); 
	}
}

void UInv_InventoryGrid::AddItemAtIndex(UInv_InventoryItem* Item, const int32 Index, const bool bStackable,
	const int32 StackAmount)
{
	// Uses NewItem (InventoryItem) to get its ItemManifest, which then returns the GridFragment that was set
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(Item, FragmentTags::GridFragment);
	const FInv_ImageFragment* ImageFragment = GetFragment<FInv_ImageFragment>(Item, FragmentTags::IconFragment);
	if (!GridFragment || !ImageFragment) return;
	// Calls below function 
	UInv_SlottedItem* SlottedItem = CreateSlottedItem(Item, bStackable, StackAmount, GridFragment, ImageFragment, Index);
	// Add Item to canvas 
	AddSlottedItemToCanvas(Index, GridFragment, SlottedItem);
	// Adds Item to Map for data manipulation later 
	SlottedItems.Add(Index,SlottedItem);
}

UInv_SlottedItem* UInv_InventoryGrid::CreateSlottedItem(UInv_InventoryItem* Item, const bool bStackable,
	const int32 StackAmount, const FInv_GridFragment* GridFragment, const FInv_ImageFragment* ImageFragment,
	const int32 Index)
{
	// Creates a GridSlotItem Widget based on SlottedItem with owner set by PlayerController 
	UInv_SlottedItem* SlottedItem = CreateWidget<UInv_SlottedItem>(GetOwningPlayer(), SlottedItemClass);
	// Sets the inventory item for the GridSlot 
	SlottedItem->SetInventoryItem(Item);
	// Creates GridSlotItem image 
	SetSlottedItemImage(SlottedItem,GridFragment,ImageFragment);
	// Sets index of GridSlotItem 
	SlottedItem->SetGridIndex(Index);

	// Sets the Stackable Property. Then checks if it's true. If it is, store StackAmount if not set to 0. Update stack
	SlottedItem->SetIsStackable(bStackable);
	const int32 StackUpdateAmount = bStackable ? StackAmount : 0;
	SlottedItem->UpdateStackCount(StackUpdateAmount);

	// Binds every slotted item created to the OnSlottedItemClicked Delegate 
	SlottedItem->OnSlottedItemClicked.AddDynamic(this, &ThisClass::OnSlottedItemClicked);
	
	// Returns the Slotted Item
	return SlottedItem;
}

void UInv_InventoryGrid::SetSlottedItemImage(const UInv_SlottedItem* SlottedItem, const FInv_GridFragment* GridFragment,
                                             const FInv_ImageFragment* ImageFragment) const
{
	// Create an empty brush that is then set to the ImageFragment. Set brush value to draw as image 
	FSlateBrush Brush;
	Brush.SetResourceObject(ImageFragment->GetIcon());
	Brush.DrawAs = ESlateBrushDrawType::Image;
	// Calls below function
	Brush.ImageSize = GetDrawSize(GridFragment);
	// Calls the individual GridSlot item to set its own ImageBrush 
	SlottedItem->SetImageBrush(Brush);
}

FVector2D UInv_InventoryGrid::GetDrawSize(const FInv_GridFragment* GridFragment) const 
{
	// Takes TileSize of Grid, subtracts from padding of fragment, and multiples by 2 for either side 
	const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2;
	return GridFragment->GetGridSize() * IconTileWidth;
}

void UInv_InventoryGrid::AddSlottedItemToCanvas(const int32 Index, const FInv_GridFragment* GridFragment,
	UInv_SlottedItem* SlottedItem) const
{
	// Adds Child 
	CanvasPanel->AddChild(SlottedItem);
	// Sets the slot of the canvas panel, then draws based on GridFragmentSize 
	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(SlottedItem);
	CanvasSlot->SetSize(GetDrawSize(GridFragment));
	// Calculates DrawPosition based on Index and Column #. Then incorporates padding 
	const FVector2D DrawPos = UInv_WidgetUtils::GetPositionFromIndex(Index, Columns) * TileSize;
	const FVector2D DrawPosWithPadding = DrawPos + FVector2D(GridFragment->GetGridPadding());
	// Sets position 
	CanvasSlot->SetPosition(DrawPosWithPadding);
}

void UInv_InventoryGrid::UpdateGridSlots(UInv_InventoryItem* NewItem, const int32 Index, bool bStackableItem, const int32 StackAmount)
{
	// Checks if there's a valid index 
	check(GridSlots.IsValidIndex(Index));

	// If the item is stackable. Set the given Index in the array of GridSlots, the StackAmount 
	if (bStackableItem)
	{
		GridSlots[Index]->SetStackCount(StackAmount); 
	}

	// Capture GridFragment by template function taking in Inventory Item and a GameplayTag. 
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(NewItem, FragmentTags::GridFragment);
	// Checks the GridFragment from the item for its size. Then stores that value. 
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1,1);
	// Loops through GridSlot array, passing in the index and dimensions, calling the below functions 
	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [&](UInv_GridSlot* GridSlot)
	{
		GridSlot->SetInventoryItem(NewItem);
		GridSlot->SetUpperLeftIndex(Index);
		GridSlot->SetOccupiedTexture();
		GridSlot->SetAvailable(false); 
	});
}

bool UInv_InventoryGrid::CheckSlotConstraints(const UInv_GridSlot* GridSlot,
	const UInv_GridSlot* SubGridSlot,
	const TSet<int32>& CheckedIndices,
	TSet<int32>& OutTentativelyClaimedIndices,
	const FGameplayTag& ItemType,
	const int32 MaxStackSize) const
{
	// Index claimed? If there is no claim, return true. 
	if (IsIndexClaimed(CheckedIndices, SubGridSlot->GetTileIndex())) return false;
	
	// Has valid item? If there is no item, add to TentativelyClaimedIndices 
	if (!HasValidItem(SubGridSlot))
	{
		OutTentativelyClaimedIndices.Add(SubGridSlot->GetTileIndex());
		return true;
	}

	// Is this GridSlot an UpperLeftSlot 
	if (!IsUpperLeftSlot(GridSlot, SubGridSlot)) return false;

	// If so, is this a stackable item? Return InventoryItem from SubGridSlot, check if stackable 
	const UInv_InventoryItem* SubItem = SubGridSlot->GetInventoryItem().Get();
	if (!SubItem->IsStackable()) return false;
	
	// Is this item the same type as the item we're trying to add?
	if (!DoesItemTypeMatch(SubItem, ItemType)) return false;
	
	// If stackable, is this slot at the max stack size already?
	if (GridSlot->GetStackCount() >= MaxStackSize) return false;
	
	return true; 
}

bool UInv_InventoryGrid::IsIndexClaimed(const TSet<int32>& CheckedIndices, const int32 Index) const
{
	return CheckedIndices.Contains(Index);
}

bool UInv_InventoryGrid::HasValidItem(const UInv_GridSlot* GridSlot) const 
{
	return GridSlot->GetInventoryItem().IsValid();
}

bool UInv_InventoryGrid::IsUpperLeftSlot(const UInv_GridSlot* GridSlot, const UInv_GridSlot* SubGridSlot) const
{
	return SubGridSlot->GetUpperLeftIndex() == GridSlot->GetTileIndex();
}

bool UInv_InventoryGrid::DoesItemTypeMatch(const UInv_InventoryItem* SubItem, const FGameplayTag& ItemType) const
{
	return SubItem->GetItemManifest().GetItemType().MatchesTagExact(ItemType);
}

bool UInv_InventoryGrid::IsInGridBounds(const int32 StartIndex, const FIntPoint& ItemDimensions) const 
{
	if (StartIndex < 0 || StartIndex >= GridSlots.Num()) return false;
	const int32 EndColumn = (StartIndex % Columns) + ItemDimensions.X;
	const int32 EndRow = (StartIndex / Columns) + ItemDimensions.Y;
	return EndColumn <= Columns && EndRow <= Rows;
}

bool UInv_InventoryGrid::IsRightClicked(const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::RightMouseButton; 
}

bool UInv_InventoryGrid::IsLeftClicked(const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
}

void UInv_InventoryGrid::PickUp(UInv_InventoryItem* ClickedInventoryItem, const int32 GridIndex)
{
	// Assign the HoverItem
	AssignHoverItem(ClickedInventoryItem, GridIndex, GridIndex);
	
	// Remove the clicked item from the grid
	RemoveItemFromGrid(ClickedInventoryItem, GridIndex);
}

void UInv_InventoryGrid::AssignHoverItem(UInv_InventoryItem* InventoryItem, const int32 GridIndex,
	const int32 PreviousGridIndex)
{
	AssignHoverItem(InventoryItem);

	HoverItem->SetPreviousGridIndex(PreviousGridIndex);
	HoverItem->UpdateStackCount(InventoryItem->IsStackable() ? GridSlots[GridIndex]->GetStackCount() : 0);
	
}

void UInv_InventoryGrid::RemoveItemFromGrid(UInv_InventoryItem* InventoryItem, const int32 GridIndex)
{
	// Get GridFragment from InventoryItem 
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(InventoryItem, FragmentTags::GridFragment);
	if (!GridFragment) return;

	// Loop through GridSlots at GridIndex by GridSize. For each GridSlot reset values 
	UInv_InventoryStatics::ForEach2D(GridSlots, GridIndex, GridFragment->GetGridSize(), Columns, [&](UInv_GridSlot* GridSlot)
	{
		GridSlot->SetInventoryItem(nullptr);
		GridSlot->SetUpperLeftIndex(INDEX_NONE);
		GridSlot->SetUnoccupiedTexture();
		GridSlot->SetAvailable(true);
		GridSlot->SetStackCount(0);
	});

	// Remove SlottedItem from array  
	if (SlottedItems.Contains(GridIndex))
	{
		TObjectPtr<UInv_SlottedItem> FoundSlottedItem;
		SlottedItems.RemoveAndCopyValue(GridIndex, FoundSlottedItem);
		FoundSlottedItem->RemoveFromParent(); 
	}
}

void UInv_InventoryGrid::AssignHoverItem(UInv_InventoryItem* InventoryItem)
{
	// Creates HoverItem widget
	if (!IsValid(HoverItem))
	{
		HoverItem = CreateWidget<UInv_HoverItem>(GetOwningPlayer(), HoverItemClass);
	}

	// Returns GridFragment and ImageFragment from InventoryItem
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(InventoryItem, FragmentTags::GridFragment);
	const FInv_ImageFragment* ImageFragment = GetFragment<FInv_ImageFragment>(InventoryItem, FragmentTags::IconFragment);
	if (!GridFragment || !ImageFragment) return;

	// Gets Item Size from GridFragment 
	const FVector2D DrawSize = GetDrawSize(GridFragment);

	// Sets IconBrush from ImageFragment 
	FSlateBrush IconBrush;
	IconBrush.SetResourceObject(ImageFragment->GetIcon());
	IconBrush.DrawAs = ESlateBrushDrawType::Image;
	IconBrush.ImageSize = DrawSize * UWidgetLayoutLibrary::GetViewportScale(this);

	// Sets the variables of the HoverItem variable 
	HoverItem->SetImageBrush(IconBrush);
	HoverItem->SetGridDimensions(GridFragment->GetGridSize());
	HoverItem->SetInventoryItem(InventoryItem);
	HoverItem->SetIsStackable(InventoryItem->IsStackable());

	// Changes the MouseCursor to the Widget when creating it 
	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, HoverItem);
	// Figure out how to add functionality to see mouse cursor change on picking up item 
}

int32 UInv_InventoryGrid::DetermineFillAmountForSlot(const bool bStackable, const int32 MaxStackSize,
                                                     const int32 AmountToFill, const UInv_GridSlot* GridSlot) const 
{
	// Get RoomInSlot by subtracting MaxStackSize from any StackAmount
	// Then if the item is Stackable, return the min of AmountToFill and RoomInSlot 
	const int32 RoomInSlot = MaxStackSize - GetStackAmount(GridSlot);
	return bStackable ? FMath::Min(AmountToFill, RoomInSlot) : 1; 
}

int32 UInv_InventoryGrid::GetStackAmount(const UInv_GridSlot* GridSlot) const
{
	int32 CurrentSlotStackCount = GridSlot->GetStackCount();
	// If we are at a slot that doesn't hold the stack count. We must get the actual stack count.
	if (const int32 UpperLeftIndex = GridSlot->GetUpperLeftIndex(); UpperLeftIndex != INDEX_NONE)
	{
		UInv_GridSlot* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
		CurrentSlotStackCount = UpperLeftGridSlot->GetStackCount();
	}
	return CurrentSlotStackCount;
}

void UInv_InventoryGrid::AddStacks(const FInv_SlotAvailabilityResult& Result)
{
	// Checks if SlotAvailabilityResult Item matches Category 
	if (!MatchesCategory(Result.Item.Get())) return;

	// Loop through all SlotAvailabilities within the SlotAvailabilityResult
	for (const auto& Availablility : Result.SlotAvailabilities)
	{
		// If there is an item at one of the SlotAvailabilities
		if (Availablility.bItemAtIndex)
		{
			// Update the SlottedItem StackCount and Update the GridSlot StackCount 
			const auto& GridSlot = GridSlots[Availablility.Index];
			const auto& SlottedItem = SlottedItems.FindChecked(Availablility.Index);
			SlottedItem->UpdateStackCount(GridSlot->GetStackCount() + Availablility.AmountToFill);
			GridSlot->SetStackCount(GridSlot->GetStackCount() + Availablility.AmountToFill);
		}
		else
		{
			// Create a new SlottedItem and then update the GridSlot 
			AddItemAtIndex(Result.Item.Get(), Availablility.Index, Result.bStackable, Availablility.AmountToFill);
			UpdateGridSlots(Result.Item.Get(), Availablility.Index, Result.bStackable, Availablility.AmountToFill);
		}
	}
}

void UInv_InventoryGrid::OnSlottedItemClicked(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	check(GridSlots.IsValidIndex(GridIndex));
	UInv_InventoryItem* ClickedInventoryItem = GridSlots[GridIndex]->GetInventoryItem().Get();

	if (!IsValid(HoverItem) && IsLeftClicked(MouseEvent))
	{
		// Pickup - Assign the hover item, and remove the slotted item from the grid
		PickUp(ClickedInventoryItem, GridIndex);
		return; 
	}

	if (IsRightClicked(MouseEvent))
	{
		CreateItemPopUp(GridIndex);
		return;
	}

	// Do the HoveredItem and the clicked InventoryItem share a type, and are they stackable?
	if (IsSameStackable(ClickedInventoryItem))
	{
		// Gathering variables for ShouldSwapStackCounts
		const int32 ClickedStackCount = GridSlots[GridIndex]->GetStackCount();
		const FInv_StackableFragment* StackableFragment = ClickedInventoryItem->GetItemManifest().GetFragmentOfType<FInv_StackableFragment>();
		const int32 MaxStackSize = StackableFragment->GetMaxStackSize();
		const int32 RoomInClickedSlot = MaxStackSize - ClickedStackCount;
		const int32 HoveredStackCount = HoverItem->GetStackCount();
		
		// Should we swap their stack counts? (Room in the clicked slot == 0 && HoveredStackCount < MaxStackSize)
		if (ShouldSwapStackCounts(RoomInClickedSlot, HoveredStackCount, MaxStackSize))
		{
			//Swap stack counts
			SwapStackCounts(ClickedStackCount, HoveredStackCount, GridIndex);
			return; 
		}
		
		// Should we consume the HoverItem's stacks? (Room in the clicked slot >= HoveredStackCount)
		if (ShouldConsumeHoverItemStacks(HoveredStackCount, RoomInClickedSlot))
		{
			ConsumeHoverItemStacks(ClickedStackCount, HoveredStackCount, GridIndex);
			return; 
		}
		
		// Should we fill in the stacks of the clicked item? (And not consume the hover item)
		if (ShouldFillInStack(RoomInClickedSlot, HoveredStackCount))
		{
			FillInStack(RoomInClickedSlot, HoveredStackCount - RoomInClickedSlot, GridIndex);
			return; 
		}
		
		// Clicked slot is already full - do nothing (maybe play a sound?)
		if (RoomInClickedSlot == 0)
		{
			return;	
		}
	}

	// Swap with the hover item
	SwapWithHoverItem(ClickedInventoryItem, GridIndex);
}

void UInv_InventoryGrid::CreateItemPopUp(const int32 GridIndex)
{
	UInv_InventoryItem* RightClickedItem = GridSlots[GridIndex]->GetInventoryItem().Get();
	if (!IsValid(RightClickedItem)) return;

	ItemPopUp = CreateWidget<UInv_ItemPopUp>(this, ItemPopUpClass);

	OwningCanvasPanel->AddChild(ItemPopUp);
	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ItemPopUp);
	const FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());
	CanvasSlot->SetPosition(MousePosition);
	CanvasSlot->SetSize(ItemPopUp->GetBoxSize()); 
}

FIntPoint UInv_InventoryGrid::GetItemDimensions(const FInv_ItemManifest& Manifest) const
{
	// Returns Item dimensions based on GridFragment 
	const FInv_GridFragment* GridFragment = Manifest.GetFragmentOfType<FInv_GridFragment>();
	return GridFragment ? GridFragment->GetGridSize() : FIntPoint(1,1); 
}

void UInv_InventoryGrid::ConstructGrid()
{
	// Assigns array size to prevent it from dynamically adjusting 
	GridSlots.Reserve(Rows * Columns);

	// Loops through all rows and all columns 
	for (int32 j = 0; j < Rows; ++j)
	{
		for (int32 i = 0; i < Columns; ++i)
		{
			// Creates a widget for each loop passthrough which is then added to canvas 
			UInv_GridSlot* GridSlot = CreateWidget<UInv_GridSlot>(this, GridSlotClass);
			CanvasPanel->AddChild(GridSlot);
			
			// Helper function is called which creates index from row and column point
			// Which is then passed through GridSlot function to set its index
			const FIntPoint TilePosition(i, j); 
			GridSlot->SetTileIndex(UInv_WidgetUtils::GetIndexFromPosition(TilePosition, Columns));

			// Sets the slot for the individual grid slot. then sets size and position of slot 
			UCanvasPanelSlot* GridCanvasPanelSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridSlot);
			GridCanvasPanelSlot->SetSize(FVector2D(TileSize));
			GridCanvasPanelSlot->SetPosition(FVector2D(TilePosition * TileSize));

			// Add each created GridSlot widget to the GridSlot array 
			GridSlots.Add(GridSlot);

			GridSlot->GridSlotClicked.AddDynamic(this, &ThisClass::OnGridSlotClicked);
			GridSlot->GridSlotHovered.AddDynamic(this, &ThisClass::OnGridSlotHovered);
			GridSlot->GridSlotUnhovered.AddDynamic(this, &ThisClass::OnGridSlotUnhovered);
		}
	}
}


void UInv_InventoryGrid::OnGridSlotClicked(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	// Do we have a valid HoverItem and ItemDropIndex 
	if (!IsValid(HoverItem)) return;
	if (!GridSlots.IsValidIndex(ItemDropIndex)) return;

	// Is there a valid item at the location already? 
	if (CurrentSpaceQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentSpaceQueryResult.UpperLeftIndex))
	{
		OnSlottedItemClicked(CurrentSpaceQueryResult.UpperLeftIndex, MouseEvent);
		return;
	}

	// There's no item at index. Place item down 
	auto GridSlot = GridSlots[ItemDropIndex];
	if (!GridSlot->GetInventoryItem().IsValid())
	{
		PutDownOnIndex(ItemDropIndex);
	}
}

void UInv_InventoryGrid::PutDownOnIndex(const int32 Index)
{
	// When we place an item it is added to the index, then updates the GridSlot, then destroys the HoverItem
	AddItemAtIndex(HoverItem->GetInventoryItem(), Index, HoverItem->IsStackable(), HoverItem->GetStackCount());
	UpdateGridSlots(HoverItem->GetInventoryItem(), Index, HoverItem->IsStackable(), HoverItem->GetStackCount());
	ClearHoverItem();
}

void UInv_InventoryGrid::ClearHoverItem()
{
	if (!IsValid(HoverItem)) return;

	// Nuke HoverItem 
	HoverItem->SetInventoryItem(nullptr);
	HoverItem->SetIsStackable(false);
	HoverItem->SetPreviousGridIndex(INDEX_NONE);
	HoverItem->UpdateStackCount(0);
	HoverItem->SetImageBrush(FSlateNoResource());

	HoverItem->RemoveFromParent();
	HoverItem = nullptr;

	// Show mouse cursor
	ShowCursor();
}

bool UInv_InventoryGrid::IsSameStackable(const UInv_InventoryItem* ClickedInventoryItem) const
{
	// Checks if ClickedInventoryItem and HoverItem are the same 
	const bool bIsSameItem = ClickedInventoryItem == HoverItem->GetInventoryItem();
	const bool bIsStackable = ClickedInventoryItem->IsStackable();
	return bIsSameItem && bIsStackable && HoverItem->GetItemType().MatchesTagExact(ClickedInventoryItem->GetItemManifest().GetItemType());
}

void UInv_InventoryGrid::SwapWithHoverItem(UInv_InventoryItem* ClickedInventoryItem, const int32 GridIndex)
{
	if (!IsValid(HoverItem)) return;

	// Creates a temporary variable for swapping 
	UInv_InventoryItem* TempInventoryItem = HoverItem->GetInventoryItem();
	const int32 TempStackCount = HoverItem->GetStackCount();
	const bool bTempIsStackable = HoverItem->IsStackable();

	// Keep the same previous GridIndex
	AssignHoverItem(ClickedInventoryItem, GridIndex, HoverItem->GetPreviousGridIndex());
	RemoveItemFromGrid(ClickedInventoryItem, GridIndex);
	AddItemAtIndex(TempInventoryItem, ItemDropIndex, bTempIsStackable, TempStackCount);
	UpdateGridSlots(TempInventoryItem, ItemDropIndex, bTempIsStackable, TempStackCount);
}

bool UInv_InventoryGrid::ShouldSwapStackCounts(const int32 RoomInClickedSlot, const int32 HoveredStackCount,
	const int32 MaxStackSize) const
{
	return RoomInClickedSlot == 0 && HoveredStackCount < MaxStackSize;
}

void UInv_InventoryGrid::SwapStackCounts(const int32 ClickedStackCount, const int32 HoveredStackCount,
	const int32 Index)
{
	UInv_GridSlot* GridSlot = GridSlots[Index];
	GridSlot->SetStackCount(HoveredStackCount);

	// Check SlottedItems array for Index, UpdateStackCount by HoveredStack Count 
	UInv_SlottedItem* ClickedSlottedItem = SlottedItems.FindChecked(Index);
	ClickedSlottedItem->UpdateStackCount(HoveredStackCount);

	HoverItem->UpdateStackCount(ClickedStackCount);
}

bool UInv_InventoryGrid::ShouldConsumeHoverItemStacks(const int32 HoveredStackCount,
	const int32 RoomInClickedSlot) const
{
	return RoomInClickedSlot >= HoveredStackCount;
}

void UInv_InventoryGrid::ConsumeHoverItemStacks(const int32 ClickedStackCount, const int32 HoveredStackCount,
	const int32 Index)
{
	// Stores HoveredStackCount and then adds it to the ClickedItemStackCount 
	const int32 AmountToTransfer = HoveredStackCount;
	const int32 NewClickedStackCount = ClickedStackCount + AmountToTransfer;
	
	// Update the GridSlot at Index, and SlottedItem Array at Index the NewStackCount
	GridSlots[Index]->SetStackCount(NewClickedStackCount);
	SlottedItems.FindChecked(Index)->UpdateStackCount(NewClickedStackCount);

	// Remove HoverItem and show mouse cursor 
	ClearHoverItem();
	ShowCursor();

	// Update the background of the slots 
	const FInv_GridFragment* GridFragment = GridSlots[Index]->GetInventoryItem()->GetItemManifest().GetFragmentOfType<FInv_GridFragment>();
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1,1);
	HighlightSlots(Index, Dimensions);
}

bool UInv_InventoryGrid::ShouldFillInStack(const int32 RoomInClickedSlot, const int32 HoveredStackCount) const
{
	return RoomInClickedSlot < HoveredStackCount;
}

void UInv_InventoryGrid::FillInStack(const int32 FillAmount, const int32 Remainder, const int32 Index)
{
	// Update GridSlot 
	UInv_GridSlot* GridSlot = GridSlots[Index];
	const int32 NewStackCount = GridSlot->GetStackCount() + FillAmount;

	GridSlot->SetStackCount(NewStackCount);

	// Update SlottedItems 
	UInv_SlottedItem* ClickedSlottedItem = SlottedItems.FindChecked(Index);
	ClickedSlottedItem->UpdateStackCount(NewStackCount);

	HoverItem->UpdateStackCount(Remainder);
}

UUserWidget* UInv_InventoryGrid::GetVisibleCursorWidget()
{
	if (!IsValid(GetOwningPlayer())) return nullptr;
	if (!IsValid(VisibleCursorWidget))
	{
		VisibleCursorWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), VisibleCursorWidgetClass);
	}
	return VisibleCursorWidget;
}

UUserWidget* UInv_InventoryGrid::GetHiddenCursorWidget()
{
	if (!IsValid(GetOwningPlayer())) return nullptr;
	if (!IsValid(HiddenCursorWidget))
	{
		HiddenCursorWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), HiddenCursorWidgetClass);
	}
	return HiddenCursorWidget;
}

void UInv_InventoryGrid::ShowCursor()
{
	if (!IsValid(GetOwningPlayer())) return;
	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, GetVisibleCursorWidget());
}

void UInv_InventoryGrid::HideCursor()
{
	if (!IsValid(GetOwningPlayer())) return;
	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, GetHiddenCursorWidget());
}

void UInv_InventoryGrid::SetOwningCanvas(UCanvasPanel* OwningCanvas)
{
	OwningCanvasPanel = OwningCanvas;
}

void UInv_InventoryGrid::OnGridSlotHovered(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	// Checks for valid hover item, if there is it will highlight the GridSlot 
	if (IsValid(HoverItem)) return;

	UInv_GridSlot* GridSlot = GridSlots[GridIndex];
	if (GridSlot->IsAvailable())
	{
		GridSlot->SetOccupiedTexture();
	}
}

void UInv_InventoryGrid::OnGridSlotUnhovered(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	// Checks for valid hover item, if there is it will unhighlight the GridSlot when mouse is gone 
	if (IsValid(HoverItem)) return;

	UInv_GridSlot* GridSlot = GridSlots[GridIndex];
	if (GridSlot->IsAvailable())
	{
		GridSlot->SetUnoccupiedTexture();
	}
}

bool UInv_InventoryGrid::MatchesCategory(const UInv_InventoryItem* Item) const
{
	// Checks ItemManifest to item category 
	return Item->GetItemManifest().GetItemCategory() == ItemCategory; 
}
