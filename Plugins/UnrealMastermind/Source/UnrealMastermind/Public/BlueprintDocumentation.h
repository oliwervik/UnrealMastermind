// Copyright © Froströk. All Rights Reserved.
// This plugin is governed by the Unreal Engine Marketplace EULA.
// This software cannot be redistributed, modified, or resold outside of the original purchase.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "BlueprintDocumentation.generated.h"

UCLASS()
class UNREALMASTERMIND_API UBlueprintDocumentation : public UObject
{
	GENERATED_BODY()
	
public:
	// Save documentation to Blueprint metadata
	static bool SaveDocumentation(UBlueprint* Blueprint, const FString& Documentation);
	
	// Retrieve documentation from Blueprint metadata
	static FString GetDocumentation(UBlueprint* Blueprint);
	
	// Check if a Blueprint has documentation
	static bool HasDocumentation(UBlueprint* Blueprint);
	
	// Clear documentation from a Blueprint
	static bool ClearDocumentation(UBlueprint* Blueprint);
	
private:
	// Metadata key used to store documentation
	static const FName DocumentationMetadataKey;
};