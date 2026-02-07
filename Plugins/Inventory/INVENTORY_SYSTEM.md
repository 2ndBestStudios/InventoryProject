# Inventory System Plugin Documentation

## Project Overview

The Inventory plugin is a self-contained Unreal Engine 5 plugin that provides a complete, multiplayer-ready inventory and equipment management system. It implements a **spatial grid-based inventory** where items occupy one or more grid cells, a **fragment-based item composition system** for flexible item definitions, a **full equipment pipeline** with visual actor attachment, and a **composite widget pattern** for dynamic UI assembly.

The plugin is designed as a drop-in module: projects add it to their `Plugins/` folder and reference the `Inventory` module with no engine modification required.

---

## Architecture Summary

```
┌─────────────────────────────────────────────────────────────┐
│                     Player Controller                       │
│  (Input handling, line-trace item detection, HUD creation)  │
└──────────┬──────────────────────────────────┬───────────────┘
           │                                  │
           ▼                                  ▼
┌─────────────────────┐           ┌──────────────────────────┐
│ Inventory Component │◄─────────►│   Equipment Component    │
│ (Item storage, RPCs,│  equip/   │ (Spawn actors on sockets,│
│  fast-array replic.) │  unequip  │  proxy mesh for preview) │
└──────────┬──────────┘  events   └──────────────────────────┘
           │
           ▼
┌─────────────────────────────────────────────────────────────┐
│                    Spatial Inventory UI                      │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐         │
│  │ Equippables │  │ Consumables │  │ Craftables  │  (tabs)  │
│  │    Grid     │  │    Grid     │  │    Grid     │         │
│  └─────────────┘  └─────────────┘  └─────────────┘         │
│  ┌─────────────────────────┐  ┌────────────────────┐       │
│  │   Equipped Grid Slots   │  │ Character Display  │       │
│  └─────────────────────────┘  └────────────────────┘       │
└─────────────────────────────────────────────────────────────┘
```

---

## Core Systems

### 1. Fragment-Based Item System

Items are not defined through deep class hierarchies. Instead, every item is a composition of **fragments** — small, self-contained data blocks stored in an `FInv_ItemManifest`. Each fragment carries both data and behavior.

**Why fragments?** A sword needs grid-size info, an icon, a name, equipment modifiers, and flavor text. A potion needs grid-size, icon, name, stack count, and consume effects. Rather than building parallel inheritance trees, both items share the same `UInv_InventoryItem` class and differ only in which fragments their manifest contains.

#### Fragment Hierarchy

| Fragment | Base | Purpose |
|----------|------|---------|
| `FInv_ItemFragment` | — | Abstract root. Holds a `FGameplayTag` identifier. |
| `FInv_InventoryItemFragment` | ItemFragment | Adds `Assimilate()` — pushes data into UI widgets. |
| `FInv_GridFragment` | ItemFragment | Grid dimensions (`FIntPoint`) and padding. |
| `FInv_StackableFragment` | ItemFragment | Max stack size and current count. |
| `FInv_ImageFragment` | InventoryItemFragment | `UTexture2D` icon and display dimensions. |
| `FInv_TextFragment` | InventoryItemFragment | Single `FText` value (name, flavor text, etc.). |
| `FInv_LabeledNumberFragment` | InventoryItemFragment | Label + numeric value with optional randomization on manifest. |
| `FInv_ConsumableFragment` | InventoryItemFragment | Contains an array of `FInv_ConsumeModifier` sub-fragments. |
| `FInv_ConsumeModifier` | LabeledNumberFragment | Base for consumable effects (e.g., health/mana restore). |
| `FInv_HealthPotionFragment` | ConsumeModifier | Restores health on consume. |
| `FInv_ManaPotionFragment` | ConsumeModifier | Restores mana on consume. |
| `FInv_EquipmentFragment` | InventoryItemFragment | Equipment type, socket name, equip actor class, array of `FInv_EquipModifier`. |
| `FInv_EquipModifier` | LabeledNumberFragment | Base for stat modifiers applied on equip/unequip. |
| `FInv_StrengthModifier` | EquipModifier | Adjusts strength stat. |

