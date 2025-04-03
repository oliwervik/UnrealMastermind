// Copyright 2025 © Froströk. All Rights Reserved.

#pragma once

struct FBlueprintDocumentationSettings
{
	// General settings
	bool bIncludeBasicInfo = true;        // Blueprint name, parent class
	bool bIncludeVariables = true;        // Variables and their types

	// Component detail levels
	enum class EComponentDetailLevel
	{
		Minimal,    // Just component name and class
		Basic,      // Include important properties (location, rotation, etc.)
		Full        // Include all non-default properties
	};
	EComponentDetailLevel ComponentDetailLevel = EComponentDetailLevel::Basic;

	// Execution flow settings
	bool bTraceExecutionFlow = true;      // Show node connections
	int32 MaxExecutionFlowDepth = 5;      // How deep to follow execution chains
    
	// Variable usage tracking
	bool bTrackVariableUsage = true;      // Find where variables are used
    
	// Comments
	bool bIncludeComments = true;         // Include comment nodes

	// Amount of characters in the preview of documentation in blueprint details
	int32 DocumentationPreviewChars = 400;
    
	// Filter settings
	TArray<FString> IgnoredPropertyPrefixes = { "b", "Base", "Cached", "Default" };  // Skip properties starting with these
};
