// Copyright 2025 © Froströk. All Rights Reserved.

#include "LLMConnector.h"
#include "HttpModule.h"
#include "UnrealMastermindSettings.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

ULLMConnector::ULLMConnector()
{
	RequestTimeout = 60.0f;
	bRequestComplete = false;
	Result = TEXT("");
}

FString ULLMConnector::GenerateDocumentation(const FString& BlueprintInfo, const FString& CustomPrompt)
{
	// Create the prompt for the AI
	const FString Prompt = CreatePrompt(BlueprintInfo, CustomPrompt);

	// Get the settings
	const UUnrealMastermindSettings* Settings = GetDefault<UUnrealMastermindSettings>();

	// Call the appropriate provider
	switch (Settings->SelectedProvider)
	{
	case ELLMProvider::OpenAI:
		return GenerateWithOpenAI(Prompt);
	case ELLMProvider::Anthropic:
		return GenerateWithAnthropic(Prompt);
	case ELLMProvider::Other:
		return GenerateWithOtherProvider(Prompt);
	default:
		return TEXT("Error: Unknown provider selected.");
	}
}

FString ULLMConnector::CreatePrompt(const FString& BlueprintInfo, const FString& CustomPrompt)
{
	const UUnrealMastermindSettings* Settings = GetDefault<UUnrealMastermindSettings>();

	FString Prompt = TEXT(
		"I need you to analyze this Unreal Engine Blueprint and provide professional documentation for it. ");

	// Add settings-based instructions
	if (Settings->bIncludeOverallSummary)
	{
		Prompt += TEXT("Include an overall summary of what this Blueprint does. ");
	}

	if (Settings->bIncludeVariableDescriptions)
	{
		Prompt += TEXT("Provide descriptions for each variable and its purpose. ");
	}

	if (Settings->bIncludeFunctionBreakdowns)
	{
		Prompt += TEXT("Break down each function and explain what it does. ");
	}

	// Add custom prompt if provided
	if (!CustomPrompt.IsEmpty())
	{
		Prompt += TEXT("\nAdditional instructions: ") + CustomPrompt + TEXT("\n\n");
	}

	// Add the Blueprint info
	Prompt += TEXT("\nHere is the Blueprint information:\n\n");
	Prompt += BlueprintInfo;

	return Prompt;
}

FString ULLMConnector::GenerateWithOpenAI(const FString& Prompt)
{
	const UUnrealMastermindSettings* Settings = GetDefault<UUnrealMastermindSettings>();
	const FString ApiKey = Settings->OpenAIApiKey;
	const FString Model = Settings->OpenAIModel;

	if (ApiKey.IsEmpty())
	{
		return TEXT("Error: OpenAI API key not provided. Please enter your API key in the plugin settings.");
	}

	// Create the JSON request
	const TSharedPtr<FJsonObject> RequestJsonObject = MakeShareable(new FJsonObject);
	RequestJsonObject->SetStringField(TEXT("model"), Model);

	TArray<TSharedPtr<FJsonValue>> MessagesArray;

	// System message
	const TSharedPtr<FJsonObject> SystemMessageObject = MakeShareable(new FJsonObject);
	SystemMessageObject->SetStringField(TEXT("role"), TEXT("system"));
	SystemMessageObject->SetStringField(TEXT("content"), Settings->SystemPrompt);
	MessagesArray.Add(MakeShareable(new FJsonValueObject(SystemMessageObject)));

	// User message with the prompt
	const TSharedPtr<FJsonObject> UserMessageObject = MakeShareable(new FJsonObject);
	UserMessageObject->SetStringField(TEXT("role"), TEXT("user"));
	UserMessageObject->SetStringField(TEXT("content"), Prompt);

	MessagesArray.Add(MakeShareable(new FJsonValueObject(UserMessageObject)));

	RequestJsonObject->SetArrayField(TEXT("messages"), MessagesArray);
	RequestJsonObject->SetNumberField(TEXT("max_tokens"), Settings->MaxTokens);
	RequestJsonObject->SetNumberField(TEXT("temperature"), Settings->Temperature);

	// Convert to string
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestJsonObject.ToSharedRef(), Writer);

	// Create the HTTP request
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Settings->OpenAIEndpoint);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *ApiKey));
	HttpRequest->SetContentAsString(RequestBody);

	// Set up the response handling
	Result.Empty();
	bRequestComplete = false;
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &ULLMConnector::OnResponseReceived);

	// Send the request
	HttpRequest->ProcessRequest();

	// Wait for completion or timeout
	const double StartTime = FPlatformTime::Seconds();
	while (!bRequestComplete)
	{
		FPlatformProcess::Sleep(0.1f);

		if (FPlatformTime::Seconds() - StartTime > RequestTimeout)
		{
			return TEXT("Error: Request timed out.");
		}
	}

	return Result;
}