Fragments are stored as `TArray<TInstancedStruct<FInv_ItemFragment>>` inside the manifest, enabling polymorphic serialization and replication without `UObject` overhead.

#### Fragment Tags

Every fragment is identified by a `FGameplayTag` from the `FragmentTags` hierarchy:

```
FragmentTags.GridFragment
FragmentTags.IconFragment
FragmentTags.StackableFragment
FragmentTags.ConsumableFragment
FragmentTags.EquipmentFragment
FragmentTags.ItemNameFragment
FragmentTags.PrimaryStatFragment
FragmentTags.ItemTypeFragment
FragmentTags.FlavorTextFragment
FragmentTags.SellValueFragment
FragmentTags.RequiredLevelFragment
FragmentTags.StatMod_1 / StatMod_2 / StatMod_3
```

#### Item Tags

Items themselves are identified by gameplay tags under the `GameItems` hierarchy:

```
GameItems.Equipment.Weapons.Axe / Sword
GameItems.Equipment.Cloaks.RedCloak
GameItems.Equipment.Masks.SteelMask
GameItems.Consumables.Potions.Red.Small / Large
GameItems.Consumables.Potions.Blue.Small / Large
GameItems.Craftables.FireFernFruit / LuminDaisy / ScorchPedalBlossom
```

#### Item Manifest (`FInv_ItemManifest`)

The manifest is the data definition for an item. It holds:

- `TArray<TInstancedStruct<FInv_ItemFragment>> Fragments` — all fragments
- `EInv_ItemCategory ItemCategory` — Equippable, Consumable, Craftable, or None
- `FGameplayTag ItemType` — unique gameplay tag identifier
- `TSubclassOf<AActor> PickupActorClass` — actor to spawn when dropped

Key operations:
- `Manifest(UObject* Outer)` — creates a `UInv_InventoryItem` from the manifest, calling each fragment's `Manifest()`.
- `GetFragmentOfType<T>()` / `GetFragmentOfTypeWithTag<T>(Tag)` — template-based fragment queries.
- `AssimilateInventoryFragments(UInv_CompositeBase*)` — pushes all fragment data into a widget tree.
- `SpawnPickUpActor()` — spawns the world representation when an item is dropped.

#### Inventory Item (`UInv_InventoryItem`)

The runtime object stored in the inventory. It wraps a manifest and adds:

- Network replication via `GetLifetimeReplicatedProps()` and `IsSupportedForNetworking()`
- `TotalStackCount` tracking (replicated)
- `IsStackable()` / `IsConsumable()` convenience checks
- `GetFragment<T>(Item, Tag)` static template accessor

---

### 2. Inventory Management

#### Inventory Component (`UInv_InventoryComponent`)

An `UActorComponent` attached to the player controller. It is the central authority for adding, removing, dropping, consuming, and equipping items.

**Delegates broadcast:**
| Delegate | Parameters | Fires when |
|----------|-----------|------------|
| `OnItemAdded` | `UInv_InventoryItem*` | New item enters inventory |
| `OnItemRemoved` | `UInv_InventoryItem*` | Item leaves inventory |
| `NoRoomInventory` | — | Pickup attempted with no space |
| `OnStackChange` | `FInv_SlotAvailabilityResult&` | Stack counts change on existing items |
| `OnItemEquipped` | `UInv_InventoryItem*` | Item is equipped |
| `OnItemUnEquipped` | `UInv_InventoryItem*` | Item is unequipped |
| `OnInventoryMenuToggled` | `bool bOpen` | Menu opens/closes |

