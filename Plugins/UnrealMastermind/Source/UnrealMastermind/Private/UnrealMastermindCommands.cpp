// Copyright Froströk. All Rights Reserved.

#include "UnrealMastermindCommands.h"

#define LOCTEXT_NAMESPACE "FUnrealMastermindModule"

void FUnrealMastermindCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Unreal Mastermind", "Open the Unreal Mastermind window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE