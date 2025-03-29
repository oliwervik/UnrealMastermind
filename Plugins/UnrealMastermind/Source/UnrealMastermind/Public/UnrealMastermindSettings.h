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

	// The currently selected LLM provider
	UPROPERTY(Config, EditAnywhere, Category = "LLM Configuration")
	ELLMProvider SelectedProvider;

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

public:
	//~ Begin UDeveloperSettings Interface
	virtual FName GetCategoryName() const override;
	virtual FText GetSectionText() const override;
	//~ End UDeveloperSettings Interface
};