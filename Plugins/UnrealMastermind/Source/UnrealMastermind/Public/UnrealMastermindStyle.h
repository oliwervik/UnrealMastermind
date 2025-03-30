// Copyright © Froströk. All Rights Reserved.
// This plugin is governed by the Unreal Engine Marketplace EULA.
// This software cannot be redistributed, modified, or resold outside of the original purchase.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class FUnrealMastermindStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static void ReloadTextures();
	static const ISlateStyle& Get();
	static FName GetStyleSetName();

private:
	static TSharedRef<FSlateStyleSet> Create();
	static TSharedPtr<FSlateStyleSet> StyleInstance;
};