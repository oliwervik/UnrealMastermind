#include "..\Public\BlueprintDetailsCustomization.h"
#include "BlueprintDocumentation.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "UnrealMastermindSettings.h"
#include "UnrealMastermindTab.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"

#define LOCTEXT_NAMESPACE "UnrealMastermindBlueprintDetails"

// Static member initialization
bool FBlueprintDetailsCustomization::bIsRegistered = false;

void FBlueprintDetailsCustomization::Register()
{
    if (!bIsRegistered)
    {
        FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
       
        //Register UBlueprint class itself (for Blueprint assets in the editor)
        PropertyModule.RegisterCustomClassLayout(
            UBlueprint::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FBlueprintDetailsCustomization::MakeInstance)
        );
        
        //Register important base classes that are commonly used as Blueprint parents
        TArray<UClass*> BlueprintBaseClasses;
        BlueprintBaseClasses.Add(AActor::StaticClass());
        BlueprintBaseClasses.Add(UActorComponent::StaticClass());
        BlueprintBaseClasses.Add(APawn::StaticClass());
        BlueprintBaseClasses.Add(ACharacter::StaticClass());
        BlueprintBaseClasses.Add(UUserWidget::StaticClass());
        
        //We can add other important base classes here if needed
        
        for (UClass* BaseClass : BlueprintBaseClasses)
        {
            PropertyModule.RegisterCustomClassLayout(
                BaseClass->GetFName(),
                FOnGetDetailCustomizationInstance::CreateStatic(&FBlueprintDetailsCustomization::MakeInstance)
            );
        }
        
        //Register all Blueprint-generated classes
        for (TObjectIterator<UClass> It; It; ++It)
        {
            UClass* Class = *It;
            
            // Check if this is a Blueprint-generated class
            if (Class && Class->ClassGeneratedBy && Cast<UBlueprint>(Class->ClassGeneratedBy))
            {
                PropertyModule.RegisterCustomClassLayout(
                    Class->GetFName(),
                    FOnGetDetailCustomizationInstance::CreateStatic(&FBlueprintDetailsCustomization::MakeInstance)
                );
            }
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
        if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
        {
            FPropertyEditorModule& PropertyModule = 
                FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

            for (TObjectIterator<UClass> It; It; ++It)
            {
                UClass* Class = *It;
                if (Class)
                {
                    PropertyModule.UnregisterCustomClassLayout(Class->GetFName());
                }
            }
        }
        bIsRegistered = false;
    }
}

TSharedRef<IDetailCustomization> FBlueprintDetailsCustomization::MakeInstance()
{
    return MakeShareable(new FBlueprintDetailsCustomization);
}

void FBlueprintDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    // Get all selected objects
    TArray<TWeakObjectPtr<UObject>> SelectedObjects;
    DetailBuilder.GetObjectsBeingCustomized(SelectedObjects);
    
    for (TWeakObjectPtr<UObject> Object : SelectedObjects)
    {
        if (Object.IsValid())
        {
            // With this more comprehensive approach:
            UObject* ObjectPtr = Object.Get();
            
            // Direct approach - is it already a UBlueprint?
            SelectedBlueprint = Cast<UBlueprint>(ObjectPtr);
            
            // If not a direct blueprint, check if it's a blueprint class instance
            if (!SelectedBlueprint.IsValid())
            {
                if (const UClass* Class = ObjectPtr->GetClass(); Class && Class->ClassGeneratedBy)
                {
                    SelectedBlueprint = Cast<UBlueprint>(Class->ClassGeneratedBy);
                }
            }
        }
    }
    
    // If no blueprint found, exit
    if (!SelectedBlueprint.IsValid())
    {
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
        const UUnrealMastermindSettings* Settings = GetDefault<UUnrealMastermindSettings>();
        
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
                    .Text_Lambda([this, Settings]() -> FText {
                        FString DocText = GetDocumentation();
                        int32 MaxChars = Settings->DocumentationPreviewChars;
                        if (DocText.Len() > MaxChars)
                        {
                            DocText = DocText.Left(MaxChars) + TEXT("...");
                        }
                        return FText::FromString(DocText);
                    })
                    .AutoWrapText(true)
                ]
            ]
        ];
    }
}

FReply FBlueprintDetailsCustomization::OnViewDocumentationClicked() const
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
    }
    
    return FReply::Handled();
}

FReply FBlueprintDetailsCustomization::OnGenerateDocumentationClicked() const
{
    if (!SelectedBlueprint.IsValid())
        return FReply::Handled();
    
    // Open the documentation generation window
    if (const TSharedPtr<SDockTab> DocTab = FGlobalTabmanager::Get()->TryInvokeTab(FName("UnrealMastermind")); DocTab.IsValid())
    {
        // Get the tab content
        const TSharedRef<SUnrealMastermindTab> MastermindTab = 
            StaticCastSharedRef<SUnrealMastermindTab>(DocTab->GetContent());
        
        // Select the current blueprint
        MastermindTab.Get().SelectBlueprint(SelectedBlueprint->GetName());
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