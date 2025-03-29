// Copyright YourCompany. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "IDetailCustomNodeBuilder.h"

class FBlueprintDetailsExtension : public IDetailCustomNodeBuilder, public TSharedFromThis<FBlueprintDetailsExtension>
{
public:
    FBlueprintDetailsExtension(TWeakObjectPtr<UBlueprint> InBlueprint);
    
    // IDetailCustomNodeBuilder Interface
    virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override {}
    virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
    virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
    virtual void Tick(float DeltaTime) override {}
    virtual bool RequiresTick() const override { return false; }
    virtual bool InitiallyCollapsed() const override { return false; }
    virtual FName GetName() const override { return TEXT("UnrealMastermindDocs"); }
    
private:
    TWeakObjectPtr<UBlueprint> Blueprint;
    
    FReply OnViewDocumentationClicked();
    FReply OnGenerateDocumentationClicked();
};

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
    FReply OnViewDocumentationClicked();
    FReply OnGenerateDocumentationClicked();
    
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