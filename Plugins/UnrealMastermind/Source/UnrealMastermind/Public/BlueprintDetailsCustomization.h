// Copyright 2025 © Froströk. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class FBlueprintDetailsCustomization : public IDetailCustomization
{
public:
    /** Makes a new instance of this detail layout class for a specific detail view requesting it */
    static TSharedRef<IDetailCustomization> MakeInstance();

    /** IDetailCustomization interface */
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
    /** The blueprint being displayed */
    TWeakObjectPtr<UBlueprint> SelectedBlueprint;

    /** Button handlers */
    FReply OnViewDocumentationClicked() const;
    FReply OnGenerateDocumentationClicked() const;
    
    /** Documentation retrieval */
    bool HasDocumentation() const;
    FString GetDocumentation() const;

    /** Register the details customization */
    static bool bIsRegistered;
    
public:
    /** Register the customization for the Blueprint class */
    static void Register();
    
    /** Unregister the customization */
    static void Unregister();
};