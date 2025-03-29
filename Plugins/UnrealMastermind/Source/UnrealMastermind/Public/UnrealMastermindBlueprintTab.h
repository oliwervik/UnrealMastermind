// Copyright YourCompany. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Engine/Blueprint.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

class SUnrealMastermindBlueprintTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnrealMastermindBlueprintTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TWeakObjectPtr<UBlueprint> InBlueprint);
    
	// Updates the tab with the latest documentation
	void RefreshDocumentation();
	void OnTabClosed(TSharedRef<SDockTab> Tab);
	void OnActiveTabChanged(TSharedPtr<SDockTab> PreviouslyActive, TSharedPtr<SDockTab> NewlyActivated);
    
	// Sets the active blueprint
	void SetBlueprint(TWeakObjectPtr<UBlueprint> InBlueprint);

	// Delegate handle for tab activation
	FDelegateHandle TabActivationHandle;

private:
	// UI Elements
	TSharedPtr<SMultiLineEditableTextBox> DocumentationTextBox;
    
	// Button Callbacks
	FReply OnViewInWindowClicked();
	FReply OnGenerateDocumentationClicked();
	FReply OnSaveDocumentationClicked();
    
	// The blueprint being edited
	TWeakObjectPtr<UBlueprint> Blueprint;
    
	// Current documentation
	FString CurrentDocumentation;
};

class FUnrealMastermindBlueprintTabExtension
{
public:
	void Initialize();
	void Shutdown();
    
private:
	TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
	void OnTabClosed(TSharedRef<SDockTab> Tab);
    void UpdateActiveBlueprint();
    void OnAssetSelectionChanged(const TArray<FAssetData>& NewSelectedAssets, bool bIsPrimaryBrowser);
	void OnBlueprintEditorClosed(UBlueprint* InBlueprint);
	TSharedRef<FExtender> ExtendBlueprintEditorMenu(const TSharedRef<FUICommandList> CommandList, const TArray<UObject*> ContextSensitiveObjects);
	void AddAIDocumentationMenuItem(FMenuBuilder& MenuBuilder);
	void OpenDocumentationTab();
	
	// The tab ID
	static const FName TabID;
    
	// The currently active blueprint
	TWeakObjectPtr<UBlueprint> ActiveBlueprint;
    
	// The tab widget
	TSharedPtr<SUnrealMastermindBlueprintTab> TabWidget;
    
	// Delegate handles
	FDelegateHandle BlueprintEditorOpenedHandle;
	FDelegateHandle BlueprintEditorClosedHandle;
	
};