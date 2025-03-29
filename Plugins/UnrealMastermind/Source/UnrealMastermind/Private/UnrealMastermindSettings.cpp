// Copyright Froströk. All Rights Reserved.

#include "UnrealMastermindSettings.h"

UUnrealMastermindSettings::UUnrealMastermindSettings()
{
	// Default provider
	SelectedProvider = ELLMProvider::OpenAI;
    
	// Default model settings
	OpenAIModel = TEXT("gpt-4");
	ClaudeModel = TEXT("claude-3-sonnet-20240229");
    
	// Default documentation settings
	bIncludeVariableDescriptions = true;
	bIncludeFunctionBreakdowns = true;
	bIncludeOverallSummary = true;
}

FName UUnrealMastermindSettings::GetCategoryName() const
{
	return FName("Plugins");
}

FText UUnrealMastermindSettings::GetSectionText() const
{
	return FText::FromString("Unreal Mastermind");
}