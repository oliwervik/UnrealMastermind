// Copyright © Froströk. All Rights Reserved.
// This plugin is governed by the Unreal Engine Marketplace EULA.
// This software cannot be redistributed, modified, or resold outside of the original purchase.

#include "UnrealMastermindTab.h"
#include "LLMConnector.h"
#include "BlueprintDocumentation.h"
#include "BlueprintDocumentationSettings.h"
#include "EdGraphNode_Comment.h"
#include "K2Node_Event.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "SSearchableComboBox.h"
#include "UnrealMastermindSettings.h"
#include "EdGraph/EdGraphPin.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

void SUnrealMastermindTab::Construct(const FArguments& InArgs)
{
	PopulateAvailableBlueprints();
	bIsGenerating = false;

	// Create detail level options
	DetailLevelOptions.Add(MakeShareable(new FString(TEXT("Minimal"))));
	DetailLevelOptions.Add(MakeShareable(new FString(TEXT("Basic"))));
	DetailLevelOptions.Add(MakeShareable(new FString(TEXT("Full"))));

	// Get initial settings
	const UUnrealMastermindSettings* Settings = GetDefault<UUnrealMastermindSettings>();

	ChildSlot
	[
		SNew(SVerticalBox)

		// Title
		+ SVerticalBox::Slot()
		  .AutoHeight()
		  .Padding(10)
		  .HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Unreal Mastermind - Blueprint Documentation Generator"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
		]

		// Blueprint Selection
		+ SVerticalBox::Slot()
		  .AutoHeight()
		  .Padding(10)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			  .AutoWidth()
			  .VAlign(VAlign_Center)
			  .Padding(0, 0, 10, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Select Blueprint:"))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(BlueprintSelectionComboBox, SSearchableComboBox)
		.OptionsSource(&AvailableBlueprints)
		.OnGenerateWidget(this, &SUnrealMastermindTab::MakeBlueprintComboItemWidget)
		.OnSelectionChanged(this, &SUnrealMastermindTab::OnBlueprintSelected)
		.ContentPadding(FMargin(2.0f, 2.0f))
		.MaxListHeight(200.0f)
		.Content()
				[
					SAssignNew(SelectedBlueprintText, STextBlock)
					.Text(FText::FromString("Select a Blueprint"))
				]
			]

			+ SHorizontalBox::Slot()
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

		// Documentation Settings
		+ SVerticalBox::Slot()
		  .AutoHeight()
		  .Padding(10)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			  .AutoHeight()
			  .Padding(0, 0, 0, 5)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Documentation Settings:"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]

			// Detail Level Setting
			+ SVerticalBox::Slot()
			  .AutoHeight()
			  .Padding(0, 5)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				  .AutoWidth()
				  .VAlign(VAlign_Center)
				  .Padding(0, 0, 10, 0)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Detail Level:"))
					.MinDesiredWidth(120.0f)
				]
				+ SHorizontalBox::Slot()
				  .AutoWidth()
				  .Padding(5, 0)
				[
					SAssignNew(DetailLevelComboBox, SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&DetailLevelOptions)
					.InitiallySelectedItem(DetailLevelOptions[Settings->ComponentDetailLevel])
					.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewValue, ESelectInfo::Type)
					{
						// Get a mutable settings object
						UUnrealMastermindSettings* Settings = GetMutableDefault<UUnrealMastermindSettings>();

						// Update the setting
						if (*NewValue == TEXT("Minimal"))
							Settings->ComponentDetailLevel = 0;
						else if (*NewValue == TEXT("Basic"))
							Settings->ComponentDetailLevel = 1;
						else if (*NewValue == TEXT("Full"))
							Settings->ComponentDetailLevel = 2;

						// Save the setting
						Settings->SaveConfig();
					})
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
					{
						return SNew(STextBlock).Text(FText::FromString(*Item));
					})
					.Content()
					[
						SNew(STextBlock)
						.Text_Lambda([this]()
						{
							const UUnrealMastermindSettings* Settings = GetDefault<UUnrealMastermindSettings>();
							return FText::FromString(*DetailLevelOptions[Settings->ComponentDetailLevel].Get());
						})
					]
				]
			]

			// Variable Tracking Option
			+ SVerticalBox::Slot()
			  .AutoHeight()
			  .Padding(0, 5)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				  .AutoWidth()
				  .VAlign(VAlign_Center)
				  .Padding(0, 0, 10, 0)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Track Variable Usage:"))
					.MinDesiredWidth(120.0f)
				]
				+ SHorizontalBox::Slot()
				  .AutoWidth()
				  .Padding(5, 0)
				[
					SNew(SCheckBox)
					.IsChecked(Settings->bTrackVariableUsage ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
					.OnCheckStateChanged_Lambda([](ECheckBoxState NewState)
					               {
						               UUnrealMastermindSettings* Settings = GetMutableDefault<
							               UUnrealMastermindSettings>();
						               Settings->bTrackVariableUsage = (NewState == ECheckBoxState::Checked);
						               Settings->SaveConfig();
					               })
				]
			]

			// Max Flow Depth
			+ SVerticalBox::Slot()
			  .AutoHeight()
			  .Padding(0, 5)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				  .AutoWidth()
				  .VAlign(VAlign_Center)
				  .Padding(0, 0, 10, 0)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Max Execution Flow Depth:"))
					.MinDesiredWidth(120.0f)
				]
				+ SHorizontalBox::Slot()
				  .AutoWidth()
				  .Padding(5, 0)
				[
					SNew(SSpinBox<int32>)
					.Value(Settings->MaxExecutionFlowDepth)
					.MinValue(1)
					.MaxValue(10)
					.Delta(1)
					.OnValueChanged_Lambda([](int32 NewValue)
					                     {
						                     UUnrealMastermindSettings* Settings = GetMutableDefault<
							                     UUnrealMastermindSettings>();
						                     Settings->MaxExecutionFlowDepth = NewValue;
						                     Settings->SaveConfig();
					                     }).MinDesiredWidth(30.0f)
				]
			]
		]

		// Custom Prompt Input
		+ SVerticalBox::Slot()
		  .AutoHeight()
		  .Padding(10)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			  .AutoHeight()
			  .Padding(0, 0, 0, 5)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Custom Prompt (Optional):"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(CustomPromptTextBox, SEditableTextBox)
				.HintText(FText::FromString("Add specific instructions for the AI documentation generator..."))
			]
		]

		// Loading Indicator
		+ SVerticalBox::Slot()
		  .AutoHeight()
		  .Padding(10)
		[
			SAssignNew(LoadingIndicatorBox, SVerticalBox)
			.Visibility_Lambda([this]() -> EVisibility
			{
				return bIsGenerating ? EVisibility::Visible : EVisibility::Collapsed;
			})

			+ SVerticalBox::Slot()
			  .AutoHeight()
			  .Padding(0, 0, 0, 5)
			[
				SAssignNew(GenerationStatusText, STextBlock)
				.Text(FText::FromString("Generating documentation..."))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(GenerationProgressBar, SProgressBar)
				.Percent(0.0f)
				.BarFillType(EProgressBarFillType::LeftToRight)
				.FillColorAndOpacity(FLinearColor(0.0f, 0.5f, 1.0f, 1.0f))
			]
		]

		// Documentation Output
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			  .AutoHeight()
			  .Padding(0, 0, 0, 5)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Generated Documentation:"))
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(DocumentationTextBox, SMultiLineEditableTextBox)
				.IsReadOnly(false)
				.HintText(FText::FromString("Documentation will appear here after generation..."))
				.AutoWrapText(true)
			]
		]

		// Buttons
		+ SVerticalBox::Slot()
		  .AutoHeight()
		  .Padding(10)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			  .FillWidth(1.0f)
			  .Padding(0, 0, 5, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.Text(FText::FromString("Generate Documentation"))
				.OnClicked(this, &SUnrealMastermindTab::OnGenerateDocumentationClicked)
				.IsEnabled_Lambda([this]() -> bool { return !bIsGenerating; })
			]

			+ SHorizontalBox::Slot()
			  .FillWidth(1.0f)
			  .Padding(5, 0, 0, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.Text(FText::FromString("Save Documentation"))
				.OnClicked(this, &SUnrealMastermindTab::OnSaveDocumentationClicked)
				.IsEnabled_Lambda([this]() -> bool
				             {
					             return !bIsGenerating && !GeneratedDocumentation.IsEmpty();
				             })
			]
		]
	];

	// Initialize with hidden loading indicator
	SetGenerationStatus(false);
}

