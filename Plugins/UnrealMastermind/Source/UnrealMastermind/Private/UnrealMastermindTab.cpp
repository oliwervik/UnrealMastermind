// Copyright Froströk. All Rights Reserved.

#include "UnrealMastermindTab.h"
#include "LLMConnector.h"
#include "BlueprintDocumentation.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboBox.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "K2Node_FunctionEntry.h"
#include "EdGraph/EdGraphPin.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

void SUnrealMastermindTab::Construct(const FArguments& InArgs)
{
	PopulateAvailableBlueprints();
	bIsGenerating = false;
	
	ChildSlot
	[
		SNew(SVerticalBox)
		
		// Title
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Unreal Mastermind - Blueprint Documentation Generator"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
		]
		
		// Blueprint Selection
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		[
			SNew(SHorizontalBox)
			
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 10, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Select Blueprint:"))
			]
			
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(BlueprintSelectionComboBox, SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&AvailableBlueprints)
				.OnGenerateWidget(this, &SUnrealMastermindTab::MakeBlueprintComboItemWidget)
				.OnSelectionChanged(this, &SUnrealMastermindTab::OnBlueprintSelected)
				[
					SAssignNew(SelectedBlueprintText, STextBlock)
					.Text(FText::FromString("Select a Blueprint"))
				]
			]
			
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(10, 0, 0, 0)
			[
				SNew(SButton)
				.Text(FText::FromString("Refresh"))
				.OnClicked_Lambda([this]()
				{
					PopulateAvailableBlueprints();
					return FReply::Handled();
				})
			]
		]
		
		// Custom Prompt Input
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		[
			SNew(SVerticalBox)
			
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 5)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Custom Prompt (Optional):"))
			]
			
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(CustomPromptTextBox, SEditableTextBox)
				.HintText(FText::FromString("Add specific instructions for the AI documentation generator..."))
			]
		]
		
		// Loading Indicator
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		[
			SAssignNew(LoadingIndicatorBox, SVerticalBox)
			.Visibility_Lambda([this]() -> EVisibility { 
				return bIsGenerating ? EVisibility::Visible : EVisibility::Collapsed; 
			})
			
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 5)
			[
				SAssignNew(GenerationStatusText, STextBlock)
				.Text(FText::FromString("Generating documentation..."))
			]
			
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(GenerationProgressBar, SProgressBar)
				.Percent(0.0f)
				.BarFillType(EProgressBarFillType::LeftToRight)
				.FillColorAndOpacity(FLinearColor(0.0f, 0.5f, 1.0f, 1.0f))
			]
		]
		
		// Documentation Output
		+SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(10)
		[
			SNew(SVerticalBox)
			
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 5)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Generated Documentation:"))
			]
			
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(DocumentationTextBox, SMultiLineEditableTextBox)
				.IsReadOnly(false)
				.HintText(FText::FromString("Documentation will appear here after generation..."))
				.AutoWrapText(true)
			]
		]
		
		// Buttons
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10)
		[
			SNew(SHorizontalBox)
			
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0, 0, 5, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.Text(FText::FromString("Generate Documentation"))
				.OnClicked(this, &SUnrealMastermindTab::OnGenerateDocumentationClicked)
				.IsEnabled_Lambda([this]() -> bool { return !bIsGenerating; })
			]
			
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(5, 0, 0, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.Text(FText::FromString("Save Documentation"))
				.OnClicked(this, &SUnrealMastermindTab::OnSaveDocumentationClicked)
				.IsEnabled_Lambda([this]() -> bool { 
					return !bIsGenerating && !GeneratedDocumentation.IsEmpty(); 
				})
			]
		]
	];
	
	// Initialize with hidden loading indicator
	SetGenerationStatus(false);
}

void SUnrealMastermindTab::SetGenerationStatus(bool bIsGenerating, float Progress, const FString& StatusMessage)
{
	this->bIsGenerating = bIsGenerating;
	
	if (GenerationProgressBar.IsValid())
	{
		GenerationProgressBar->SetPercent(Progress);
	}
	
	if (GenerationStatusText.IsValid() && !StatusMessage.IsEmpty())
	{
		GenerationStatusText->SetText(FText::FromString(StatusMessage));
	}
}