**Server RPCs:**
| RPC | Purpose |
|-----|---------|
| `Server_AddNewItem` | Add a brand new item (authority creates it) |
| `Server_AddStacksToItem` | Add stacks to an existing item |
| `Server_DropItem` | Remove item and spawn pickup actor |
| `Server_ConsumeItem` | Use a consumable (removes one stack) |
| `Server_EquipSlotClicked` | Equip/swap equipment in a slot |
| `MultiCast_EquipSlotClicked` | Sync equip visuals to all clients |

#### Fast Array Replication (`FInv_InventoryFastArray`)

The inventory stores items in an `FFastArraySerializer`-derived container. This provides **delta replication** — only changed entries are sent over the network, not the entire list.

- `FInv_InventoryEntry` wraps a `UInv_InventoryItem*` and inherits `FFastArraySerializerItem`
- `PreReplicatedRemove` / `PostReplicatedAdd` callbacks fire delegates for UI updates
- `NetDeltaSerialize` handles the wire format

#### Utility Library (`UInv_InventoryStatics`)

Blueprint-callable static helpers:
- `GetInventoryComponent(PlayerController)` — find the inventory component
- `ItemHovered` / `ItemUnhovered` — trigger item hover tooltips
- `GetHoverItem` / `GetInventoryWidget` — accessor shortcuts
- `ForEach2D<T>()` — iterate a 2D sub-region of a flat array (used by grid logic)

---

### 3. Equipment Management

#### Equipment Component (`UInv_EquipmentComponent`)

An `UActorComponent` that listens to inventory equip/unequip delegates and manages the visual equipment actors.

Key responsibilities:
- Bind to `OnItemEquipped` / `OnItemUnEquipped` from the inventory component
- Spawn `AInv_EquipActor` subclasses and attach them to a skeletal mesh socket
- Track equipped actors in `TArray<AInv_EquipActor*>`
- Find/remove actors by `FGameplayTag EquipmentType`
- Handle pawn changes via `OnPossessedPawnChange`

#### Equip Actor (`AInv_EquipActor`)

A replicated `AActor` that represents an equipped item visually in the world (a sword mesh, a cloak particle effect, etc.). Tagged with an `FGameplayTag EquipmentType` for identification.

#### Proxy Mesh (`AInv_ProxyMesh`)

An actor spawned in the inventory UI scene to display the player's character model. It has its own `UInv_EquipmentComponent` so equipment can be previewed on the proxy mesh without affecting the real character until confirmed.

---

### 4. Player Controller (`AInv_PlayerController`)

Extends `APlayerController` with:

- **Enhanced Input** setup — binds `PrimaryInteractAction` and `ToggleInventoryAction` via `UInputMappingContext`
- **Per-frame line trace** (`TraceForItem`) from screen center to detect `UInv_ItemComponent` actors
- **Highlight system** — actors implementing `IInv_Highlightable` get highlighted when aimed at
- **HUD widget** creation — spawns `UInv_HUDWidget` for pickup messages and alerts
- **Inventory toggle** — opens/closes the spatial inventory menu

---

### 5. Interaction System

#### Highlightable Interface (`IInv_Highlightable`)

A `UINTERFACE` with two methods:
- `Highlight()` — called when the player's trace hits the actor
- `UnHighlight()` — called when the trace leaves the actor

#### Highlightable Static Mesh (`UInv_HighlightableStaticMesh`)

A `UStaticMeshComponent` that implements `IInv_Highlightable` by swapping to a highlight material when aimed at.

#### Item Component (`UInv_ItemComponent`)

Attached to world pickup actors. Holds the `FInv_ItemManifest` and a `PickupMessage` string. When picked up:
1. Fires `OnPickedUp` (BlueprintImplementableEvent)
2. Destroys the owning actor

---

### 6. UI Widget System

#### Composite Pattern

The UI uses a **Composite design pattern** for building dynamic item descriptions:

```
UInv_CompositeBase (abstract)
├── UInv_Composite (container — has children)
│   └── UInv_ItemDescription (item detail panel)
└── UInv_Leaf (terminal node)
    ├── UInv_Leaf_Image (icon display)
    ├── UInv_Leaf_Text (text display)
    └── UInv_Leaf_LabeledValue (label + number pair)
```

