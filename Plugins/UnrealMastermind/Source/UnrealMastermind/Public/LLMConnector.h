// Copyright YourCompany. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "LLMConnector.generated.h"

UCLASS()
class UNREALMASTERMIND_API ULLMConnector : public UObject
{
	GENERATED_BODY()
	
public:
	ULLMConnector();
	
	// Main function to generate documentation
	FString GenerateDocumentation(const FString& BlueprintInfo, const FString& CustomPrompt);
	
private:
	// Provider-specific implementations
	FString GenerateWithOpenAI(const FString& Prompt);
	FString GenerateWithClaude(const FString& Prompt);
	FString GenerateWithOtherProvider(const FString& Prompt);
	
	// Helper methods
	FString CreatePrompt(const FString& BlueprintInfo, const FString& CustomPrompt);
	
	// HTTP Request handlers
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	
	// Default timeout in seconds
	float RequestTimeout;
	
	// Result container
	FString Result;
	bool bRequestComplete;
};