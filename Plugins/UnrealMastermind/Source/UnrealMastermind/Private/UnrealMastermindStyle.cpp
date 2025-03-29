// Copyright Froströk. All Rights Reserved.

#include "UnrealMastermindStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FUnrealMastermindStyle::StyleInstance = nullptr;

void FUnrealMastermindStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FUnrealMastermindStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FUnrealMastermindStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("UnrealMastermindStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef<FSlateStyleSet> FUnrealMastermindStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("UnrealMastermindStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("UnrealMastermind")->GetBaseDir() / TEXT("Resources"));

	Style->Set("UnrealMastermind.OpenPluginWindow", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40x40));
	
	return Style;
}

void FUnrealMastermindStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FUnrealMastermindStyle::Get()
{
	return *StyleInstance;
}