void SUnrealMastermindTab::SetGenerationStatus(bool bGenerating, float Progress, const FString& StatusMessage)
{
	this->bIsGenerating = bGenerating;

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

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<
		FAssetRegistryModule>("AssetRegistry");
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
		BlueprintSelectionComboBox->SetSelectedItem(FoundItem);
	}
	else
	{
		// Try to find the blueprint directly to refresh the list
		UObject* FoundObject = FindObject<UObject>(nullptr, *FString::Printf(TEXT("/Game/%s.%s"),
		                                                                     *BlueprintName, *BlueprintName));

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
		if (UBlueprint* Blueprint = GetSelectedBlueprint())
		{
			if (FString ExistingDocumentation = UBlueprintDocumentation::GetDocumentation(Blueprint); !
				ExistingDocumentation.IsEmpty())
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

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<
		FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> BlueprintAssets;

	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;

	AssetRegistryModule.Get().GetAssets(Filter, BlueprintAssets);

	for (const FAssetData& Asset : BlueprintAssets)
	{
		if (Asset.AssetName.ToString() == *CurrentSelectedBlueprint)
		{
			return Cast<UBlueprint>(Asset.GetAsset());
		}
	}

	return nullptr;
}

void SUnrealMastermindTab::ExtractEventGraphInfo(UBlueprint* Blueprint, const FBlueprintDocumentationSettings& Settings,
                                                 FString& BlueprintInfo)
{
	BlueprintInfo += TEXT("\nEvent Graphs:\n");
	for (UEdGraph* EventGraph : Blueprint->UbergraphPages)
	{
		BlueprintInfo += FString::Printf(TEXT("- %s\n"), *EventGraph->GetName());

		// Find event nodes
		TArray<UK2Node_Event*> EventNodes;
		for (UEdGraphNode* Node : EventGraph->Nodes)
		{
			if (UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node))
			{
				EventNodes.Add(EventNode);
			}
		}

		// Process each event
		for (UK2Node_Event* EventNode : EventNodes)
		{
			BlueprintInfo += FString::Printf(
				TEXT("  Event: %s\n"), *EventNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString());

			// Follow execution flow from this event
			for (UEdGraphPin* Pin : EventNode->Pins)
			{
				if (Pin->Direction == EGPD_Output && Pin->PinType.PinCategory == "exec")
				{
					TraceExecutionFlow(Pin, BlueprintInfo, 2, Settings.MaxExecutionFlowDepth);
					break;
				}
			}
			BlueprintInfo += TEXT("\n");
		}
	}
}

void SUnrealMastermindTab::ExtractFunctionGraphInfo(UBlueprint* Blueprint,
                                                    const FBlueprintDocumentationSettings& Settings,
                                                    FString& BlueprintInfo)
{
	BlueprintInfo += TEXT("\nFunctions:\n");
	for (UEdGraph* FunctionGraph : Blueprint->FunctionGraphs)
	{
		BlueprintInfo += FString::Printf(TEXT("- %s\n"), *FunctionGraph->GetName());

		// Find function entry node to get parameters
		UK2Node_FunctionEntry* EntryNode = nullptr;
		for (UEdGraphNode* Node : FunctionGraph->Nodes)
		{
			if (UK2Node_FunctionEntry* PotentialEntryNode = Cast<UK2Node_FunctionEntry>(Node))
			{
				EntryNode = PotentialEntryNode;
				break;
			}
		}

		if (EntryNode)
		{
			// Parameters
			BlueprintInfo += TEXT("  Parameters:\n");
			for (UEdGraphPin* Pin : EntryNode->Pins)
			{
				if (Pin->Direction == EGPD_Output && Pin->PinType.PinCategory != "exec")
				{
					BlueprintInfo += FString::Printf(TEXT("    - %s (%s)\n"),
					                                 *Pin->PinName.ToString(),
					                                 *Pin->PinType.PinCategory.ToString());
				}
			}

			// Node execution flow - follow the exec lines
			BlueprintInfo += TEXT("  Execution Flow:\n");
			for (UEdGraphPin* Pin : EntryNode->Pins)
			{
				// Find the 'then' execution pin
				if (Pin->Direction == EGPD_Output && Pin->PinType.PinCategory == "exec")
				{
					TraceExecutionFlow(Pin, BlueprintInfo, 1, Settings.MaxExecutionFlowDepth);
					break;
				}
			}
		}
	}
}

void SUnrealMastermindTab::ExtractVariableInfo(UBlueprint* Blueprint, FString& BlueprintInfo) const
{
	// Add this to track where variables are used
	BlueprintInfo += TEXT("\nVariable Usages:\n");
	for (FBPVariableDescription& Variable : Blueprint->NewVariables)
	{
		FString VarName = Variable.VarName.ToString();
		BlueprintInfo += FString::Printf(TEXT("- %s is used in:\n"), *VarName);

		// Search all graphs for references to this variable
		bool bFoundUsage = false;
		for (UEdGraph* Graph : Blueprint->UbergraphPages)
		{
			for (UEdGraphNode* Node : Graph->Nodes)
			{
				// Look for get/set variable nodes
				const UK2Node_VariableGet* GetNode = Cast<UK2Node_VariableGet>(Node);
				const UK2Node_VariableSet* SetNode = Cast<UK2Node_VariableSet>(Node);

				if ((GetNode && GetNode->VariableReference.GetMemberName() == Variable.VarName) ||
					(SetNode && SetNode->VariableReference.GetMemberName() == Variable.VarName))
				{
					UEdGraph* OwningGraph = Node->GetGraph();
					BlueprintInfo += FString::Printf(TEXT("    %s (%s)\n"),
					                                 *OwningGraph->GetName(),
					                                 GetNode ? TEXT("Read") : TEXT("Write"));
					bFoundUsage = true;
				}
			}
		}

		if (!bFoundUsage)
		{
			BlueprintInfo += TEXT("    Not used in any graph\n");
		}
	}
}

