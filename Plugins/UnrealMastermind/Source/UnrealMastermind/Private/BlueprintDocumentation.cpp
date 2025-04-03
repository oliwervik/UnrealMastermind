// Copyright 2025 © Froströk. All Rights Reserved.

#include "BlueprintDocumentation.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Blueprint.h"
#include "Misc/PackageName.h"
#include "UObject/UObjectHash.h"
#include "UObject/MetaData.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

const FName UBlueprintDocumentation::DocumentationMetadataKey = FName("UnrealMastermindDocumentation");

bool UBlueprintDocumentation::SaveDocumentation(UBlueprint* Blueprint, const FString& Documentation)
{
	if (!Blueprint)
		return false;

	// Create a JSON object to store the documentation
	const TSharedPtr<FJsonObject> DocObject = MakeShareable(new FJsonObject);
	DocObject->SetStringField("documentation", Documentation);
	DocObject->SetStringField("timestamp", FDateTime::Now().ToString());

	// Convert to string
	FString DocJson;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&DocJson);
	FJsonSerializer::Serialize(DocObject.ToSharedRef(), Writer);

	// Get the package
	UPackage* Package = Blueprint->GetPackage();
	if (!Package)
	{
		return false;
	}

	// Set the metadata using the correct approach
	if (UMetaData* MetaData = Package->GetMetaData())
	{
		// Set the metadata on the Blueprint object
		MetaData->SetValue(Blueprint, DocumentationMetadataKey, *DocJson);
	}
	else
	{
		return false;
	}

	// Mark the Blueprint as dirty
	Blueprint->Modify();

	// Mark the package as dirty
	Package->SetDirtyFlag(true);

	// Save the package to disk
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;

	FString PackageFileName;
	if (FPackageName::TryConvertLongPackageNameToFilename(Package->GetName(), PackageFileName,
	                                                      FPackageName::GetAssetPackageExtension()))
	{
		// Make sure the file is writable using platform-independent approach
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (PlatformFile.FileExists(*PackageFileName) && PlatformFile.IsReadOnly(*PackageFileName))
		{
			PlatformFile.SetReadOnly(*PackageFileName, false);
		}

		// Use UPackage::SavePackage to save the asset
		const bool bSuccess = UPackage::SavePackage(Package, nullptr, *PackageFileName, SaveArgs);


		// Also notify the asset registry that the asset has changed
		FAssetRegistryModule::AssetCreated(Blueprint);

		// Verify the save
		FString VerifyDoc = GetDocumentation(Blueprint);


		return bSuccess;
	}


	return false;
}

FString UBlueprintDocumentation::GetDocumentation(UBlueprint* Blueprint)
{
	if (!Blueprint)
		return FString();

	FString DocJson;

	// Use the correct approach to get metadata
	if (UMetaData* MetaData = Blueprint->GetPackage()->GetMetaData())
	{
		// Get the metadata from the Blueprint object
		DocJson = MetaData->GetValue(Blueprint, DocumentationMetadataKey);
	}
	else
	{
		return FString();
	}

	if (DocJson.IsEmpty())
	{
		return FString();
	}

	// Parse the JSON
	TSharedPtr<FJsonObject> DocObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(DocJson);
	if (FJsonSerializer::Deserialize(Reader, DocObject) && DocObject.IsValid())
	{
		// Extract the documentation field
		FString Documentation;
		if (DocObject->TryGetStringField(TEXT("documentation"), Documentation))
		{
			return Documentation;
		}
	}


	// Fallback: return the raw JSON if parsing failed
	return DocJson;
}

bool UBlueprintDocumentation::HasDocumentation(UBlueprint* Blueprint)
{
	if (!Blueprint)
		return false;

	// Use the correct approach to check metadata
	if (UMetaData* MetaData = Blueprint->GetPackage()->GetMetaData())
	{
		return MetaData->HasValue(Blueprint, DocumentationMetadataKey);
	}

	return false;
}

bool UBlueprintDocumentation::ClearDocumentation(UBlueprint* Blueprint)
{
	if (!Blueprint)
		return false;

	// Use the correct approach to remove metadata
	if (UMetaData* MetaData = Blueprint->GetPackage()->GetMetaData())
	{
		MetaData->RemoveValue(Blueprint, DocumentationMetadataKey);
	}

	// Mark the Blueprint as dirty
	Blueprint->Modify();

	// Save the Blueprint
	UPackage* Package = Blueprint->GetPackage();
	if (Package)
	{
		Package->SetDirtyFlag(true);

		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;

		FString PackageFileName;
		if (FPackageName::TryConvertLongPackageNameToFilename(Package->GetName(), PackageFileName,
		                                                      FPackageName::GetAssetPackageExtension()))
		{
			return UPackage::SavePackage(Package, nullptr, *PackageFileName, SaveArgs);
		}
	}

	return false;
}