Each widget node has a `FGameplayTag FragmentTag`. When `AssimilateInventoryFragments()` is called on an item manifest, each fragment walks the widget tree via `ApplyFunction()` and populates nodes whose tag matches its own.

This means the item description UI assembles itself dynamically from whatever fragments exist on the item — no hardcoded layout per item type.

#### Spatial Inventory (`UInv_SpatialInventory`)

The main inventory menu widget. Contains:

- **Three inventory grids** (`UInv_InventoryGrid`) — one per category (Equippables, Consumables, Craftables), switched via tab buttons through a `UWidgetSwitcher`
- **Equipped grid slots** (`UInv_EquippedGridSlot`) — dedicated slots for each equipment type (weapon, cloak, mask, etc.)
- **Character display** (`UInv_CharacterDisplay`) — shows the proxy mesh for equipment preview, supports click-drag rotation
- **Item description panel** (`UInv_ItemDescription`) — appears on hover after a configurable delay

#### Inventory Grid (`UInv_InventoryGrid`)

The core grid widget handling all spatial item placement logic:

- **Grid construction** — creates `Rows × Columns` grid slots on a `UCanvasPanel`
- **Multi-cell items** — items can span multiple cells defined by their `FInv_GridFragment` dimensions
- **Quadrant-based placement** — hover position is refined to quadrants (TopLeft, TopRight, BottomLeft, BottomRight) within a tile for precise placement of multi-cell items
- **Space queries** — `HasRoomAtIndex()` recursively checks slot constraints for multi-cell placement, handling stacking, type matching, and bounds validation
- **Drag and drop** — picking up an item creates an `UInv_HoverItem` that follows the cursor; dropping places it at the calculated index
- **Stacking** — stackable items of the same type in the same slot combine their counts; the grid handles partial fills, stack swaps, and stack splitting
- **Slot highlighting** — slots change visual state (Unoccupied, Occupied, Selected, GrayedOut) during hover to indicate valid/invalid placement
- **Context menu** (`UInv_ItemPopUp`) — right-click opens a popup with Split, Drop, and Consume actions

#### Grid Slot (`UInv_GridSlot`)

A single cell in the grid. Tracks:
- Tile index, upper-left index (for multi-cell items), stack count
- Availability state
- Visual state via `FSlateBrush` for each `EInv_GridSlotState`
- Mouse event delegates (clicked, hovered, unhovered)

#### Equipped Grid Slot (`UInv_EquippedGridSlot`)

Extends `UInv_GridSlot` for equipment-specific behavior:
- Filtered by `FGameplayTag EquipmentTypeTag`
- Creates/manages `UInv_EquippedSlottedItem` when an item is equipped
- Grayed-out icon shown when slot is empty

#### Slotted Item (`UInv_SlottedItem`)

The visual widget placed on the grid canvas to represent an item. Shows the item icon and stack count. Supports click events for pickup.

#### Hover Item (`UInv_HoverItem`)

A widget that follows the mouse cursor when dragging an item. Stores the item reference, previous grid index (for returning the item), grid dimensions, and stack count.

#### HUD Widget (`UInv_HUDWidget`)

The main heads-up display containing:
- `UInv_InfoMessage` — timed message display for notifications (e.g., "Inventory full")
- `ShowPickupMessage` / `HidePickupMessage` — BlueprintImplementableEvents for pickup prompts

---

## Data Flow

### Picking Up an Item