void SUnrealMastermindTab::PopulateAvailableBlueprints()
{
	AvailableBlueprints.Empty();
	
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> BlueprintAssets;
	
	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;
	
	AssetRegistryModule.Get().GetAssets(Filter, BlueprintAssets);
	
	for (const FAssetData& AssetData : BlueprintAssets)
	{
		FString AssetName = AssetData.AssetName.ToString();
		AvailableBlueprints.Add(MakeShareable(new FString(AssetName)));
	}
}

TSharedRef<SWidget> SUnrealMastermindTab::MakeBlueprintComboItemWidget(TSharedPtr<FString> BlueprintName)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*BlueprintName));
}

void SUnrealMastermindTab::SelectBlueprint(const FString& BlueprintName)
{
    UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Selecting blueprint by name: %s"), *BlueprintName);
    
    // Find the blueprint in the available list
    TSharedPtr<FString> FoundItem;
    for (TSharedPtr<FString> Item : AvailableBlueprints)
    {
        if (*Item == BlueprintName)
        {
            FoundItem = Item;
            break;
        }
    }
    
    // If found, select it in the combo box
    if (FoundItem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Found blueprint in dropdown, selecting it"));
        BlueprintSelectionComboBox->SetSelectedItem(FoundItem);
        // This should trigger OnBlueprintSelected which will update everything else
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Blueprint not found in dropdown list"));
        
        // Try to find the blueprint directly to refresh the list
        UObject* FoundObject = FindObject<UObject>(nullptr, *FString::Printf(TEXT("/Game/%s.%s"), 
            *BlueprintName, *BlueprintName));
        
        if (!FoundObject)
        {
            // Try another common pattern
            FoundObject = FindObject<UObject>(nullptr, *FString::Printf(TEXT("/Game/Blueprints/%s.%s"), 
                *BlueprintName, *BlueprintName));
        }
        
        if (FoundObject && FoundObject->IsA<UBlueprint>())
        {
            UBlueprint* BP = Cast<UBlueprint>(FoundObject);
            if (BP)
            {
                // Try again after refresh
                for (TSharedPtr<FString> Item : AvailableBlueprints)
                {
                    if (*Item == BlueprintName)
                    {
                        BlueprintSelectionComboBox->SetSelectedItem(Item);
                        break;
                    }
                }
            }
        }
    }
}

void SUnrealMastermindTab::OnBlueprintSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectedItem.IsValid())
	{
		CurrentSelectedBlueprint = SelectedItem;
		SelectedBlueprintText->SetText(FText::FromString(*SelectedItem));
		
		// Check if the selected blueprint has documentation and load it
		UBlueprint* Blueprint = GetSelectedBlueprint();
		if (Blueprint)
		{
			FString ExistingDocumentation = UBlueprintDocumentation::GetDocumentation(Blueprint);
			if (!ExistingDocumentation.IsEmpty())
			{
				// Existing documentation found, load it
				GeneratedDocumentation = ExistingDocumentation;
				DocumentationTextBox->SetText(FText::FromString(GeneratedDocumentation));
			}
			else
			{
				// No documentation found, clear the text area
				GeneratedDocumentation = TEXT("");
				DocumentationTextBox->SetText(FText::FromString(TEXT("")));
			}
		}
		else
		{
			// Invalid blueprint, clear the text area
			GeneratedDocumentation = TEXT("");
			DocumentationTextBox->SetText(FText::FromString(TEXT("")));
		}
	}
}

