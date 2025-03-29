// Copyright YourCompany. All Rights Reserved.

#include "UnrealMastermindBlueprintTab.h"
#include "BlueprintDocumentation.h"
#include "BlueprintEditorModule.h"
#include "ContentBrowserModule.h"
#include "UnrealMastermindTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/Commands/UIAction.h"

#define LOCTEXT_NAMESPACE "UnrealMastermindBlueprintTab"

// Define the tab ID
const FName FUnrealMastermindBlueprintTabExtension::TabID = FName("UnrealMastermindDocumentation");

void SUnrealMastermindBlueprintTab::Construct(const FArguments& InArgs, TWeakObjectPtr<UBlueprint> InBlueprint)
{
    Blueprint = InBlueprint;
    TabActivationHandle = FDelegateHandle();
    
    // Load existing documentation if available
    RefreshDocumentation();
    
    ChildSlot
    [
        SNew(SVerticalBox)
        
        // Header
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(10)
        .HAlign(HAlign_Center)
        [
            SNew(STextBlock)
            .Text(FText::FromString("Unreal Mastermind - AI Documentation"))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
        ]
        
        // Status
        +SVerticalBox::Slot()
        .AutoHeight()
        .Padding(10, 0)
        [
            SNew(STextBlock)
            .Text_Lambda([this]() -> FText {
                if (!Blueprint.IsValid())
                    return FText::FromString("No Blueprint Selected");
                
                return UBlueprintDocumentation::HasDocumentation(Blueprint.Get()) ?
                    FText::FromString("Documentation Available") :
                    FText::FromString("No Documentation Available");
            })
            .ColorAndOpacity_Lambda([this]() -> FSlateColor {
                if (!Blueprint.IsValid())
                    return FSlateColor(FLinearColor::Red);
                
                return UBlueprintDocumentation::HasDocumentation(Blueprint.Get()) ?
                    FSlateColor(FLinearColor::Green) :
                    FSlateColor(FLinearColor::Red);
            })
        ]
        
        // Documentation
        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(10)
        [
            SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
            .Padding(4)
            [
                SAssignNew(DocumentationTextBox, SMultiLineEditableTextBox)
                .Text(FText::FromString(CurrentDocumentation))
                .AutoWrapText(true)
                .ModiferKeyForNewLine(EModifierKey::Shift)
                .HintText(FText::FromString("Documentation will appear here..."))
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
                .OnClicked(this, &SUnrealMastermindBlueprintTab::OnGenerateDocumentationClicked)
            ]
            
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .Padding(5, 0, 5, 0)
            [
                SNew(SButton)
                .HAlign(HAlign_Center)
                .Text(FText::FromString("Save Documentation"))
                .OnClicked(this, &SUnrealMastermindBlueprintTab::OnSaveDocumentationClicked)
                .IsEnabled_Lambda([this]() -> bool {
                    return Blueprint.IsValid() && !DocumentationTextBox->GetText().IsEmpty();
                })
            ]
            
            +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .Padding(5, 0, 0, 0)
            [
                SNew(SButton)
                .HAlign(HAlign_Center)
                .Text(FText::FromString("View In Window"))
                .OnClicked(this, &SUnrealMastermindBlueprintTab::OnViewInWindowClicked)
                .IsEnabled_Lambda([this]() -> bool {
                    return Blueprint.IsValid() && !DocumentationTextBox->GetText().IsEmpty();
                })
            ]
        ]
    ];
}

void SUnrealMastermindBlueprintTab::RefreshDocumentation()
{
    if (!Blueprint.IsValid())
    {
        CurrentDocumentation = TEXT("");
        if (DocumentationTextBox.IsValid())
        {
            DocumentationTextBox->SetText(FText::FromString(TEXT("")));
        }
        return;
    }
    
    // Check if documentation exists
    if (UBlueprintDocumentation::HasDocumentation(Blueprint.Get()))
    {
        // Load the documentation
        CurrentDocumentation = UBlueprintDocumentation::GetDocumentation(Blueprint.Get());
        
        UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Loaded documentation, length: %d"), CurrentDocumentation.Len());
        
        if (DocumentationTextBox.IsValid())
        {
            DocumentationTextBox->SetText(FText::FromString(CurrentDocumentation));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: No documentation found"));
        
        CurrentDocumentation = TEXT("");
        if (DocumentationTextBox.IsValid())
        {
            DocumentationTextBox->SetText(FText::FromString(TEXT("")));
        }
    }
}

void SUnrealMastermindBlueprintTab::SetBlueprint(TWeakObjectPtr<UBlueprint> InBlueprint)
{
    if (Blueprint != InBlueprint)
    {
        Blueprint = InBlueprint;
        RefreshDocumentation();
    }
}

FReply SUnrealMastermindBlueprintTab::OnViewInWindowClicked()
{
    if (!Blueprint.IsValid())
        return FReply::Handled();
    
    FString Documentation = DocumentationTextBox->GetText().ToString();
    if (Documentation.IsEmpty())
        return FReply::Handled();
    
    // Create a window to display the documentation
    TSharedRef<SWindow> DocumentationWindow = SNew(SWindow)
        .Title(FText::Format(LOCTEXT("DocumentationWindowTitle", "Documentation for {0}"), FText::FromString(Blueprint->GetName())))
        .ClientSize(FVector2D(600, 400))
        .SupportsMaximize(true)
        .SupportsMinimize(true);
        
    DocumentationWindow->SetContent(
        SNew(SScrollBox)
        +SScrollBox::Slot()
        .Padding(10)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Documentation))
            .AutoWrapText(true)
        ]
    );
    
    FSlateApplication::Get().AddWindow(DocumentationWindow);
    
    return FReply::Handled();
}

FReply SUnrealMastermindBlueprintTab::OnGenerateDocumentationClicked()
{
    if (!Blueprint.IsValid())
        return FReply::Handled();
    
    // Open the Unreal Mastermind window
    TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->TryInvokeTab(FName("UnrealMastermind"));
    
    // If the tab opened successfully, select the current blueprint
    if (Tab.IsValid())
    {
        // Get the tab content
        TSharedRef<SUnrealMastermindTab> MastermindTab = 
            StaticCastSharedRef<SUnrealMastermindTab>(Tab->GetContent());
        
        // Select the current blueprint by name
        MastermindTab.Get().SelectBlueprint(Blueprint->GetName());
        
        UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Opened main tab and selected blueprint: %s"), 
            *Blueprint->GetName());
    }
    
    return FReply::Handled();
}

FReply SUnrealMastermindBlueprintTab::OnSaveDocumentationClicked()
{
    if (!Blueprint.IsValid())
        return FReply::Handled();
    
    FString Documentation = DocumentationTextBox->GetText().ToString();
    if (Documentation.IsEmpty())
        return FReply::Handled();
    
    UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Saving documentation for %s, length: %d"), 
        *Blueprint->GetName(), Documentation.Len());
    
    // Save the documentation
    bool bSaved = UBlueprintDocumentation::SaveDocumentation(Blueprint.Get(), Documentation);
    
    if (bSaved)
    {
        // Show success notification
        FNotificationInfo SuccessInfo(FText::FromString("Documentation saved successfully!"));
        SuccessInfo.ExpireDuration = 3.0f;
        FSlateNotificationManager::Get().AddNotification(SuccessInfo);
        
        UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Documentation saved successfully"));
    }
    else
    {
        // Show error notification
        FNotificationInfo ErrorInfo(FText::FromString("Failed to save documentation. Check Output Log for details."));
        ErrorInfo.ExpireDuration = 5.0f;
        FSlateNotificationManager::Get().AddNotification(ErrorInfo);
        
        UE_LOG(LogTemp, Error, TEXT("Unreal Mastermind: Failed to save documentation"));
    }
    
    return FReply::Handled();
}

void FUnrealMastermindBlueprintTabExtension::Initialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Initializing Blueprint Tab Extension"));
    
    // Register tab spawner
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        TabID,
        FOnSpawnTab::CreateRaw(this, &FUnrealMastermindBlueprintTabExtension::SpawnTab)
    )
    .SetDisplayName(LOCTEXT("TabTitle", "AI Documentation"))
    .SetTooltipText(LOCTEXT("TabTooltip", "View and manage AI-generated documentation for Blueprints"));
    
    // Register for asset selection changes if module is loaded
    if (FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
    {
        FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
        ContentBrowserModule.GetOnAssetSelectionChanged().AddRaw(this, &FUnrealMastermindBlueprintTabExtension::OnAssetSelectionChanged);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Blueprint Tab Extension initialized"));
}


TSharedRef<FExtender> FUnrealMastermindBlueprintTabExtension::ExtendBlueprintEditorMenu(const TSharedRef<FUICommandList> CommandList, const TArray<UObject*> ContextSensitiveObjects)
{
    TSharedRef<FExtender> Extender(new FExtender());
    
    if (ContextSensitiveObjects.Num() > 0)
    {
        UBlueprint* Blueprint = Cast<UBlueprint>(ContextSensitiveObjects[0]);
        if (Blueprint)
        {
            ActiveBlueprint = Blueprint;
            
            // Add a menu entry to the View menu
            Extender->AddMenuExtension(
                "ViewOptions",
                EExtensionHook::After,
                CommandList,
                FMenuExtensionDelegate::CreateRaw(this, &FUnrealMastermindBlueprintTabExtension::AddAIDocumentationMenuItem)
            );
            
            UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Extended Blueprint editor menu for %s"), *Blueprint->GetName());
        }
    }
    
    return Extender;
}

void FUnrealMastermindBlueprintTabExtension::AddAIDocumentationMenuItem(FMenuBuilder& MenuBuilder)
{
    MenuBuilder.AddMenuEntry(
        LOCTEXT("AIDocumentationTabTitle", "Show AI Documentation"),
        LOCTEXT("AIDocumentationTabTooltip", "Opens the AI Documentation tab for this Blueprint"),
        FSlateIcon(),
        FUIAction(
            FExecuteAction::CreateRaw(this, &FUnrealMastermindBlueprintTabExtension::OpenDocumentationTab)
        )
    );
}

void FUnrealMastermindBlueprintTabExtension::OpenDocumentationTab()
{
    UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Opening AI Documentation tab"));
    
    // Open the tab
    TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->TryInvokeTab(TabID);
    
    // Update the tab with the current blueprint if needed
    if (Tab.IsValid() && TabWidget.IsValid() && ActiveBlueprint.IsValid())
    {
        TabWidget->SetBlueprint(ActiveBlueprint);
    }
}

void SUnrealMastermindBlueprintTab::OnActiveTabChanged(TSharedPtr<SDockTab> PreviouslyActive, TSharedPtr<SDockTab> NewlyActivated)
{
    // Check if our tab is the newly activated one
    if (NewlyActivated.IsValid() && NewlyActivated->GetContent() == AsShared())
    {
        UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Blueprint documentation tab activated"));
        
        // Try to find the currently active Blueprint using multiple methods
        UBlueprint* ActiveBlueprint = nullptr;
        
        // First look for blueprints that are open in editors
        UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
        if (AssetEditorSubsystem)
        {
            // Get all open editor assets
            TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();
            
            UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Found %d edited assets"), EditedAssets.Num());
            
            // Check each one
            for (UObject* EditedAsset : EditedAssets)
            {
                if (EditedAsset && EditedAsset->IsA(UBlueprint::StaticClass()))
                {
                    UBlueprint* BP = Cast<UBlueprint>(EditedAsset);
                    if (BP)
                    {
                        ActiveBlueprint = BP;
                        UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Found blueprint from editor: %s"), 
                            *ActiveBlueprint->GetName());
                        break;
                    }
                }
            }
        }
        
        // If no active editor found, try content browser selection
        if (!ActiveBlueprint)
        {
            TArray<FAssetData> SelectedAssets;
            GEditor->GetContentBrowserSelections(SelectedAssets);
            
            for (const FAssetData& Asset : SelectedAssets)
            {
                if (Asset.GetClass() == UBlueprint::StaticClass() || 
                    Asset.AssetClassPath == UBlueprint::StaticClass()->GetClassPathName())
                {
                    UObject* AssetObj = Asset.GetAsset();
                    if (AssetObj && AssetObj->IsA(UBlueprint::StaticClass()))
                    {
                        ActiveBlueprint = Cast<UBlueprint>(AssetObj);
                        if (ActiveBlueprint)
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Found blueprint from content browser: %s"), 
                                *ActiveBlueprint->GetName());
                            break;
                        }
                    }
                }
            }
        }
        
        // If we found a different active blueprint, switch to it
        if (ActiveBlueprint && (!Blueprint.IsValid() || Blueprint.Get() != ActiveBlueprint))
        {
            UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Switching to newly detected active blueprint: %s"), 
                *ActiveBlueprint->GetName());
            Blueprint = ActiveBlueprint;
        }
        
        // Refresh the documentation display
        RefreshDocumentation();
    }
}

void FUnrealMastermindBlueprintTabExtension::Shutdown()
{
    UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Shutting down Blueprint Tab Extension"));
    
    // Close the tab if it's open to prevent crashes
    TSharedPtr<SDockTab> DocumentationTab = FGlobalTabmanager::Get()->FindExistingLiveTab(TabID);
    if (DocumentationTab.IsValid())
    {
        DocumentationTab->RequestCloseTab();
    }
    
    // Reset the tab widget
    if (TabWidget.IsValid())
    {
        TabWidget.Reset();
    }
    
    // Clear the active blueprint reference
    ActiveBlueprint = nullptr;
    
    // Unregister tab spawner
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabID);
    
    // Unregister event handlers
    if (FModuleManager::Get().IsModuleLoaded("Kismet"))
    {
        FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::GetModuleChecked<FBlueprintEditorModule>("Kismet");
        BlueprintEditorModule.OnBlueprintEditorOpened().Remove(BlueprintEditorOpenedHandle);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Blueprint Tab Extension shutdown"));
}

TSharedRef<SDockTab> FUnrealMastermindBlueprintTabExtension::SpawnTab(const FSpawnTabArgs& Args)
{
    UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Spawning Blueprint tab"));
    
    TSharedRef<SDockTab> Tab = SNew(SDockTab)
        .TabRole(ETabRole::NomadTab).OnTabClosed_Raw(this, &FUnrealMastermindBlueprintTabExtension::OnTabClosed)
        .Label(LOCTEXT("TabTitle", "AI Documentation"));
    
    TabWidget = SNew(SUnrealMastermindBlueprintTab, ActiveBlueprint);
    
    TabWidget->TabActivationHandle = FGlobalTabmanager::Get()->OnActiveTabChanged_Subscribe(
        FOnActiveTabChanged::FDelegate::CreateSP(TabWidget.ToSharedRef(), &SUnrealMastermindBlueprintTab::OnActiveTabChanged)
    );

    Tab->SetContent(TabWidget.ToSharedRef());
    
    return Tab;
}

void FUnrealMastermindBlueprintTabExtension::OnTabClosed(TSharedRef<SDockTab> Tab)
{
    // Clean up when the tab is closed
    if (TabWidget.IsValid())
    {
        TabWidget->OnTabClosed(Tab);
        TabWidget.Reset();
    }
    
    ActiveBlueprint = nullptr;
}

void SUnrealMastermindBlueprintTab::OnTabClosed(TSharedRef<SDockTab> Tab)
{
    // Unsubscribe from tab change notifications using the stored handle
    if (TabActivationHandle.IsValid())
    {
        FGlobalTabmanager::Get()->OnActiveTabChanged_Unsubscribe(TabActivationHandle);
        TabActivationHandle.Reset();
    }
    
    Blueprint = nullptr;
    CurrentDocumentation.Empty();
}

void FUnrealMastermindBlueprintTabExtension::UpdateActiveBlueprint()
{
    // Try to find a blueprint from current selection
    TArray<FAssetData> SelectedAssets;
    GEditor->GetContentBrowserSelections(SelectedAssets);
    
    UBlueprint* FoundBlueprint = nullptr;
    for (const FAssetData& Asset : SelectedAssets)
    {
        if (Asset.AssetClassPath == UBlueprint::StaticClass()->GetClassPathName())
        {
            FoundBlueprint = Cast<UBlueprint>(Asset.GetAsset());
            if (FoundBlueprint)
            {
                break;
            }
        }
    }
    
    if (FoundBlueprint)
    {
        UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Active blueprint updated to %s"), *FoundBlueprint->GetName());
        
        ActiveBlueprint = FoundBlueprint;
        
        // Update the tab widget if it exists
        if (TabWidget.IsValid())
        {
            TabWidget->SetBlueprint(ActiveBlueprint);
        }
    }
}

void FUnrealMastermindBlueprintTabExtension::OnBlueprintEditorClosed(UBlueprint* InBlueprint)
{
    UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Blueprint editor closed for %s"), *InBlueprint->GetName());
    
    if (ActiveBlueprint.Get() == InBlueprint)
    {
        ActiveBlueprint = nullptr;
        
        // Update the tab widget if it exists
        if (TabWidget.IsValid())
        {
            TabWidget->SetBlueprint(nullptr);
        }
    }
}

void FUnrealMastermindBlueprintTabExtension::OnAssetSelectionChanged(const TArray<FAssetData>& NewSelectedAssets, bool bIsPrimaryBrowser)
{
    // Only process events from the primary browser
    if (!bIsPrimaryBrowser)
        return;
        
    for (const FAssetData& Asset : NewSelectedAssets)
    {
        if (Asset.AssetClassPath == UBlueprint::StaticClass()->GetClassPathName())
        {
            UBlueprint* Blueprint = Cast<UBlueprint>(Asset.GetAsset());
            if (Blueprint)
            {
                UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Asset selection changed to Blueprint %s"), *Blueprint->GetName());
                
                ActiveBlueprint = Blueprint;
                
                // Update the tab widget if it exists
                if (TabWidget.IsValid())
                {
                    TabWidget->SetBlueprint(ActiveBlueprint);
                }
                
                break;
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE