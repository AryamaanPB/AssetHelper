// Fill out your copyright notice in the Description page of Project Settings.


#include "TextureHelperEditorLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "FileHelpers.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"

#define LOCTEXT_NAMESPACE "TextureHelpers"

TArray<FColor> UTextureHelperEditorLibrary::BufferColorData;

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

void UTextureHelperEditorLibrary::RotateTextureInPlace(UTexture2D* InTexture, ERotationMode RotationMode)
{
	if (!InTexture || !InTexture->PlatformData || InTexture->PlatformData->Mips.Num() == 0)
	{
		return;
	}

	// Get the original texture dimensions
	const int32 OriginalWidth = InTexture->PlatformData->Mips[0].SizeX;
	const int32 OriginalHeight = InTexture->PlatformData->Mips[0].SizeY;

	// Lock the texture's mipmap data for reading
	uint8* OriginalMipData = static_cast<uint8*>(InTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	// Create a temporary buffer for the rotated data
	TArray<uint8> RotatedData;
	RotatedData.SetNumUninitialized(OriginalWidth * OriginalHeight * 4);

	// Fill the temporary buffer with the rotated data
	for (int32 Y = 0; Y < OriginalHeight; ++Y)
	{
		for (int32 X = 0; X < OriginalWidth; ++X)
		{
			int32 OriginalIndex = ((Y * OriginalWidth) + X) * 4;
			int32 RotatedIndex;
			if (RotationMode == ERotationMode::ROTATE_CLOCKWISE)
			{
				RotatedIndex = ((X * OriginalHeight) + (OriginalHeight - 1 - Y)) * 4;
			}
			else if (RotationMode == ERotationMode::ROTATE_ANTICLOCKWISE)
			{
				RotatedIndex = (((OriginalWidth - 1 - X) * OriginalHeight) + Y) * 4;
			}

			RotatedData[RotatedIndex + 0] = OriginalMipData[OriginalIndex + 0]; // Blue
			RotatedData[RotatedIndex + 1] = OriginalMipData[OriginalIndex + 1]; // Green
			RotatedData[RotatedIndex + 2] = OriginalMipData[OriginalIndex + 2]; // Red
			RotatedData[RotatedIndex + 3] = OriginalMipData[OriginalIndex + 3]; // Alpha
		}
	}

	// Reallocate the original texture to the new dimensions
	InTexture->PlatformData->SizeX = OriginalHeight;
	InTexture->PlatformData->SizeY = OriginalWidth;
	InTexture->PlatformData->Mips[0].SizeX = OriginalHeight;
	InTexture->PlatformData->Mips[0].SizeY = OriginalWidth;

	// Copy the rotated data back into the original texture
	FMemory::Memcpy(OriginalMipData, RotatedData.GetData(), RotatedData.Num());

	// Unlock the texture data
	InTexture->PlatformData->Mips[0].BulkData.Unlock();

	// Update the texture resource
	InTexture->UpdateResource();

}

void UTextureHelperEditorLibrary::ChromaKeyTexture(UTexture2D* InTexture, FColor ChromaColor, float InTolerance)
{
	if (!InTexture || !InTexture->GetPlatformData())
	{
		return;
	}

	BackupTexture(InTexture);

	int32 TextureHeight = InTexture->GetSizeY();
	int32 TextureWidth = InTexture->GetSizeX();
	FColor* InTextureColor = static_cast<FColor*>(InTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	// Normalize the ChromaColor values to be in the range [0, 1]
	FVector ChromaVector(ChromaColor.R / 255.0f, ChromaColor.G / 255.0f, ChromaColor.B / 255.0f);

	for (int32 Y = 0; Y < TextureHeight; ++Y)
	{
		for (int32 X = 0; X < TextureWidth; ++X)
		{
			int32 Index = X + (Y * TextureWidth);
			FColor& CurColor = InTextureColor[Index];
			FColor& OriginalColor = BufferColorData[Index];

			CurColor.A = OriginalColor.A;

			// Normalize the current color values to be in the range [0, 1]
			FVector CurVector(CurColor.R / 255.0f, CurColor.G / 255.0f, CurColor.B / 255.0f);

			// Calculate the distance in RGB color space
			float ColorDiff = FVector::Dist(CurVector, ChromaVector);

			if (ColorDiff <= InTolerance)
			{
				CurColor.A = 0;
			}
		}
	}

	InTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
	InTexture->UpdateResource();
}

void UTextureHelperEditorLibrary::BackupTexture(UTexture2D* InTexture)
{
	if (!InTexture)
	{
		return;
	}

	int32 TextureHeight = InTexture->GetSizeY();
	int32 TextureWidth = InTexture->GetSizeX();
	FColor* InTextureColor = static_cast<FColor*>(InTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	// Backup the original texture data if not already backed up
	if (BufferColorData.Num() == 0)
	{
		BufferColorData.SetNum(TextureWidth * TextureHeight);
		FMemory::Memcpy(BufferColorData.GetData(), InTextureColor, TextureWidth * TextureHeight * sizeof(FColor));
	}

	InTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
}

UTexture2D* UTextureHelperEditorLibrary::CreateCheckeredTexture()
{
	const int32 TextureWidth = 1920; // Texture dimensions
	const int32 TextureHeight = 1080; // Texture dimensions
	const int32 CheckeredSize = 8; // Size of each checkered block

	// Create a transient UTexture2D
	UTexture2D* NewTexture = UTexture2D::CreateTransient(TextureWidth, TextureHeight, PF_B8G8R8A8);
	if (!NewTexture || !NewTexture->PlatformData)
	{
		return nullptr;
	}

	// Ensure the texture has at least one mipmap
	FTexture2DMipMap& Mip = NewTexture->PlatformData->Mips[0];
	Mip.SizeX = TextureWidth;
	Mip.SizeY = TextureHeight;
	Mip.BulkData.Lock(LOCK_READ_WRITE);

	// Allocate memory for the texture data
	const int32 TotalBytes = TextureWidth * TextureHeight * 4;
	uint8* MipData = (uint8*)Mip.BulkData.Realloc(TotalBytes);
	FMemory::Memzero(MipData, TotalBytes);

	// Fill the texture with a checkered pattern
	for (int32 Y = 0; Y < TextureHeight; ++Y)
	{
		for (int32 X = 0; X < TextureWidth; ++X)
		{
			// Determine the color based on the checkered pattern
			bool bIsGray = ((X / CheckeredSize) % 2 == (Y / CheckeredSize) % 2);
			FColor PixelColor = bIsGray ? FColor(128, 128, 128, 255) : FColor(0, 0, 0, 255);

			// Set the pixel data
			int32 Index = ((Y * TextureWidth) + X) * 4;
			MipData[Index + 0] = PixelColor.B; // Blue
			MipData[Index + 1] = PixelColor.G; // Green
			MipData[Index + 2] = PixelColor.R; // Red
			MipData[Index + 3] = PixelColor.A; // Alpha
		}
	}

	// Unlock the texture data
	Mip.BulkData.Unlock();

	// Update the texture resource
	NewTexture->UpdateResource();

	return NewTexture;
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

	BufferColorData.Empty();
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

void UTextureHelperEditorLibrary::Clear()
{
	BufferColorData.Empty();
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

	BackupTexture(TextureAsset);

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
