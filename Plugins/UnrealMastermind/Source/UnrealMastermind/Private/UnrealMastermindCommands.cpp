// Copyright © Froströk. All Rights Reserved.
// This plugin is governed by the Unreal Engine Marketplace EULA.
// This software cannot be redistributed, modified, or resold outside of the original purchase.

#include "UnrealMastermindCommands.h"

#define LOCTEXT_NAMESPACE "FUnrealMastermindModule"

void FUnrealMastermindCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Unreal Mastermind", "Open the Unreal Mastermind window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE