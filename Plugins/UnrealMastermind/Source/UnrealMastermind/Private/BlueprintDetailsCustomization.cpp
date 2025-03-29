#include "BlueprintDetailsExtension.h"
#include "BlueprintDocumentation.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "UnrealMastermindTab.h"

#define LOCTEXT_NAMESPACE "UnrealMastermindBlueprintDetails"

// Static member initialization
bool FBlueprintDetailsCustomization::bIsRegistered = false;

void FBlueprintDetailsCustomization::Register()
{
    if (!bIsRegistered)
    {
        FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
        
        // Register for all relevant classes
        TArray<UClass*> ClassesToCustomize;
        ClassesToCustomize.Add(UBlueprint::StaticClass());
        ClassesToCustomize.Add(UBlueprintGeneratedClass::StaticClass());
        
        for (UClass* Class : ClassesToCustomize)
        {
            PropertyModule.RegisterCustomClassLayout(
                Class->GetFName(),
                FOnGetDetailCustomizationInstance::CreateStatic(&FBlueprintDetailsCustomization::MakeInstance)
            );
            UE_LOG(LogTemp, Warning, TEXT("Registered customization for: %s"), *Class->GetName());
        }
        
        bIsRegistered = true;
        PropertyModule.NotifyCustomizationModuleChanged();
    }
}

// Unregister the customization
void FBlueprintDetailsCustomization::Unregister()
{
    if (bIsRegistered)
    {
        FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.UnregisterCustomClassLayout(UBlueprint::StaticClass()->GetFName());
        bIsRegistered = false;
        UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Unregistered Blueprint details customization"));
    }
}

TSharedRef<IDetailCustomization> FBlueprintDetailsCustomization::MakeInstance()
{
    return MakeShareable(new FBlueprintDetailsCustomization);
}

void FBlueprintDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    UE_LOG(LogTemp, Warning, TEXT("### CUSTOMIZE DETAILS FINALLY CALLED ###"));
    UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Customizing Blueprint details"));
    
    // Get all selected objects
    TArray<TWeakObjectPtr<UObject>> SelectedObjects;
    DetailBuilder.GetObjectsBeingCustomized(SelectedObjects);
    
    UE_LOG(LogTemp, Warning, TEXT("Found %d objects in details panel"), SelectedObjects.Num());
    
    for (TWeakObjectPtr<UObject> Object : SelectedObjects)
    {
        if (Object.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("Object in details: %s (Class: %s)"), 
                *Object->GetName(), 
                *Object->GetClass()->GetName());
                
            if (Object->IsA<UBlueprint>())
            {
                SelectedBlueprint = Cast<UBlueprint>(Object.Get());
                UE_LOG(LogTemp, Warning, TEXT("Found UBlueprint: %s"), *SelectedBlueprint->GetName());
            }
            else if (Object->IsA<UBlueprintGeneratedClass>())
            {
                UE_LOG(LogTemp, Warning, TEXT("Found UBlueprintGeneratedClass: %s"), *Object->GetName());
                // Try to get the blueprint from the generated class
                UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(Object.Get());
                SelectedBlueprint = Cast<UBlueprint>(BPClass->ClassGeneratedBy);
                if (SelectedBlueprint.IsValid())
                {
                    UE_LOG(LogTemp, Warning, TEXT("Found parent UBlueprint: %s"), *SelectedBlueprint->GetName());
                }
            }
        }
    }
    
    // If no blueprint found, exit
    if (!SelectedBlueprint.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: No valid Blueprint found in details view"));
        return;
    }
    
    // Create a custom category
    IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(
        "AI Documentation",
        FText::FromString("AI Documentation"),
        ECategoryPriority::Important
    );
    
    // Add a custom row for the documentation status
    Category.AddCustomRow(FText::FromString("Documentation Status"))
    .NameContent()
    [
        SNew(STextBlock)
        .Text(FText::FromString("Documentation Status"))
        .Font(IDetailLayoutBuilder::GetDetailFont())
    ]
    .ValueContent()
    .MinDesiredWidth(300.0f)
    [
        SNew(STextBlock)
        .Text_Lambda([this]() -> FText {
            return HasDocumentation() ? 
                FText::FromString("Available") : 
                FText::FromString("Not Available");
        })
        .ColorAndOpacity_Lambda([this]() -> FSlateColor {
            return HasDocumentation() ? 
                FSlateColor(FLinearColor(0.0f, 0.8f, 0.0f)) : 
                FSlateColor(FLinearColor(0.8f, 0.0f, 0.0f));
        })
        .Font(IDetailLayoutBuilder::GetDetailFont())
    ];
    
    // Add buttons row
    Category.AddCustomRow(FText::FromString("AI Documentation Actions"))
    .WholeRowContent()
    [
        SNew(SHorizontalBox)
        
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        .Padding(5.0f)
        [
            SNew(SButton)
            .Text(FText::FromString("View Documentation"))
            .IsEnabled_Lambda([this]() -> bool {
                return HasDocumentation();
            })
            .OnClicked(this, &FBlueprintDetailsCustomization::OnViewDocumentationClicked)
            .HAlign(HAlign_Center)
        ]
        
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        .Padding(5.0f)
        [
            SNew(SButton)
            .Text(FText::FromString("Generate Documentation"))
            .IsEnabled_Lambda([this]() -> bool {
                return SelectedBlueprint.IsValid();
            })
            .OnClicked(this, &FBlueprintDetailsCustomization::OnGenerateDocumentationClicked)
            .HAlign(HAlign_Center)
        ]
    ];
    
    // If documentation exists, show a preview
    if (HasDocumentation())
    {
        Category.AddCustomRow(FText::FromString("Documentation Preview"))
        .WholeRowContent()
        [
            SNew(SBox)
            .Padding(FMargin(10.0f))
            [
                SNew(SBorder)
                .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                .Padding(FMargin(4.0f))
                [
                    SNew(STextBlock)
                    .Text_Lambda([this]() -> FText {
                        FString DocText = GetDocumentation();
                        // Limit preview to first 200 characters
                        if (DocText.Len() > 200)
                        {
                            DocText = DocText.Left(200) + TEXT("...");
                        }
                        return FText::FromString(DocText);
                    })
                    .AutoWrapText(true)
                ]
            ]
        ];
    }
}

FReply FBlueprintDetailsCustomization::OnViewDocumentationClicked()
{
    if (!SelectedBlueprint.IsValid() || !HasDocumentation())
        return FReply::Handled();
    
    // Open the documentation viewer window
    TSharedPtr<SDockTab> DocTab = FGlobalTabmanager::Get()->TryInvokeTab(FName("UnrealMastermind"));
    if (DocTab.IsValid())
    {
        // Get the tab content
        TSharedRef<SUnrealMastermindTab> MastermindTab = 
            StaticCastSharedRef<SUnrealMastermindTab>(DocTab->GetContent());
        
        // Select the current blueprint and view docs
        MastermindTab.Get().SelectBlueprint(SelectedBlueprint->GetName());
        
        UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Viewing documentation for blueprint: %s"), 
            *SelectedBlueprint->GetName());
    }
    
    return FReply::Handled();
}

FReply FBlueprintDetailsCustomization::OnGenerateDocumentationClicked()
{
    if (!SelectedBlueprint.IsValid())
        return FReply::Handled();
    
    // Open the documentation generation window
    TSharedPtr<SDockTab> DocTab = FGlobalTabmanager::Get()->TryInvokeTab(FName("UnrealMastermind"));
    if (DocTab.IsValid())
    {
        // Get the tab content
        TSharedRef<SUnrealMastermindTab> MastermindTab = 
            StaticCastSharedRef<SUnrealMastermindTab>(DocTab->GetContent());
        
        // Select the current blueprint
        MastermindTab.Get().SelectBlueprint(SelectedBlueprint->GetName());
        
        UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Generating documentation for blueprint: %s"), 
            *SelectedBlueprint->GetName());
    }
    
    return FReply::Handled();
}

bool FBlueprintDetailsCustomization::HasDocumentation() const
{
    if (!SelectedBlueprint.IsValid())
        return false;
    
    return UBlueprintDocumentation::HasDocumentation(SelectedBlueprint.Get());
}

FString FBlueprintDetailsCustomization::GetDocumentation() const
{
    if (!SelectedBlueprint.IsValid())
        return FString();
    
    return UBlueprintDocumentation::GetDocumentation(SelectedBlueprint.Get());
}

#undef LOCTEXT_NAMESPACE