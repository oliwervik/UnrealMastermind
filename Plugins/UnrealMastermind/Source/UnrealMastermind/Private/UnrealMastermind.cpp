// Copyright Froströk. All Rights Reserved.

#include "UnrealMastermind.h"

#include "UnrealMastermindStyle.h"
#include "UnrealMastermindCommands.h"
#include "UnrealMastermindTab.h"
#include "Widgets/Docking/SDockTab.h"
#include "ToolMenus.h"
#include "PropertyEditorModule.h"
#include "Selection.h"
#include "BlueprintDetailsExtension.h"
#include "UnrealMastermindBlueprintTab.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

static const FName UnrealMastermindTabName("UnrealMastermind");

#define LOCTEXT_NAMESPACE "FUnrealMastermindModule"

void FUnrealMastermindModule::StartupModule()
{
	// Initialize the style set
	FUnrealMastermindStyle::Initialize();
	FUnrealMastermindStyle::ReloadTextures();

	FUnrealMastermindCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FUnrealMastermindCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FUnrealMastermindModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUnrealMastermindModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(UnrealMastermindTabName, FOnSpawnTab::CreateRaw(this, &FUnrealMastermindModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FUnrealMastermindTabTitle", "Unreal Mastermind"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	
	UE_LOG(LogTemp, Warning, TEXT("UnrealMastermind module starting up"));

	// Check if PropertyEditor is loaded
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		UE_LOG(LogTemp, Warning, TEXT("PropertyEditor is loaded, registering customizations"));
		FBlueprintDetailsCustomization::Register();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PropertyEditor not loaded yet, deferring registration"));
		FModuleManager::Get().OnModulesChanged().AddRaw(this, &FUnrealMastermindModule::HandleModulesChanged);
	}
    
	// Initialize Blueprint tab extension
	BlueprintTabExtension = MakeShareable(new FUnrealMastermindBlueprintTabExtension());
	BlueprintTabExtension->Initialize();
    
	UE_LOG(LogTemp, Warning, TEXT("Unreal Mastermind: Module loaded with Blueprint tab extension"));
}

void FUnrealMastermindModule::HandleModulesChanged(FName ModuleName, EModuleChangeReason Reason)
{
	if (ModuleName == "PropertyEditor" && Reason == EModuleChangeReason::ModuleLoaded)
	{
		UE_LOG(LogTemp, Warning, TEXT("PropertyEditor module loaded, now registering customizations"));
		FBlueprintDetailsCustomization::Register();
		FModuleManager::Get().OnModulesChanged().RemoveAll(this);
	}
}

void FUnrealMastermindModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	
	FUnrealMastermindStyle::Shutdown();
	FUnrealMastermindCommands::Unregister();
	FBlueprintDetailsCustomization::Unregister();
	
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(UnrealMastermindTabName);
	
	// Shutdown Blueprint tab extension
	if (BlueprintTabExtension.IsValid())
	{
		BlueprintTabExtension->Shutdown();
		BlueprintTabExtension.Reset();
	}
}


TSharedRef<SDockTab> FUnrealMastermindModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SUnrealMastermindTab)
		];
}

void FUnrealMastermindModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(UnrealMastermindTabName);
}

void FUnrealMastermindModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	// Add menu extension to the "Window" menu in the editor
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FUnrealMastermindCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	// Add toolbar button
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FUnrealMastermindCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}

	// Add debug menu item
    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
    if (Menu)
    {
        FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
        Section.AddMenuEntry(
            "UnrealMastermindDebug",
            FText::FromString("Debug Selected Class"),
            FText::FromString("Print info about the currently selected object"),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([]() {
                // Get the selected actor(s)
                TArray<UObject*> SelectedObjects;
                GEditor->GetSelectedActors()->GetSelectedObjects(SelectedObjects);
                
                // Get the selected assets
                TArray<FAssetData> SelectedAssets;
                GEditor->GetContentBrowserSelections(SelectedAssets);
                
                UE_LOG(LogTemp, Warning, TEXT("Selected Actors/Components (%d):"), SelectedObjects.Num());
                for (UObject* Object : SelectedObjects)
                {
                    UE_LOG(LogTemp, Warning, TEXT("  - %s (Class: %s)"), 
                        *Object->GetName(), 
                        *Object->GetClass()->GetName());
                }
                
                UE_LOG(LogTemp, Warning, TEXT("Selected Assets (%d):"), SelectedAssets.Num());
                for (const FAssetData& Asset : SelectedAssets)
                {
                    UE_LOG(LogTemp, Warning, TEXT("  - %s (Class: %s)"), 
                        *Asset.AssetName.ToString(), 
                        *Asset.AssetClassPath.ToString());
                    
                    // If it's a Blueprint, load it and print more info
                    UObject* AssetObject = Asset.GetAsset();
                    if (AssetObject && AssetObject->IsA<UBlueprint>())
                    {
                        UBlueprint* Blueprint = Cast<UBlueprint>(AssetObject);
                        UE_LOG(LogTemp, Warning, TEXT("    Blueprint Class: %s"), *Blueprint->GetClass()->GetName());
                        UE_LOG(LogTemp, Warning, TEXT("    Blueprint Name: %s"), *Blueprint->GetFName().ToString());
                    }
                }
                
                FNotificationInfo Info(FText::FromString("Selected objects logged. Check the Output Log."));
                Info.ExpireDuration = 3.0f;
                FSlateNotificationManager::Get().AddNotification(Info);
            }))
        );
    }
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnrealMastermindModule, UnrealMastermind)