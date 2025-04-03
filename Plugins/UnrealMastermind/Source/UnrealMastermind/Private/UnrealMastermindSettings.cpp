// Copyright 2025 © Froströk. All Rights Reserved.

#include "UnrealMastermindSettings.h"

UUnrealMastermindSettings::UUnrealMastermindSettings()
{
	// Default provider
	SelectedProvider = ELLMProvider::OpenAI;

	// Default model settings
	OpenAIModel = TEXT("gpt-4");
	OpenAIEndpoint = TEXT("https://api.openai.com/v1/chat/completions");
	AnthropicModel = TEXT("claude-3-5-haiku-latest");
	AnthropicApiEndpoint = TEXT("https://api.anthropic.com/v1/messages");

	// Default documentation settings
	bIncludeVariableDescriptions = true;
	bIncludeFunctionBreakdowns = true;
	bIncludeOverallSummary = true;
	IgnoredPropertyPrefixes = {"Example"};
	bIncludeComments = true;
	MaxExecutionFlowDepth = 5;
	bTrackVariableUsage = true;
	ComponentDetailLevel = 1;
	MaxTokens = 4000;
	Temperature = 0.5f;

	//Default display settings
	DocumentationPreviewChars = 500;

	//Prompt settings
	SystemPrompt = TEXT(
		"You are a professional technical documentation writer specializing in Unreal Engine Blueprints. "
		"Your task is to provide comprehensive, clear documentation that explains: "
		"1. The purpose of the Blueprint (what it does) "
		"2. How the Blueprint works (key mechanisms and flow) "
		"3. Important variables and their purposes "
		"4. Main function and event behaviors "
		"5. Component setup and configuration "
		"6. Recommendations for usage or potential improvements "
		"Format the documentation in a clear, structured way with headings, bullet points, and code-like notation for Blueprint nodes. Dont specifically document individual node behaviours such as ForEach Loops, Then etc. Instead focus on summarizing the behaviour, intention and functionality of the blueprint.");
}

FName UUnrealMastermindSettings::GetCategoryName() const
{
	return FName("Plugins");
}

FText UUnrealMastermindSettings::GetSectionText() const
{
	return FText::FromString("Unreal Mastermind");
}
