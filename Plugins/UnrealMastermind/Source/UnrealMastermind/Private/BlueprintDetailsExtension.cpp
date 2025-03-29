// // Copyright YourCompany. All Rights Reserved.
//
// #include "BlueprintDetailsExtension.h"
// #include "BlueprintDocumentation.h"
// #include "DetailLayoutBuilder.h"
// #include "DetailCategoryBuilder.h"
// #include "DetailWidgetRow.h"
// #include "Widgets/Input/SButton.h"
// #include "Widgets/Text/STextBlock.h"
// #include "Framework/Application/SlateApplication.h"
// #include "Widgets/Layout/SScrollBox.h"
// #include "IDetailChildrenBuilder.h"
// #include "Engine/Blueprint.h"
//
// TSharedRef<IDetailCustomization> FBlueprintDetailsCustomization::MakeInstance()
// {
//     UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: FBlueprintDetailsCustomization::MakeInstance called"));
//     return MakeShareable(new FBlueprintDetailsCustomization());
// }
//
// void FBlueprintDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
// {
//     UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: FBlueprintDetailsCustomization::CustomizeDetails called"));
//     
//     // Get all selected objects
//     TArray<TWeakObjectPtr<UObject>> SelectedObjects;
//     DetailBuilder.GetObjectsBeingCustomized(SelectedObjects);
//     
//     // Log the selected objects for debugging
//     UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: CustomizeDetails called with %d objects"), SelectedObjects.Num());
//     
//     if (SelectedObjects.Num() != 1)
//     {
//         UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Multiple objects selected, not showing documentation tools"));
//         return;
//     }
//         
//     // Check if the selected object is a Blueprint
//     TWeakObjectPtr<UBlueprint> Blueprint = Cast<UBlueprint>(SelectedObjects[0].Get());
//     if (!Blueprint.IsValid())
//     {
//         UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Selected object is not a Blueprint, it's a %s"), 
//             *SelectedObjects[0]->GetClass()->GetName());
//         return;
//     }
//     
//     UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Customizing details for Blueprint %s"), *Blueprint->GetName());
//         
//     // Create a custom category for Unreal Mastermind
//     IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(
//         "Unreal Mastermind", 
//         FText::FromString("Unreal Mastermind"), 
//         ECategoryPriority::Important
//     );
//     
//     // Add our custom node builder
//     TSharedRef<FBlueprintDetailsExtension> Extension = MakeShareable(new FBlueprintDetailsExtension(Blueprint));
//     Category.AddCustomBuilder(Extension);
//     
//     UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Added custom builder to category"));
// }
//
// FBlueprintDetailsExtension::FBlueprintDetailsExtension(TWeakObjectPtr<UBlueprint> InBlueprint)
//     : Blueprint(InBlueprint)
// {
// }
//
// void FBlueprintDetailsExtension::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
// {
//     if (!Blueprint.IsValid())
//     {
//         UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Blueprint is not valid in GenerateHeaderRowContent"));
//         return;
//     }
//     
//     UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Generating header row for Blueprint %s"), *Blueprint->GetName());
//     
//     NodeRow
//     .NameContent()
//     [
//         SNew(STextBlock)
//         .Text(FText::FromString("AI Documentation"))
//         .Font(IDetailLayoutBuilder::GetDetailFont())
//     ]
//     .ValueContent()
//     .MinDesiredWidth(300.0f)
//     [
//         SNew(STextBlock)
//         .Text_Lambda([this]() -> FText {
//             return UBlueprintDocumentation::HasDocumentation(Blueprint.Get()) ? 
//                 FText::FromString("Documentation Available") : 
//                 FText::FromString("No Documentation Available");
//         })
//         .ColorAndOpacity_Lambda([this]() -> FSlateColor {
//             return UBlueprintDocumentation::HasDocumentation(Blueprint.Get()) ? 
//                 FLinearColor(0.0f, 1.0f, 0.0f, 1.0f) : 
//                 FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
//         })
//     ];
// }
//
// void FBlueprintDetailsExtension::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
// {
//     if (!Blueprint.IsValid())
//     {
//         UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Blueprint is not valid in GenerateChildContent"));
//         return;
//     }
//     
//     UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Generating child content for Blueprint %s"), *Blueprint->GetName());
//     
//     ChildrenBuilder.AddCustomRow(FText::FromString("Unreal Mastermind Actions"))
//     [
//         SNew(SHorizontalBox)
//         
//         +SHorizontalBox::Slot()
//         .AutoWidth()
//         .Padding(0, 0, 10, 0)
//         [
//             SNew(SButton)
//             .Text(FText::FromString("View Documentation"))
//             .IsEnabled_Lambda([this]() -> bool {
//                 return Blueprint.IsValid() && UBlueprintDocumentation::HasDocumentation(Blueprint.Get());
//             })
//             .OnClicked(this, &FBlueprintDetailsExtension::OnViewDocumentationClicked)
//         ]
//         
//         +SHorizontalBox::Slot()
//         .AutoWidth()
//         [
//             SNew(SButton)
//             .Text(FText::FromString("Generate Documentation"))
//             .OnClicked(this, &FBlueprintDetailsExtension::OnGenerateDocumentationClicked)
//         ]
//     ];
// }
//
// FReply FBlueprintDetailsExtension::OnViewDocumentationClicked()
// {
//     if (!Blueprint.IsValid())
//         return FReply::Handled();
//         
//     // Get the documentation
//     FString Documentation = UBlueprintDocumentation::GetDocumentation(Blueprint.Get());
//     
//     // Create a window to display the documentation
//     TSharedRef<SWindow> DocumentationWindow = SNew(SWindow)
//         .Title(FText::Format(FText::FromString("Documentation for {0}"), FText::FromString(Blueprint->GetName())))
//         .ClientSize(FVector2D(600, 400))
//         .SupportsMaximize(true)
//         .SupportsMinimize(true);
//         
//     DocumentationWindow->SetContent(
//         SNew(SScrollBox)
//         
//         +SScrollBox::Slot()
//         .Padding(10)
//         [
//             SNew(STextBlock)
//             .Text(FText::FromString(Documentation))
//             .AutoWrapText(true)
//         ]
//     );
//     
//     FSlateApplication::Get().AddWindow(DocumentationWindow);
//     
//     return FReply::Handled();
// }
//
// FReply FBlueprintDetailsExtension::OnGenerateDocumentationClicked()
// {
//     // Open the Unreal Mastermind window
//     FGlobalTabmanager::Get()->TryInvokeTab(FName("UnrealMastermind"));
//     
//     return FReply::Handled();
// }