FString ULLMConnector::GenerateWithAnthropic(const FString& Prompt)
{
    const UUnrealMastermindSettings* Settings = GetDefault<UUnrealMastermindSettings>();
    FString ApiKey = Settings->AnthropicApiKey;
    FString Model = Settings->AnthropicModel;
    FString Endpoint = Settings->AnthropicApiEndpoint;
    
    if (ApiKey.IsEmpty())
    {
        return TEXT("Error: Anthropic API key not provided. Please enter your API key in the plugin settings.");
    }
    
    if (Endpoint.IsEmpty())
    {
        // Default Anthropic API endpoint
        Endpoint = TEXT("https://api.anthropic.com/v1/messages");
    }
    
    // Create the JSON request
    TSharedPtr<FJsonObject> RequestJsonObject = MakeShareable(new FJsonObject);
    RequestJsonObject->SetStringField(TEXT("model"), Model);
    
    // Set system prompt at top level for Anthropic
    RequestJsonObject->SetStringField(TEXT("system"), Settings->SystemPrompt);
    
    // Create messages array (for Anthropic, this should only have user messages)
    TArray<TSharedPtr<FJsonValue>> MessagesArray;
    
    // User message with the prompt
    TSharedPtr<FJsonObject> UserMessageObject = MakeShareable(new FJsonObject);
    UserMessageObject->SetStringField(TEXT("role"), TEXT("user"));
    UserMessageObject->SetStringField(TEXT("content"), Prompt);
    MessagesArray.Add(MakeShareable(new FJsonValueObject(UserMessageObject)));
    
    RequestJsonObject->SetArrayField(TEXT("messages"), MessagesArray);
    RequestJsonObject->SetNumberField(TEXT("max_tokens"), Settings->MaxTokens);
    RequestJsonObject->SetNumberField(TEXT("temperature"), Settings->Temperature);
    
    // Convert to string
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(RequestJsonObject.ToSharedRef(), Writer);
    
    // Create the HTTP request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(Endpoint);
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetHeader(TEXT("x-api-key"), ApiKey);
    HttpRequest->SetHeader(TEXT("anthropic-version"), TEXT("2023-06-01"));  // Important for Anthropic API
    HttpRequest->SetContentAsString(RequestBody);
    
    // Set up the response handling
    Result.Empty();
    bRequestComplete = false;
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &ULLMConnector::OnAnthropicResponseReceived);
    
    // Send the request
    HttpRequest->ProcessRequest();
    
    // Wait for completion or timeout
    double StartTime = FPlatformTime::Seconds();
    while (!bRequestComplete)
    {
        FPlatformProcess::Sleep(0.1f);
        
        if (FPlatformTime::Seconds() - StartTime > RequestTimeout)
        {
            return TEXT("Error: Request timed out.");
        }
    }
    
    return Result;
}

FString ULLMConnector::GenerateWithOtherProvider(const FString& Prompt)
{
	const UUnrealMastermindSettings* Settings = GetDefault<UUnrealMastermindSettings>();
	const FString ApiKey = Settings->OtherProviderApiKey;
	const FString Endpoint = Settings->OtherProviderEndpoint;

	if (ApiKey.IsEmpty() || Endpoint.IsEmpty())
	{
		return TEXT(
			"Error: API key or endpoint not provided for the selected provider. Please check the plugin settings.");
	}

	// Create a simple JSON request that most providers would accept
	const TSharedPtr<FJsonObject> RequestJsonObject = MakeShareable(new FJsonObject);
	RequestJsonObject->SetStringField(TEXT("prompt"), Prompt);
	RequestJsonObject->SetNumberField(TEXT("max_tokens"), Settings->MaxTokens);
	RequestJsonObject->SetNumberField(TEXT("temperature"), Settings->Temperature);

	// Convert to string
	FString RequestBody;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestJsonObject.ToSharedRef(), Writer);

	// Create the HTTP request
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Endpoint);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *ApiKey));
	HttpRequest->SetContentAsString(RequestBody);

	// Set up the response handling
	Result.Empty();
	bRequestComplete = false;
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &ULLMConnector::OnResponseReceived);

	// Send the request
	HttpRequest->ProcessRequest();

	// Wait for completion or timeout
	const double StartTime = FPlatformTime::Seconds();
	while (!bRequestComplete)
	{
		FPlatformProcess::Sleep(0.1f);

		if (FPlatformTime::Seconds() - StartTime > RequestTimeout)
		{
			return TEXT("Error: Request timed out.");
		}
	}

	return Result;
}

