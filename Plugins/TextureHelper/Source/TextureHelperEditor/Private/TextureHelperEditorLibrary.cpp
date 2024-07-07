// Fill out your copyright notice in the Description page of Project Settings.


#include "TextureHelperEditorLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "FileHelpers.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"

#define LOCTEXT_NAMESPACE "TextureHelpers"

UTexture2D* UTextureHelperEditorLibrary::CurrTexture = nullptr;

void UTextureHelperEditorLibrary::Grayscale(UTexture2D* InTexture)
{
	if (!InTexture)
	{
		return;
	}
	int32 row = InTexture->GetSizeY();
	int32 col = InTexture->GetSizeX();
	FColor* InTextureColor = static_cast<FColor*>(InTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	for (int r = 0; r <= row; r++)
	{
		for (int c = 0; c <= col; c++)
		{

			FColor& CurColor = InTextureColor[(c + (r * col))];

			uint8 avg = (CurColor.R + CurColor.G + CurColor.B) / 3;
			//	change color
			CurColor.R = avg;
			CurColor.G = avg;
			CurColor.B = avg;

		}
	}

	InTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

	InTexture->UpdateResource();
}

void UTextureHelperEditorLibrary::Negative(UTexture2D* InTexture)
{
	if (!InTexture)
	{
		return;
	}

	int32 row = InTexture->GetSizeY();
	int32 col = InTexture->GetSizeX();
	FColor* InTextureColor = static_cast<FColor*>(InTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	for (int r = 0; r <= row; r++)
	{
		for (int c = 0; c <= col; c++)
		{

			FColor& CurColor = InTextureColor[(c + (r * col))];

			//	change color
			CurColor.R = 1 - CurColor.R;
			CurColor.G = 1 - CurColor.G;
			CurColor.B = 1 - CurColor.B;
		}
	}

	InTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

	InTexture->UpdateResource();
}

void UTextureHelperEditorLibrary::SaveTexture(UTexture2D* TextureAsset, UTexture2D* WorkingTexture)
{
	if (!TextureAsset)
	{
		return;
	}

	CopyTexture(WorkingTexture, TextureAsset);

	UPackage* Package = TextureAsset->GetPackage();
	Package->FullyLoad();

	FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());

	if (UPackage::SavePackage(Package, TextureAsset, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName))
	{
		UE_LOG(LogTemp, Log, TEXT("Texture saved successfully."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save the texture."));
	}
}

static bool OpenSaveAsDialog(UClass* SavedClass, const FString& InDefaultPath, const FString& InNewNameSuggestion, FString& OutPackageName)
{
	FString DefaultPath = InDefaultPath;

	if (DefaultPath.IsEmpty())
	{
		DefaultPath = TEXT("/Game/Maps");
	}

	FString NewNameSuggestion = InNewNameSuggestion;
	check(!NewNameSuggestion.IsEmpty());

	FSaveAssetDialogConfig SaveAssetDialogConfig;
	{
		SaveAssetDialogConfig.DefaultPath = DefaultPath;
		SaveAssetDialogConfig.DefaultAssetName = NewNameSuggestion;
		SaveAssetDialogConfig.AssetClassNames.Add(SavedClass->GetClassPathName());
		SaveAssetDialogConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::AllowButWarn;
		SaveAssetDialogConfig.DialogTitleOverride = (SavedClass == UWorld::StaticClass())
			? LOCTEXT("SaveLevelDialogTitle", "Save Level As")
			: LOCTEXT("SaveAssetDialogTitle", "Save Asset As");
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FString SaveObjectPath = ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveAssetDialogConfig);

	if (!SaveObjectPath.IsEmpty())
	{
		OutPackageName = FPackageName::ObjectPathToPackageName(SaveObjectPath);
		return true;
	}

	return false;
}

void UTextureHelperEditorLibrary::SaveAsTexture(UTexture2D* WorkingTexture)
{
	/*
	FString PackageName = TEXT("/Game/kitty45");
	//OpenSaveAsDialog(UTexture2D::StaticClass(), TEXT("/Game/"), TEXT("NewTexture"), PackageName);

	UPackage* Package = CreatePackage(*PackageName);
	Package->FullyLoad();

	UTexture2D* NewTexture = NewObject<UTexture2D>(Package, TEXT("kitty45"), RF_Public | RF_Standalone | RF_MarkAsRootSet);
	int32 TextureWidth = WorkingTexture->GetSizeX();
	int32 TextureHeight = WorkingTexture->GetSizeY();
	NewTexture->AddToRoot();
	NewTexture->PlatformData = new FTexturePlatformData();
	NewTexture->PlatformData->SizeX = TextureWidth;
	NewTexture->PlatformData->SizeY = TextureHeight;
	NewTexture->PlatformData->SetNumSlices(WorkingTexture->GetPlatformData()->GetNumSlices());
	NewTexture->PlatformData->PixelFormat = WorkingTexture->GetPlatformData()->PixelFormat;

	CopyTexture(WorkingTexture, NewTexture);

	NewTexture->UpdateResource();
	Package->FullyLoad();

	// save assets under new name
	TArray<UObject*> SavedObjects;
	TArray<UObject*> ObjectsToSave;
	ObjectsToSave.Add(NewTexture);
	FEditorFileUtils::SaveAssetsAs(ObjectsToSave, SavedObjects);

	if (NewTexture && NewTexture->PlatformData)
	{
		NewTexture->UpdateResource();
		Package->MarkPackageDirty();
		FAssetRegistryModule::AssetCreated(NewTexture);

		FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
		bool bSaved = UPackage::SavePackage(Package, NewTexture, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName, GError, nullptr, true, true, SAVE_NoError);

		if (bSaved)
		{
			UE_LOG(LogTemp, Log, TEXT("Texture saved successfully."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save the texture."));
		}
	}
	*/
}

void UTextureHelperEditorLibrary::Undo()
{
}

UTexture2D* UTextureHelperEditorLibrary::DisplayTexture(UTexture2D* TextureAsset)
{
	if (!TextureAsset)
	{
		return nullptr;
	}

	return DuplicateTexture(TextureAsset);
}

void UTextureHelperEditorLibrary::CopyTexture(UTexture2D* SourceTexture, UTexture2D* DestinationTexture)
{
	if (!SourceTexture || !DestinationTexture)
	{
		return;
	}

	// Ensure the destination texture's platform data is initialized
	if (!DestinationTexture->PlatformData)
	{
		DestinationTexture->PlatformData = new FTexturePlatformData();
	}

	// Copy basic properties
	DestinationTexture->PlatformData->SizeX = SourceTexture->PlatformData->SizeX;
	DestinationTexture->PlatformData->SizeY = SourceTexture->PlatformData->SizeY;
	DestinationTexture->PlatformData->PixelFormat = SourceTexture->PlatformData->PixelFormat;
	DestinationTexture->SRGB = SourceTexture->SRGB;

	// Clear existing mipmaps in the destination texture
	DestinationTexture->PlatformData->Mips.Empty();

	// Copy each mip level
	for (int32 MipIndex = 0; MipIndex < SourceTexture->PlatformData->Mips.Num(); ++MipIndex)
	{
		const FTexture2DMipMap& SourceMip = SourceTexture->PlatformData->Mips[MipIndex];
		FTexture2DMipMap* DestMip = new FTexture2DMipMap();

		DestMip->SizeX = SourceMip.SizeX;
		DestMip->SizeY = SourceMip.SizeY;

		// Allocate memory for the mip data
		DestMip->BulkData.Lock(LOCK_READ_WRITE);
		void* DestData = DestMip->BulkData.Realloc(SourceMip.BulkData.GetBulkDataSize());
		const void* SourceData = SourceMip.BulkData.LockReadOnly();

		// Copy the data
		FMemory::Memcpy(DestData, SourceData, SourceMip.BulkData.GetBulkDataSize());

		SourceMip.BulkData.Unlock();
		DestMip->BulkData.Unlock();

		// Add the new mip level to the destination texture
		DestinationTexture->PlatformData->Mips.Add(DestMip);
	}

	// Update the destination texture resource
	DestinationTexture->UpdateResource();
}

UTexture2D* UTextureHelperEditorLibrary::DuplicateTexture(UTexture2D* SourceTexture)
{
	if (!SourceTexture)
	{
		return nullptr;
	}

	// Create a new UTexture2D object
	UTexture2D* NewTexture = NewObject<UTexture2D>(
		GetTransientPackage(),
		NAME_None,
		RF_Transient
	);

	if (!NewTexture)
	{
		return nullptr;
	}

	// Create new platform data
	NewTexture->PlatformData = new FTexturePlatformData();
	NewTexture->PlatformData->SizeX = SourceTexture->PlatformData->SizeX;
	NewTexture->PlatformData->SizeY = SourceTexture->PlatformData->SizeY;
	NewTexture->PlatformData->PixelFormat = SourceTexture->PlatformData->PixelFormat;

	// Copy mip data
	for (int32 MipIndex = 0; MipIndex < SourceTexture->PlatformData->Mips.Num(); ++MipIndex)
	{
		const FTexture2DMipMap& SourceMip = SourceTexture->PlatformData->Mips[MipIndex];
		FTexture2DMipMap* DestMip = new FTexture2DMipMap();

		DestMip->SizeX = SourceMip.SizeX;
		DestMip->SizeY = SourceMip.SizeY;

		// Allocate memory for the mip data
		DestMip->BulkData.Lock(LOCK_READ_WRITE);
		void* DestData = DestMip->BulkData.Realloc(SourceMip.BulkData.GetBulkDataSize());
		const void* SourceData = SourceMip.BulkData.LockReadOnly();

		FMemory::Memcpy(DestData, SourceData, SourceMip.BulkData.GetBulkDataSize());

		SourceMip.BulkData.Unlock();
		DestMip->BulkData.Unlock();

		NewTexture->PlatformData->Mips.Add(DestMip);
	}

	// Copy additional properties
	NewTexture->SRGB = SourceTexture->SRGB;

	// Update the resource
	NewTexture->UpdateResource();

	return NewTexture;
}
