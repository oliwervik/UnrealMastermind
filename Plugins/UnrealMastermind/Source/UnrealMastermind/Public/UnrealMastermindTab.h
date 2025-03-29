// Copyright Froströk. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "EdGraph/EdGraphNode.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
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
	
	// Loading indicator elements
	TSharedPtr<SProgressBar> GenerationProgressBar;
	TSharedPtr<STextBlock> GenerationStatusText;
	TSharedPtr<SVerticalBox> LoadingIndicatorBox;
	
	// Button Callbacks
	FReply OnGenerateDocumentationClicked();
	FReply OnSaveDocumentationClicked();
	
	// Blueprint Functions
	void PopulateAvailableBlueprints();
	void OnBlueprintSelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);
	TSharedRef<SWidget> MakeBlueprintComboItemWidget(TSharedPtr<FString> BlueprintName);
	UBlueprint* GetSelectedBlueprint() const;
	FString ExtractBlueprintInfo(UBlueprint* Blueprint);
	
	// Documentation Generation
	TSharedPtr<FString> CurrentSelectedBlueprint;
	FString GeneratedDocumentation;
	
	// Generation status
	bool bIsGenerating;
};