FString SUnrealMastermindTab::ExtractBlueprintInfo(UBlueprint* Blueprint,
                                                   const FBlueprintDocumentationSettings& Settings =
	                                                   FBlueprintDocumentationSettings())
{
	if (!Blueprint)
		return TEXT("Invalid Blueprint");

	FString BlueprintInfo;

	// Basic Blueprint Info
	if (Settings.bIncludeBasicInfo)
	{
		BlueprintInfo += FString::Printf(TEXT("Blueprint Name: %s\n"), *Blueprint->GetName());
		BlueprintInfo += FString::Printf(TEXT("Parent Class: %s\n\n"), *Blueprint->ParentClass->GetName());
	}

	// Variables
	if (Settings.bIncludeVariables)
	{
		ExtractVariableInfo(Blueprint, BlueprintInfo);
	}

	ExtractEventGraphInfo(Blueprint, Settings, BlueprintInfo);
	ExtractFunctionGraphInfo(Blueprint, Settings, BlueprintInfo);

	// Enhanced component information
	if (Blueprint->SimpleConstructionScript)
	{
		if (TArray<USCS_Node*> ComponentNodes = Blueprint->SimpleConstructionScript->GetAllNodes(); ComponentNodes.Num()
			> 0)
		{
			BlueprintInfo += TEXT("\nComponents:\n");
			for (const USCS_Node* Node : ComponentNodes)
			{
				// Basic component info
				BlueprintInfo += FString::Printf(TEXT("- %s (%s)\n"),
				                                 *Node->GetVariableName().ToString(),
				                                 *Node->ComponentClass->GetName());

				// Component properties (optional, can be verbose)
				UActorComponent* TemplateComponent = Node->ComponentTemplate;
				if (TemplateComponent)
				{
					if (Settings.ComponentDetailLevel !=
						FBlueprintDocumentationSettings::EComponentDetailLevel::Minimal)
					{
						// Add important properties for Basic level
						AddImportantComponentProperties(BlueprintInfo, TemplateComponent, Node);

						// Add all non-default properties for Full level
						if (Settings.ComponentDetailLevel ==
							FBlueprintDocumentationSettings::EComponentDetailLevel::Full)
						{
							AddAllComponentProperties(BlueprintInfo, TemplateComponent, Node,
							                          Settings.IgnoredPropertyPrefixes);
						}
					}
				}
			}
		}
	}

	//Extract comments
	BlueprintInfo += TEXT("\nDocumentation Comments:\n");
	for (UEdGraph* Graph : Blueprint->UbergraphPages)
	{
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			if (UEdGraphNode_Comment* CommentNode = Cast<UEdGraphNode_Comment>(Node))
			{
				BlueprintInfo += FString::Printf(TEXT("- [%s] %s\n"),
				                                 *Graph->GetName(), *CommentNode->NodeComment);
			}
		}
	}

	return BlueprintInfo;
}

// Helper method for important properties
void SUnrealMastermindTab::AddImportantComponentProperties(FString& BlueprintInfo, UActorComponent* Component,
                                                           const USCS_Node* Node) const
{
	// Only add the most important properties
	FString ImportantProps[] = {
		"RelativeLocation",
		"RelativeRotation",
		"RelativeScale3D",
		"AttachParent",
		"Mobility"
	};

	for (const FString& PropName : ImportantProps)
	{
		FProperty* Property = Component->GetClass()->FindPropertyByName(FName(*PropName));
		if (Property)
		{
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Component);
			FString ValueStr;
			Property->ExportTextItem_Direct(ValueStr, ValuePtr, nullptr, nullptr, PPF_None);

			if (!ValueStr.IsEmpty() && ValueStr != TEXT("()"))
			{
				BlueprintInfo += FString::Printf(TEXT("    %s: %s\n"), *PropName, *ValueStr);
			}
		}
	}
}

// Helper method for all properties
void SUnrealMastermindTab::AddAllComponentProperties(FString& BlueprintInfo, UActorComponent* Component,
                                                     const USCS_Node* Node,
                                                     const TArray<FString>& IgnoredPrefixes) const
{
	for (TFieldIterator<FProperty> PropIt(Component->GetClass()); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		const FString PropName = Property->GetName();

		// Skip if we should ignore this property
		bool bShouldIgnore = false;
		for (const FString& Prefix : IgnoredPrefixes)
		{
			if (PropName.StartsWith(Prefix))
			{
				bShouldIgnore = true;
				break;
			}
		}

		if (bShouldIgnore || Property->HasAnyPropertyFlags(CPF_Deprecated | CPF_Transient))
			continue;

		void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Component);
		FString ValueStr;
		Property->ExportTextItem_Direct(ValueStr, ValuePtr, nullptr, nullptr, PPF_None);

		if (!ValueStr.IsEmpty() && ValueStr != TEXT("()"))
		{
			BlueprintInfo += FString::Printf(TEXT("    %s: %s\n"), *PropName, *ValueStr);
		}
	}
}