```
1. AInv_PlayerController::Tick()
   └─ TraceForItem() — line trace from screen center
      └─ Hit actor with UInv_ItemComponent?
         └─ Highlight actor, show pickup message on HUD

2. Player presses interact key
   └─ PrimaryInteract()
      └─ UInv_InventoryComponent::TryAddItem(ItemComponent)
         ├─ UInv_InventoryBase::HasRoomForItem() — check all grids
         │  └─ UInv_InventoryGrid::HasRoomForItem() — spatial check
         │     └─ Iterate slots, check dimensions, stacking rules
         ├─ If stackable and existing item found:
         │  └─ Server_AddStacksToItem() [Server RPC]
         │     └─ OnStackChange delegate → Grid updates stack counts
         └─ If new item:
            └─ Server_AddNewItem() [Server RPC]
               └─ FInv_InventoryFastArray::AddEntry()
                  └─ PostReplicatedAdd() → OnItemAdded delegate
                     └─ UInv_InventoryGrid::AddItem()
                        └─ CreateSlottedItem() → add to canvas
```

### Equipping an Item

```
1. Player picks up item from grid (creates HoverItem)
2. Player clicks an UInv_EquippedGridSlot
   └─ UInv_SpatialInventory::EquippedGridSlotClicked()
      ├─ Validate: hover item equipment type matches slot type
      ├─ If slot occupied: unequip existing item, return to grid
      └─ Server_EquipSlotClicked() [Server RPC]
         └─ MultiCast_EquipSlotClicked() [Multicast RPC]
            ├─ OnItemEquipped delegate
            │  └─ UInv_EquipmentComponent::OnItemEquipped()
            │     └─ FInv_EquipmentFragment::OnEquip()
            │        ├─ Call all EquipModifier::OnEquip() (stat changes)
            │        └─ SpawnAttachedActor() on skeletal mesh socket
            └─ OnItemUnEquipped delegate (if swapping)
               └─ FInv_EquipmentFragment::OnUnequip()
                  ├─ Call all EquipModifier::OnUnequip()
                  └─ DestroyAttachedActor()
```

### Consuming an Item

```
1. Player right-clicks a consumable in the grid
   └─ UInv_ItemPopUp appears with Consume button
2. Player clicks Consume
   └─ UInv_InventoryGrid::OnPopUpMenuConsume()
      └─ Server_ConsumeItem() [Server RPC]
         └─ FInv_ConsumableFragment::OnConsume()
            └─ Iterate ConsumeModifiers → each calls OnConsume(PlayerController)
               └─ e.g., FInv_HealthPotionFragment restores health
         └─ Decrement stack count or remove item
```

### Dropping an Item

```
1. Player right-clicks item → popup → Drop button
   └─ UInv_InventoryGrid::OnPopUpMenuDrop()
      └─ Server_DropItem() [Server RPC]
         ├─ FInv_ItemManifest::SpawnPickUpActor() — spawn in world
         │  └─ Random angle/distance from player (configurable)
         └─ FInv_InventoryFastArray::RemoveEntry()
            └─ PreReplicatedRemove() → OnItemRemoved delegate
               └─ Grid removes slotted item widget
```

---

## File Reference

### Plugin Root

| File | Purpose |
|------|---------|
| `Inventory.uplugin` | Plugin descriptor — metadata, module list, content flag |
| `Source/Inventory/Inventory.Build.cs` | Build configuration — module dependencies (Core, NetCore, StructUtils, GameplayTags, EnhancedInput, UMG, etc.) |
| `Source/Inventory/Public/Inventory.h` | Module interface (`FInventoryModule`) and `LogInventory` log category |
| `Source/Inventory/Private/Inventory.cpp` | Module startup/shutdown implementation |

### Item System

| File | Purpose |
|------|---------|
| `Public/Types/Inv_GridTypes.h` | Enums (`EInv_ItemCategory`, `EInv_TileQuadrant`) and structs (`FInv_SlotAvailability`, `FInv_TileParameters`, `FInv_SpaceQueryResult`) used throughout the grid system |
| `Public/Items/Inv_ItemTags.h` | Declares all `GameItems.*` gameplay tags for item identification |
| `Public/Items/Fragments/Inv_FragmentTags.h` | Declares all `FragmentTags.*` gameplay tags for fragment identification |
| `Public/Items/Fragments/Inv_ItemFragment.h` | All fragment struct definitions — the building blocks of every item |
| `Public/Items/Inv_InventoryItem.h` | `UInv_InventoryItem` — the replicated runtime object stored in the inventory |
| `Public/Items/Manifest/Inv_ItemManifest.h` | `FInv_ItemManifest` — item data definition, fragment container, and template query functions |
| `Public/Items/Components/Inv_ItemComponent.h` | `UInv_ItemComponent` — attached to world pickup actors, holds the manifest |

### Inventory Management

| File | Purpose |
|------|---------|
| `Public/InventoryManagement/FastArray/Inv_FastArray.h` | `FInv_InventoryFastArray` — delta-replicated item list using `FFastArraySerializer` |
| `Public/InventoryManagement/Components/Inv_InventoryComponent.h` | `UInv_InventoryComponent` — central inventory logic, server RPCs, delegate broadcasting |
| `Public/InventoryManagement/Utils/Inv_InventoryStatics.h` | `UInv_InventoryStatics` — static utility functions for common inventory operations |

### Equipment Management

| File | Purpose |
|------|---------|
| `Public/EquipmentManagement/Components/Inv_EquipmentComponent.h` | `UInv_EquipmentComponent` — manages equipped actors, listens to inventory events |
| `Public/EquipmentManagement/EquipActor/Inv_EquipActor.h` | `AInv_EquipActor` — replicated actor representing a visually equipped item |
| `Public/EquipmentManagement/ProxyMesh/Inv_ProxyMesh.h` | `AInv_ProxyMesh` — character preview mesh in the inventory UI |

### Player & Interaction

| File | Purpose |
|------|---------|
| `Public/Player/Inv_PlayerController.h` | `AInv_PlayerController` — Enhanced Input bindings, per-frame item trace, HUD creation |
| `Public/Interaction/Inv_Highlightable.h` | `IInv_Highlightable` — interface for actors that visually respond to the player's aim |
| `Public/Interaction/Inv_HighlightableStaticMesh.h` | `UInv_HighlightableStaticMesh` — static mesh component with material-swap highlighting |

### Widget System — Composite Pattern

| File | Purpose |
|------|---------|
| `Public/Widgets/Composite/Inv_CompositeBase.h` | `UInv_CompositeBase` — abstract base with `FragmentTag`, `Collapse()`, `ApplyFunction()` |
| `Public/Widgets/Composite/Inv_Composite.h` | `UInv_Composite` — container node, propagates operations to children |
| `Public/Widgets/Composite/Inv_Leaf.h` | `UInv_Leaf` — terminal node, applies operations to self only |
| `Public/Widgets/Composite/Inv_Leaf_Image.h` | `UInv_Leaf_Image` — displays a `UTexture2D` icon with configurable size |
| `Public/Widgets/Composite/Inv_Leaf_Text.h` | `UInv_Leaf_Text` — displays a single `FText` value |
| `Public/Widgets/Composite/Inv_Leaf_LabeledValue.h` | `UInv_Leaf_LabeledValue` — displays a label + numeric value pair |
| `Public/Widgets/Utils/Inv_WidgetUtils.h` | `UInv_WidgetUtils` — position/size helpers, index-to-position conversion |

### Widget System — Inventory UI

