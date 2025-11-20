// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Utils/Inv_WidgetUtils.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/Widget.h"

FVector2D UInv_WidgetUtils::GetWidgetPosition(UWidget* Widget)
{
	// Widgets have cached geometry we can use to retrieve their position
	const FGeometry Geometry = Widget->GetCachedGeometry();

	// Out variables for below function 
	FVector2D PixelPosition;
	FVector2D ViewportPosition;
	
	// Get position of widget in viewport. Pass in the widget we are checking, the location of it, and the coordinate we are looking for 
	USlateBlueprintLibrary::LocalToViewport(Widget, Geometry, USlateBlueprintLibrary::GetLocalTopLeft(Geometry), PixelPosition, ViewportPosition);

	return ViewportPosition;
}

FVector2D UInv_WidgetUtils::GetWidgetSize(UWidget* Widget)
{
	const FGeometry Geometry = Widget->GetCachedGeometry();
	return Geometry.GetLocalSize(); 
}

bool UInv_WidgetUtils::IsWithinBounds(const FVector2D& CanvasPosition, const FVector2D& WidgetSize,
                                      const FVector2D& MousePosition)
{
	return MousePosition.X >= CanvasPosition.X && MousePosition.X <= (CanvasPosition.X + WidgetSize.X) &&
		MousePosition.Y >= CanvasPosition.Y && MousePosition.Y <= (CanvasPosition.Y + WidgetSize.Y);
}

FVector2D UInv_WidgetUtils::GetClampedWidgetPosition(const FVector2D& Boundary, const FVector2D& WidgetSize,
	const FVector2D& MousePosition)
{
	FVector2D ClampedPosition = MousePosition;

	// Adjust horizontal position to ensure that the widget stays within the boundary
	if (MousePosition.X + WidgetSize.X > Boundary.X) // Widget exceeds right edge
	{
		ClampedPosition.X = Boundary.X - WidgetSize.X;
	}
	if (MousePosition.X < 0.f) // Widget exceeds the left edge
	{
		ClampedPosition.X = 0.f; 
	}

	// Adjust vertical position to ensure that the widget stays within the boundary
	if (MousePosition.Y + WidgetSize.Y > Boundary.Y) // Widget exceeds bottom edge  
	{
		ClampedPosition.Y = Boundary.Y - WidgetSize.Y;
	}
	if (MousePosition.Y < 0.f) // Widget exceeds the top edge 
	{
		ClampedPosition.Y = 0.f;
	}
	
	return ClampedPosition;
}

int32 UInv_WidgetUtils::GetIndexFromPosition(const FIntPoint& Position, const int32 Columns)
{
	return Position.X + Position.Y * Columns;
}

FIntPoint UInv_WidgetUtils::GetPositionFromIndex(const int32 Index, const int32 Columns)
{
	return FIntPoint(Index % Columns, Index / Columns);
}
