// Copyright © Froströk. All Rights Reserved.
// This plugin is governed by the Unreal Engine Marketplace EULA.
// This software cannot be redistributed, modified, or resold outside of the original purchase.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "UnrealMastermindStyle.h"

class FUnrealMastermindCommands : public TCommands<FUnrealMastermindCommands>
{
public:
	FUnrealMastermindCommands()
		: TCommands<FUnrealMastermindCommands>(
			TEXT("UnrealMastermind"),
			NSLOCTEXT("Contexts", "UnrealMastermind", "Unreal Mastermind Plugin"),
			NAME_None,
			FUnrealMastermindStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenPluginWindow;
};