void SUnrealMastermindTab::TraceExecutionFlow(UEdGraphPin* ExecPin, FString& BlueprintInfo, int32 Depth, int32 MaxDepth)
{
	// Get the setting if not provided
	if (MaxDepth <= 0)
	{
		const UUnrealMastermindSettings* Settings = GetDefault<UUnrealMastermindSettings>();
		MaxDepth = Settings->MaxExecutionFlowDepth;
	}

	// Stop if we've reached the maximum depth
	if (Depth > MaxDepth)
	{
		FString Indent = FString::ChrN((Depth - 1) * 2, ' ');
		BlueprintInfo += FString::Printf(TEXT("%s→ ... (Max depth reached) ...\n"), *Indent);
		return;
	}

	// Stop if we have no pin or no connections
	if (!ExecPin || ExecPin->LinkedTo.Num() == 0)
		return;

	// Follow the first connection
	UEdGraphPin* ConnectedPin = ExecPin->LinkedTo[0];
	UEdGraphNode* ConnectedNode = ConnectedPin->GetOwningNode();

	// Indentation based on depth
	FString Indent = FString::ChrN(Depth * 2, ' ');

	// Describe the node
	UK2Node* K2Node = Cast<UK2Node>(ConnectedNode);
	if (K2Node)
	{
		// Add node title/type
		BlueprintInfo += FString::Printf(
			TEXT("%s→ %s\n"), *Indent, *K2Node->GetNodeTitle(ENodeTitleType::ListView).ToString());

		// Add input/output values
		for (UEdGraphPin* Pin : K2Node->Pins)
		{
			if (Pin->Direction == EGPD_Input && Pin->PinType.PinCategory != "exec")
			{
				FString PinValue = GetPinValue(Pin);
				BlueprintInfo += FString::Printf(TEXT("%s  Input: %s (%s) = %s\n"),
				                                 *Indent, *Pin->PinName.ToString(),
				                                 *Pin->PinType.PinCategory.ToString(), *PinValue);
			}
		}

		// Follow the next execution pin
		for (UEdGraphPin* Pin : K2Node->Pins)
		{
			if (Pin->Direction == EGPD_Output && Pin->PinType.PinCategory == "exec")
			{
				TraceExecutionFlow(Pin, BlueprintInfo, Depth + 1, MaxDepth);
				break;
			}
		}
	}
}

FString SUnrealMastermindTab::GetPinValue(UEdGraphPin* Pin) const
{
	// For connected pins, show what they connect to
	if (Pin->LinkedTo.Num() > 0)
	{
		const UEdGraphPin* ConnectedPin = Pin->LinkedTo[0];
		const UEdGraphNode* ConnectedNode = ConnectedPin->GetOwningNode();
		return FString::Printf(
			TEXT("Connected to %s"), *ConnectedNode->GetNodeTitle(ENodeTitleType::ListView).ToString());
	}

	// For literal values
	return Pin->DefaultValue;
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
		// Get the persistent settings
		const UUnrealMastermindSettings* PersistentSettings = GetDefault<UUnrealMastermindSettings>();

		// Create a documentation settings instance and populate from persistent settings
		FBlueprintDocumentationSettings DocSettings;
		DocSettings.ComponentDetailLevel = static_cast<FBlueprintDocumentationSettings::EComponentDetailLevel>(
			PersistentSettings->ComponentDetailLevel);
		DocSettings.bTrackVariableUsage = PersistentSettings->bTrackVariableUsage;
		DocSettings.MaxExecutionFlowDepth = PersistentSettings->MaxExecutionFlowDepth;
		DocSettings.bIncludeComments = PersistentSettings->bIncludeComments;
		DocSettings.IgnoredPropertyPrefixes = PersistentSettings->IgnoredPropertyPrefixes;

		// Extract blueprint info
		const FString BlueprintInfo = ExtractBlueprintInfo(SelectedBlueprint, DocSettings);

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

FReply SUnrealMastermindTab::OnSaveDocumentationClicked() const
{
	UBlueprint* SelectedBlueprint = GetSelectedBlueprint();
	if (!SelectedBlueprint || GeneratedDocumentation.IsEmpty())
	{
		// Show error notification
		FText ErrorMessage = GeneratedDocumentation.IsEmpty()
			                     ? FText::FromString("Please generate documentation first.")
			                     : FText::FromString("Please select a valid Blueprint first.");

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