UBlueprint* SUnrealMastermindTab::GetSelectedBlueprint() const
{
	if (!CurrentSelectedBlueprint.IsValid())
		return nullptr;
		
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	
	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.PackageNames.Add(*FString::Printf(TEXT("/Game/%s"), **CurrentSelectedBlueprint));
	
	TArray<FAssetData> AssetData;
	AssetRegistryModule.Get().GetAssets(Filter, AssetData);
	
	for (const FAssetData& Asset : AssetData)
	{
		if (Asset.AssetName.ToString() == *CurrentSelectedBlueprint)
		{
			return Cast<UBlueprint>(Asset.GetAsset());
		}
	}
	
	// Try more general search if the first approach fails
	TArray<FAssetData> AllAssets;
	Filter.PackageNames.Empty();
	AssetRegistryModule.Get().GetAssets(Filter, AllAssets);
	
	for (const FAssetData& Asset : AllAssets)
	{
		if (Asset.AssetName.ToString() == *CurrentSelectedBlueprint)
		{
			return Cast<UBlueprint>(Asset.GetAsset());
		}
	}
	
	return nullptr;
}

FString SUnrealMastermindTab::ExtractBlueprintInfo(UBlueprint* Blueprint)
{
	if (!Blueprint)
		return TEXT("Invalid Blueprint");
		
	FString BlueprintInfo;
	
	// Basic Blueprint Info
	BlueprintInfo += FString::Printf(TEXT("Blueprint Name: %s\n"), *Blueprint->GetName());
	BlueprintInfo += FString::Printf(TEXT("Parent Class: %s\n\n"), *Blueprint->ParentClass->GetName());
	
	// Variables
	BlueprintInfo += TEXT("Variables:\n");
	for (FBPVariableDescription& Variable : Blueprint->NewVariables)
	{
		BlueprintInfo += FString::Printf(TEXT("- %s (%s)\n"), 
			*Variable.VarName.ToString(), 
			*Variable.VarType.PinCategory.ToString());
			
		if (!Variable.DefaultValue.IsEmpty())
		{
			BlueprintInfo += FString::Printf(TEXT("  Default Value: %s\n"), *Variable.DefaultValue);
		}
	}
	
	BlueprintInfo += TEXT("\n");
	
	// Functions
	BlueprintInfo += TEXT("Functions:\n");
	for (UEdGraph* FunctionGraph : Blueprint->FunctionGraphs)
	{
		BlueprintInfo += FString::Printf(TEXT("- %s\n"), *FunctionGraph->GetName());
		
		// Find function entry node to get parameters
		for (UEdGraphNode* Node : FunctionGraph->Nodes)
		{
			UK2Node_FunctionEntry* EntryNode = Cast<UK2Node_FunctionEntry>(Node);
			if (EntryNode)
			{
				BlueprintInfo += TEXT("  Parameters:\n");
				for (FEdGraphPinReference Pin : EntryNode->Pins)
				{
					if (Pin.Get()->Direction == EGPD_Output)
					{
						BlueprintInfo += FString::Printf(TEXT("    - %s (%s)\n"), 
							*Pin.Get()->PinName.ToString(), 
							*Pin.Get()->PinType.PinCategory.ToString());
					}
				}
				break;
			}
		}
	}
	
	// Components (for Actor blueprints)
	if (Blueprint->SimpleConstructionScript)
	{
		TArray<USCS_Node*> ComponentNodes = Blueprint->SimpleConstructionScript->GetAllNodes();
		if (ComponentNodes.Num() > 0)
		{
			BlueprintInfo += TEXT("\nComponents:\n");
			for (USCS_Node* Node : ComponentNodes)
			{
				BlueprintInfo += FString::Printf(TEXT("- %s (%s)\n"), 
					*Node->GetVariableName().ToString(), 
					*Node->ComponentClass->GetName());
			}
		}
	}
	
	return BlueprintInfo;
}