void ULLMConnector::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		Result = TEXT("Error: Failed to connect to the API server.");
		bRequestComplete = true;
		return;
	}

	if (Response->GetResponseCode() != 200)
	{
		Result = FString::Printf(TEXT("Error: HTTP %d - %s"), Response->GetResponseCode(),
		                         *Response->GetContentAsString());
		bRequestComplete = true;
		return;
	}

	// Parse the JSON response
	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		const UUnrealMastermindSettings* Settings = GetDefault<UUnrealMastermindSettings>();

		// Different providers return responses in different structures
		if (Settings->SelectedProvider == ELLMProvider::OpenAI)
		{
			// Parse OpenAI response
			TArray<TSharedPtr<FJsonValue>> Choices = JsonObject->GetArrayField(TEXT("choices"));
			if (Choices.Num() > 0)
			{
				const TSharedPtr<FJsonObject> Choice = Choices[0]->AsObject();
				const TSharedPtr<FJsonObject> Message = Choice->GetObjectField(TEXT("message"));
				Result = Message->GetStringField(TEXT("content"));
			}
			else
			{
				Result = TEXT("Error: Could not parse API response from OpenAI.");
			}
		}
		else if (Settings->SelectedProvider == ELLMProvider::Anthropic)
		{
			// Parse Anthropic response
			const TSharedPtr<FJsonObject> Content = JsonObject->GetObjectField(TEXT("content"));
			if (Content.IsValid())
			{
				TArray<TSharedPtr<FJsonValue>> Parts = Content->GetArrayField(TEXT("parts"));
				if (Parts.Num() > 0)
				{
					Result = Parts[0]->AsObject()->GetStringField(TEXT("text"));
				}
				else
				{
					Result = TEXT("Error: Could not parse API response from Anthropic.");
				}
			}
			else
			{
				Result = TEXT("Error: Could not parse API response from Anthropic.");
			}
		}
		else
		{
			// Generic parser for other providers - try common response formats
			if (JsonObject->HasField(TEXT("text")))
			{
				Result = JsonObject->GetStringField(TEXT("text"));
			}
			else if (JsonObject->HasField(TEXT("output")))
			{
				Result = JsonObject->GetStringField(TEXT("output"));
			}
			else if (JsonObject->HasField(TEXT("result")))
			{
				Result = JsonObject->GetStringField(TEXT("result"));
			}
			else if (JsonObject->HasField(TEXT("response")))
			{
				Result = JsonObject->GetStringField(TEXT("response"));
			}
			else
			{
				// Just return the entire JSON as a fallback
				Result = Response->GetContentAsString();
			}
		}
	}
	else
	{
		Result = TEXT("Error: Could not parse API response.");
	}

	bRequestComplete = true;
}

void ULLMConnector::OnAnthropicResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
        
        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            // Extract the content from Anthropic's response
            if (JsonObject->HasField(TEXT("content")))
            {
                const TArray<TSharedPtr<FJsonValue>>* ContentArray = nullptr;
                if (JsonObject->TryGetArrayField(TEXT("content"), ContentArray) && ContentArray != nullptr)
                {
                    FString ResultText;
                    for (const TSharedPtr<FJsonValue>& ContentItem : *ContentArray)
                    {
                        const TSharedPtr<FJsonObject>* ContentObject = nullptr;
                        if (ContentItem->TryGetObject(ContentObject) && ContentObject != nullptr)
                        {
                            FString TextValue;
                            if ((*ContentObject)->TryGetStringField(TEXT("text"), TextValue))
                            {
                                ResultText += TextValue;
                            }
                        }
                    }
                    Result = ResultText;
                }
            }
            else if (JsonObject->HasField(TEXT("error")))
            {
                // Handle error
                const TSharedPtr<FJsonObject>* ErrorObject = nullptr;
                if (JsonObject->TryGetObjectField(TEXT("error"), ErrorObject) && ErrorObject != nullptr)
                {
                    FString ErrorMessage;
                    if ((*ErrorObject)->TryGetStringField(TEXT("message"), ErrorMessage))
                    {
                        Result = FString::Printf(TEXT("Error: %s"), *ErrorMessage);
                    }
                    else
                    {
                        Result = TEXT("Error: Unknown error occurred");
                    }
                }
                else
                {
                    Result = TEXT("Error: Failed to parse error object");
                }
            }
            else
            {
                Result = TEXT("Error: Unexpected response format");
            }
        }
        else
        {
            Result = TEXT("Error: Failed to parse response JSON");
        }
    }
    else
    {
        Result = TEXT("Error: Request failed");
        
        if (Response.IsValid())
        {
            Result += FString::Printf(TEXT(" (HTTP %d)"), Response->GetResponseCode());
            Result += TEXT(" - ") + Response->GetContentAsString();
        }
    }
    
    bRequestComplete = true;
}

