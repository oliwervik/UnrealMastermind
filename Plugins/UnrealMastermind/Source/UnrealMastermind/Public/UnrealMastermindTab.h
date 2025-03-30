// Copyright © Froströk. All Rights Reserved.
// This plugin is governed by the Unreal Engine Marketplace EULA.
// This software cannot be redistributed, modified, or resold outside of the original purchase.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintDocumentationSettings.h"
#include "Widgets/SCompoundWidget.h"
#include "EdGraph/EdGraphNode.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"

class SUnrealMastermindTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnrealMastermindTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	// Update loading status
	void SetGenerationStatus(bool bGenerating, float Progress = 0.0f, const FString& StatusMessage = TEXT(""));
	void SelectBlueprint(const FString& BlueprintName);
private:
	// UI Elements
	TSharedPtr<SComboBox<TSharedPtr<FString>>> BlueprintSelectionComboBox;
	TArray<TSharedPtr<FString>> AvailableBlueprints;
	TSharedPtr<STextBlock> SelectedBlueprintText;
	TSharedPtr<SMultiLineEditableTextBox> DocumentationTextBox;
	TSharedPtr<SEditableTextBox> CustomPromptTextBox;
	TArray<TSharedPtr<FString>> DetailLevelOptions;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> DetailLevelComboBox;
	
	// Loading indicator elements
	TSharedPtr<SProgressBar> GenerationProgressBar;
	TSharedPtr<STextBlock> GenerationStatusText;
	TSharedPtr<SVerticalBox> LoadingIndicatorBox;
	
	// Button Callbacks
	FReply OnGenerateDocumentationClicked();
	FReply OnSaveDocumentationClicked() const;
	
	// Blueprint Functions
	void PopulateAvailableBlueprints();
	void OnBlueprintSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);
	TSharedRef<SWidget> MakeBlueprintComboItemWidget(TSharedPtr<FString> BlueprintName);
	UBlueprint* GetSelectedBlueprint() const;
	void ExtractEventGraphInfo(UBlueprint* Blueprint, const FBlueprintDocumentationSettings& Settings,
	                           FString& BlueprintInfo);
	void ExtractFunctionGraphInfo(UBlueprint* Blueprint, const FBlueprintDocumentationSettings& Settings,
	                              FString& BlueprintInfo);
	void ExtractVariableInfo(UBlueprint* Blueprint, FString& BlueprintInfo) const;
	FString ExtractBlueprintInfo(UBlueprint* Blueprint, const FBlueprintDocumentationSettings& Settings);
	void AddImportantComponentProperties(FString& BlueprintInfo, UActorComponent* Component, const USCS_Node* Node) const;
	void TraceExecutionFlow(UEdGraphPin* ExecPin, FString& BlueprintInfo, int32 Depth, int32 MaxDepth);
	FString GetPinValue(UEdGraphPin* Pin) const;

	// Documentation Generation
	TSharedPtr<FString> CurrentSelectedBlueprint;
	FString GeneratedDocumentation;

	void AddAllComponentProperties(FString& BlueprintInfo, UActorComponent* Component, const USCS_Node* Node, const TArray<FString>& IgnoredPrefixes) const;
	
	// Generation status
	bool bIsGenerating;
};