| File | Purpose |
|------|---------|
| `Public/Widgets/Inventory/InventoryBase/Inv_InventoryBase.h` | `UInv_InventoryBase` — abstract base for inventory menus |
| `Public/Widgets/Inventory/Spatial/Inv_SpatialInventory.h` | `UInv_SpatialInventory` — main inventory widget with category tabs, equipment slots, and character display |
| `Public/Widgets/Inventory/Spatial/Inv_InventoryGrid.h` | `UInv_InventoryGrid` — the core grid logic for spatial item placement, drag-and-drop, stacking, and context menus |
| `Public/Widgets/Inventory/GridSlots/Inv_GridSlot.h` | `UInv_GridSlot` — single grid cell with visual states and mouse events |
| `Public/Widgets/Inventory/GridSlots/Inv_EquippedGridSlot.h` | `UInv_EquippedGridSlot` — equipment-specific grid slot filtered by type tag |
| `Public/Widgets/Inventory/SlottedItems/Inv_SlottedItem.h` | `UInv_SlottedItem` — item icon widget placed on the grid canvas |
| `Public/Widgets/Inventory/SlottedItems/Inv_EquippedSlottedItem.h` | `UInv_EquippedSlottedItem` — equipped item visual in an equipment slot |
| `Public/Widgets/Inventory/HoverItem/Inv_HoverItem.h` | `UInv_HoverItem` — cursor-following widget during drag operations |
| `Public/Widgets/ItemPopUp/Inv_ItemPopUp.h` | `UInv_ItemPopUp` — right-click context menu (Split, Drop, Consume) |
| `Public/Widgets/ItemDescription/Inv_ItemDescription.h` | `UInv_ItemDescription` — composite-based item detail panel |
| `Public/Widgets/HUD/Inv_HUDWidget.h` | `UInv_HUDWidget` — main HUD with pickup messages |
| `Public/Widgets/HUD/Inv_InfoMessage.h` | `UInv_InfoMessage` — timed notification message display |
| `Public/Widgets/CharacterDisplay/Inv_CharacterDisplay.h` | `UInv_CharacterDisplay` — character mesh viewport with drag-to-rotate |

---

## Design Patterns

| Pattern | Where Used | Why |
|---------|-----------|-----|
| **Composition over Inheritance** | Fragment system | Items are assemblies of fragments, not rigid class hierarchies. Adding a new item type means combining existing fragments, not writing new classes. |
| **Composite Pattern** | Widget tree (`CompositeBase` → `Composite` / `Leaf`) | Item descriptions build themselves dynamically from available fragments. No hardcoded per-item-type layouts. |
| **Observer / Delegate** | All components broadcast changes | Loose coupling — the inventory component does not know about the UI or equipment component directly; it broadcasts events that listeners handle independently. |
| **Fast Array Serialization** | `FInv_InventoryFastArray` | Network-efficient delta replication. Only changed inventory entries are sent over the wire. |
| **Server Authority** | All mutation via Server RPCs | The server owns inventory state. Clients request changes; the server validates and applies them, then replicates results back. |
| **Template Queries** | `GetFragmentOfType<T>()` | Type-safe fragment retrieval without casting at call sites. Uses `std::derived_from` concepts for compile-time safety. |
| **Gameplay Tags** | Items, fragments, equipment types | Hierarchical string-like identifiers that avoid brittle enum comparisons and support partial matching. |

---

## Networking Model

- **Authority**: Server / Listen Server owns all inventory state
- **Replication**: `UInv_InventoryItem` and `UInv_InventoryComponent` use standard UE property replication; the item list uses `FFastArraySerializer` for delta efficiency
- **RPCs**: All mutations (add, remove, drop, consume, equip) go through `Server_` RPCs; visual sync uses `MultiCast_` RPCs
- **Subobject Registration**: `UInv_InventoryItem` instances are registered as replicated subobjects of their owning component via `AddRepSubObj()`

---

## Item Categories

| Category | Grid Tab | Features |
|----------|---------|----------|
| **Equippable** | Equippables | Can be equipped to slots, spawns visual actor, applies stat modifiers |
| **Consumable** | Consumables | Can be consumed for effects (health, mana), single-use per stack unit |
| **Craftable** | Craftables | Raw materials (crafting system not yet implemented in the plugin) |

---

## Module Dependencies

```
Inventory module
├── Core
├── CoreUObject
├── Engine
├── NetCore          (networking / replication)
├── StructUtils      (FInstancedStruct for polymorphic fragments)
├── GameplayTags     (FGameplayTag for item/fragment identification)
├── EnhancedInput    (UInputAction / UInputMappingContext)
├── Slate / SlateCore
├── UMG              (UUserWidget, UMG widget components)
└── InputCore
```
