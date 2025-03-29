// Copyright Froströk. All Rights Reserved.

#include "UnrealMastermind.h"

#include "UnrealMastermindStyle.h"
#include "UnrealMastermindCommands.h"
#include "UnrealMastermindTab.h"
#include "Widgets/Docking/SDockTab.h"
#include "ToolMenus.h"
#include "PropertyEditorModule.h"
#include "..\Public\BlueprintDetailsCustomization.h"
#include "UnrealMastermindBlueprintTab.h"

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

	// Check if PropertyEditor is loaded
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FBlueprintDetailsCustomization::Register();
	}
	else
	{
		FModuleManager::Get().OnModulesChanged().AddRaw(this, &FUnrealMastermindModule::HandleModulesChanged);
	}
    
	// Initialize Blueprint tab extension
	BlueprintTabExtension = MakeShareable(new FUnrealMastermindBlueprintTabExtension());
	BlueprintTabExtension->Initialize();
}

void FUnrealMastermindModule::HandleModulesChanged(FName ModuleName, EModuleChangeReason Reason) const
{
	if (ModuleName == "PropertyEditor" && Reason == EModuleChangeReason::ModuleLoaded)
	{
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
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnrealMastermindModule, UnrealMastermind)