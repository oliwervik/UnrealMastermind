// Copyright Froströk. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UnrealMastermindSettings.generated.h"

UENUM(BlueprintType)
enum class ELLMProvider : uint8
{
	OpenAI UMETA(DisplayName = "OpenAI"),
	Claude UMETA(DisplayName = "Claude"),
	Other UMETA(DisplayName = "Other")
};

UCLASS(Config=EditorPerProjectUserSettings, DefaultConfig, meta=(DisplayName="Unreal Mastermind"))
class UNREALMASTERMIND_API UUnrealMastermindSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UUnrealMastermindSettings();

	UPROPERTY(config, EditAnywhere, Category="Documentation Generation")
	int32 ComponentDetailLevel = 1; // 0=Minimal, 1=Basic, 2=Full
    
	UPROPERTY(config, EditAnywhere, Category="Documentation Generation")
	bool bTrackVariableUsage = true;
    
	UPROPERTY(config, EditAnywhere, Category="Documentation Generation")
	int32 MaxExecutionFlowDepth = 5;
	
	UPROPERTY(config, EditAnywhere, Category="Documentation Generation")
	bool bIncludeComments = true;

	UPROPERTY(config, EditAnywhere, Category="Documentation Generation")
	TArray<FString> IgnoredPropertyPrefixes = { "Example" };  //

	// The currently selected LLM provider
	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration")
	ELLMProvider SelectedProvider;

	// AI Prompt Settings
	UPROPERTY(config, EditAnywhere, Category="AI Settings", meta=(DisplayName="System Prompt", MultiLine=true))
	FString SystemPrompt = TEXT("You are a professional technical documentation writer specializing in Unreal Engine Blueprints. "
							"Your task is to provide comprehensive, clear documentation that explains: "
							"1. The purpose of the Blueprint (what it does) "
							"2. How the Blueprint works (key mechanisms and flow) "
							"3. Important variables and their purposes "
							"4. Main function and event behaviors "
							"5. Component setup and configuration "
							"6. Recommendations for usage or potential improvements "
							"Format the documentation in a clear, structured way with headings, bullet points, and code-like notation for Blueprint nodes. Dont specifically document individual node behaviours such as ForEach Loops, Then etc. Instead focus on summarizing the behaviour, intention and functionality of the blueprint.");

	UPROPERTY(config, EditAnywhere, Category= "AI Settings", meta=(DisplayName="Temperature", ClampMin="0.0", ClampMax="1.0"))
	float Temperature = 0.5f;

	UPROPERTY(config, EditAnywhere, Category= "AI Settings", meta=(DisplayName="Max Tokens", ClampMin="100", ClampMax="16000"))
	int32 MaxTokens = 4000;

	// OpenAI Configuration
	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration|OpenAI", meta = (EditCondition = "SelectedProvider == ELLMProvider::OpenAI"))
	FString OpenAIApiKey;

	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration|OpenAI", meta = (EditCondition = "SelectedProvider == ELLMProvider::OpenAI"))
	FString OpenAIModel;

	// Claude Configuration
	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration|Claude", meta = (EditCondition = "SelectedProvider == ELLMProvider::Claude"))
	FString ClaudeApiKey;

	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration|Claude", meta = (EditCondition = "SelectedProvider == ELLMProvider::Claude"))
	FString ClaudeModel;

	// Other Provider Configuration
	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration|Other", meta = (EditCondition = "SelectedProvider == ELLMProvider::Other"))
	FString OtherProviderName;

	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration|Other", meta = (EditCondition = "SelectedProvider == ELLMProvider::Other"))
	FString OtherProviderApiKey;

	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration|Other", meta = (EditCondition = "SelectedProvider == ELLMProvider::Other"))
	FString OtherProviderEndpoint;
	

	// Documentation Settings
	UPROPERTY(Config, EditAnywhere, Category = "Documentation")
	bool bIncludeVariableDescriptions;

	UPROPERTY(Config, EditAnywhere, Category = "Documentation")
	bool bIncludeFunctionBreakdowns;

	UPROPERTY(Config, EditAnywhere, Category = "Documentation") 
	bool bIncludeOverallSummary;

	UPROPERTY(Config, EditAnywhere, Category = "Documentation") 
	// Amount of characters in the preview of documentation in blueprint details
	int32 DocumentationPreviewChars = 500;

public:
	//~ Begin UDeveloperSettings Interface
	virtual FName GetCategoryName() const override;
	virtual FText GetSectionText() const override;
	//~ End UDeveloperSettings Interface
};