// Copyright 2025 © Froströk. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UnrealMastermindSettings.generated.h"

UENUM(BlueprintType)
enum class ELLMProvider : uint8
{
	OpenAI UMETA(DisplayName = "OpenAI"),
	Anthropic UMETA(DisplayName = "Anthropic"),
	Other UMETA(DisplayName = "Other")
};

UCLASS(Config=EditorPerProjectUserSettings, DefaultConfig, meta=(DisplayName="Unreal Mastermind"))
class UNREALMASTERMIND_API UUnrealMastermindSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UUnrealMastermindSettings();

	UPROPERTY(config, EditAnywhere, Category="Documentation Generation", meta=(ToolTip="Controls how much detail is included for components: 0=Minimal (names only), 1=Basic (key properties), 2=Full (all properties)"))
	int32 ComponentDetailLevel; // 0=Minimal, 1=Basic, 2=Full
    
	UPROPERTY(config, EditAnywhere, Category="Documentation Generation", meta=(ToolTip="If enabled, the documentation will include information about where variables are used in the Blueprint"))
	bool bTrackVariableUsage;
    
	UPROPERTY(config, EditAnywhere, Category="Documentation Generation", meta=(ToolTip="Limits how deep to follow execution chains in Blueprint graphs. Higher values provide more detail but can make documentation very long"))
	int32 MaxExecutionFlowDepth;
	
	UPROPERTY(config, EditAnywhere, Category="Documentation Generation", meta=(ToolTip="If enabled, comment boxes in the Blueprint will be included in the documentation"))
	bool bIncludeComments;

	UPROPERTY(Config, EditAnywhere, Category = "Documentation Generation", meta=(ToolTip="If enabled, the documentation will include detailed descriptions of each variable in the Blueprint"))
	bool bIncludeVariableDescriptions;

	UPROPERTY(Config, EditAnywhere, Category = "Documentation Generation", meta=(ToolTip="If enabled, the documentation will include detailed explanations of each function in the Blueprint"))
	bool bIncludeFunctionBreakdowns;

	UPROPERTY(Config, EditAnywhere, Category = "Documentation Generation", meta=(ToolTip="If enabled, the documentation will include a high-level summary of what the Blueprint does"))
	bool bIncludeOverallSummary;

	UPROPERTY(config, EditAnywhere, Category="Documentation Generation", meta=(ToolTip="Component/property names that start with these prefixes will be excluded from documentation"))
	TArray<FString> IgnoredPropertyPrefixes;

	// The currently selected LLM provider
	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration", meta=(ToolTip="Select which AI provider to use for generating documentation"))
	ELLMProvider SelectedProvider;

	// AI Prompt Settings
	UPROPERTY(config, EditAnywhere, Category="AI Settings", meta=(DisplayName="System Prompt", MultiLine=true, ToolTip="This is the initial prompt that defines the AI's role and behavior when generating documentation"))
	FString SystemPrompt;

	UPROPERTY(config, EditAnywhere, Category= "AI Settings", meta=(DisplayName="Temperature", ClampMin="0.0", ClampMax="1.0", ToolTip="Controls the randomness of the AI's output. Lower values are more deterministic, higher values are more creative"))
	float Temperature;

	UPROPERTY(config, EditAnywhere, Category= "AI Settings", meta=(DisplayName="Max Tokens", ClampMin="100", ClampMax="16000", ToolTip="The maximum length of the generated documentation (in tokens, roughly 4 characters per token)"))
	int32 MaxTokens;

	// OpenAI Configuration
	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration|OpenAI", meta = (EditCondition = "SelectedProvider == ELLMProvider::OpenAI", ToolTip="Your OpenAI API key. Required to use OpenAI's services"))
	FString OpenAIApiKey;

	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration|OpenAI", meta = (EditCondition = "SelectedProvider == ELLMProvider::OpenAI", ToolTip="The OpenAI model to use (e.g., gpt-4, gpt-3.5-turbo)"))
	FString OpenAIModel;

	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration|OpenAI", meta = (EditCondition = "SelectedProvider == ELLMProvider::OpenAI", ToolTip="The endpoint URL for OpenAI API calls. Usually leave as default unless using a proxy"))
	FString OpenAIEndpoint;

	// Anthropic Configuration
	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration|Anthropic", meta = (EditCondition = "SelectedProvider == ELLMProvider::Anthropic", ToolTip="Your Anthropic API key. Required to use Anthropic services"))
	FString AnthropicApiKey;
	
	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration|Anthropic", meta = (EditCondition = "SelectedProvider == ELLMProvider::Anthropic", ToolTip="The Anthropic model to use (e.g., claude-2, claude-instant-1)"))
	FString AnthropicModel;
	
	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration|Anthropic", meta = (EditCondition = "SelectedProvider == ELLMProvider::Anthropic", ToolTip="The endpoint URL for Anthropic API calls"))
	FString AnthropicApiEndpoint;

	// Other Provider Configuration
	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration|Other", meta = (EditCondition = "SelectedProvider == ELLMProvider::Other", ToolTip="Name of the alternative AI provider you're using"))
	FString OtherProviderName;

	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration|Other", meta = (EditCondition = "SelectedProvider == ELLMProvider::Other", ToolTip="API key for the alternative provider"))
	FString OtherProviderApiKey;

	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration|Other", meta = (EditCondition = "SelectedProvider == ELLMProvider::Other", ToolTip="Endpoint URL for the alternative provider"))
	FString OtherProviderEndpoint;
	

	// Display Settings

	UPROPERTY(Config, EditAnywhere, Category = "Display Settings", meta=(ToolTip="Maximum number of characters to show in the documentation preview in the Blueprint details panel"))
	int32 DocumentationPreviewChars;

public:
	//~ Begin UDeveloperSettings Interface
	virtual FName GetCategoryName() const override;
	virtual FText GetSectionText() const override;
	//~ End UDeveloperSettings Interface
};