FReply SUnrealMastermindTab::OnGenerateDocumentationClicked()
{
	UBlueprint* SelectedBlueprint = GetSelectedBlueprint();
	if (!SelectedBlueprint)
	{
		// Show error notification
		FNotificationInfo Info(FText::FromString("Please select a valid Blueprint first."));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		return FReply::Handled();
	}
	
	// Get the custom prompt text from the UI thread BEFORE starting background work
	FString CustomPromptText = CustomPromptTextBox->GetText().ToString();
	
	// Show the loading indicator
	SetGenerationStatus(true, 0.0f, "Analyzing Blueprint structure...");
	
	// Clear previous results
	DocumentationTextBox->SetText(FText::FromString(""));
	
	// Generate documentation on a background thread to not freeze UI
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, SelectedBlueprint, CustomPromptText]()
	{
		// Extract blueprint info
		FString BlueprintInfo = ExtractBlueprintInfo(SelectedBlueprint);
		
		// Update UI to show we're contacting the LLM
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			SetGenerationStatus(true, 0.3f, "Contacting AI model...");
		});
		
		// Create the LLM connector and generate documentation
		ULLMConnector* LLMConnector = NewObject<ULLMConnector>();
		FString GeneratedDoc = LLMConnector->GenerateDocumentation(BlueprintInfo, CustomPromptText);
		
		// Update UI to show we're processing the response
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			SetGenerationStatus(true, 0.7f, "Processing response...");
		});
		
		// Small delay to show the progress bar moving
		FPlatformProcess::Sleep(0.5f);
		
		// Update the UI on the game thread
		AsyncTask(ENamedThreads::GameThread, [this, GeneratedDoc]()
		{
			// Now it's safe to update UI elements
			GeneratedDocumentation = GeneratedDoc;
			DocumentationTextBox->SetText(FText::FromString(GeneratedDocumentation));
			
			// Hide the loading indicator
			SetGenerationStatus(false, 1.0f, "Generation complete");
			
			// Check if the result contains an error message
			bool bIsError = GeneratedDocumentation.StartsWith(TEXT("Error:"));
			
			// Show appropriate notification
			if (bIsError)
			{
				// Extract error message (limited to first line)
				FString ErrorMessage = GeneratedDocumentation;
				int32 NewlineIndex;
				if (ErrorMessage.FindChar('\n', NewlineIndex))
				{
					ErrorMessage = ErrorMessage.Left(NewlineIndex);
				}
				
				// Show error notification
				FNotificationInfo ErrorInfo(FText::FromString(ErrorMessage));
				ErrorInfo.ExpireDuration = 5.0f;
				ErrorInfo.bUseSuccessFailIcons = true;
				ErrorInfo.Image = FCoreStyle::Get().GetBrush(TEXT("MessageLog.Error"));
				FSlateNotificationManager::Get().AddNotification(ErrorInfo);
			}
			else
			{
				// Show success notification
				FNotificationInfo SuccessInfo(FText::FromString("Documentation generated successfully!"));
				SuccessInfo.ExpireDuration = 3.0f;
				SuccessInfo.bUseSuccessFailIcons = true;
				SuccessInfo.Image = FCoreStyle::Get().GetBrush(TEXT("MessageLog.Success"));
				FSlateNotificationManager::Get().AddNotification(SuccessInfo);
			}
		});
	});
	
	return FReply::Handled();
}

FReply SUnrealMastermindTab::OnSaveDocumentationClicked()
{
	UBlueprint* SelectedBlueprint = GetSelectedBlueprint();
	if (!SelectedBlueprint || GeneratedDocumentation.IsEmpty())
	{
		// Show error notification
		FText ErrorMessage = GeneratedDocumentation.IsEmpty() ? 
			FText::FromString("Please generate documentation first.") : 
			FText::FromString("Please select a valid Blueprint first.");
			
		FNotificationInfo Info(ErrorMessage);
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		return FReply::Handled();
	}
	
	// Get the current documentation text (in case the user edited it)
	FString DocumentationText = DocumentationTextBox->GetText().ToString();
	
	// Save documentation to the Blueprint's metadata
	UBlueprintDocumentation::SaveDocumentation(SelectedBlueprint, DocumentationText);
	
	// Show success notification
	FNotificationInfo SuccessInfo(FText::FromString("Documentation saved successfully!"));
	SuccessInfo.ExpireDuration = 3.0f;
	FSlateNotificationManager::Get().AddNotification(SuccessInfo);
	
	return FReply::Handled();
}