// Fill out your copyright notice in the Description page of Project Settings.


#include "TextureHelperEditorLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "FileHelpers.h"
#include "IContentBrowserSingleton.h"
#include "RHI.h"
#include "ContentBrowserModule.h"

#define LOCTEXT_NAMESPACE "TextureHelpers"

TArray<FColor> UTextureHelperEditorLibrary::BufferColorData;

UEngineWindowController* UTextureHelperEditorLibrary::EngineWindowManager = nullptr;

//FOnColorPicked UTextureHelperEditorLibrary::OnColorPickedEvent;

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
	if (!InTexture || !InTexture->GetPlatformData() || InTexture->GetPlatformData()->Mips.Num() == 0)
	{
		return;
	}

	// Get the original texture dimensions
	const int32 OriginalWidth = InTexture->GetPlatformData()->Mips[0].SizeX;
	const int32 OriginalHeight = InTexture->GetPlatformData()->Mips[0].SizeY;

	// Lock the texture's mipmap data for reading
	uint8* OriginalMipData = static_cast<uint8*>(InTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

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
	InTexture->GetPlatformData()->SizeX = OriginalHeight;
	InTexture->GetPlatformData()->SizeY = OriginalWidth;
	InTexture->GetPlatformData()->Mips[0].SizeX = OriginalHeight;
	InTexture->GetPlatformData()->Mips[0].SizeY = OriginalWidth;

	// Copy the rotated data back into the original texture
	FMemory::Memcpy(OriginalMipData, RotatedData.GetData(), RotatedData.Num());

	// Unlock the texture data
	InTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

	// Update the texture resource
	InTexture->UpdateResource();

}

void UTextureHelperEditorLibrary::FlipTexture(UTexture2D* InTexture, EOrientationMode InOrientation)
{
	if (!InTexture || !InTexture->GetPlatformData() || InTexture->GetPlatformData()->Mips.Num() == 0)
	{
		return;
	}

	// Get the original texture dimensions
	const int32 Width = InTexture->GetPlatformData()->Mips[0].SizeX;
	const int32 Height = InTexture->GetPlatformData()->Mips[0].SizeY;

	// Lock the texture's mipmap data for reading
	uint8* OriginalMipData = static_cast<uint8*>(InTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	// Create a temporary buffer for the flipped data
	TArray<uint8> FlippedData;
	FlippedData.SetNumUninitialized(Width * Height * 4);

	// Fill the temporary buffer with the flipped data
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			int32 OriginalIndex = ((Y * Width) + X) * 4;

			int32 FlippedIndex;

			switch (InOrientation)
			{
			case EOrientationMode::ORIENTATION_HORIZONTAL:
				FlippedIndex = ((Y * Width) + (Width - 1 - X)) * 4;
				break;
			case EOrientationMode::ORIENTATION_VERTICAL:
				FlippedIndex = (((Height - 1 - Y) * Width) + X) * 4;
				break;
			default:
				UE_LOG(LogTemp, Display, TEXT("No orientation selected"));
				return;
			}

			// Copy pixel data (Blue, Green, Red, Alpha)
			FlippedData[FlippedIndex + 0] = OriginalMipData[OriginalIndex + 0]; // Blue
			FlippedData[FlippedIndex + 1] = OriginalMipData[OriginalIndex + 1]; // Green
			FlippedData[FlippedIndex + 2] = OriginalMipData[OriginalIndex + 2]; // Red
			FlippedData[FlippedIndex + 3] = OriginalMipData[OriginalIndex + 3]; // Alpha
		}
	}

	// Copy the flipped data back into the original texture
	FMemory::Memcpy(OriginalMipData, FlippedData.GetData(), FlippedData.Num());

	// Unlock the texture data
	InTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

	// Update the texture resource
	InTexture->UpdateResource();
}


FVector2D UTextureHelperEditorLibrary::GetGameResolution()
{
	FVector2D Result = FVector2D(1, 1);

	Result.X = GSystemResolution.ResX;
	Result.Y = GSystemResolution.ResY;

	return Result;
}


void UTextureHelperEditorLibrary::EraseTextureArea(UTexture2D* InTexture, const FVector2D& Center, float Radius = 0.1)
{
	if (!InTexture || !InTexture->GetPlatformData() || InTexture->GetPlatformData()->Mips.Num() == 0)
	{
		return;
	}

	// Get the texture dimensions
	const int32 Width = InTexture->GetPlatformData()->Mips[0].SizeX;
	const int32 Height = InTexture->GetPlatformData()->Mips[0].SizeY;

	// Lock the texture's mipmap data for reading and writing
	uint8* MipData = static_cast<uint8*>(InTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	// Create an FColor buffer to manipulate pixel data
	TArray<FColor> PixelData;
	PixelData.SetNumUninitialized(Width * Height);

	// Radius squared to optimize the distance check
	const float RadiusSquared = Radius * Radius;

	// Iterate through each pixel of the texture
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			// Calculate the index for the pixel
			int32 PixelIndex = (Y * Width) + X;

			// Calculate the distance squared from the center point
			float DistSquared = FVector2D::DistSquared(FVector2D(X, Y), Center);

			// If the pixel is within the radius threshold, erase it (set alpha to 0)
			if (DistSquared <= RadiusSquared)
			{
				// Read the pixel color
				FColor* PixelColor = reinterpret_cast<FColor*>(MipData + PixelIndex * sizeof(FColor));

				// Set alpha to 0 to erase the pixel (making it fully transparent)
				PixelColor->A = 0;

				// Optionally, modify other color channels (e.g., clear RGB to make the pixel fully black/transparent)
				PixelColor->R = 0;
				PixelColor->G = 0;
				PixelColor->B = 0;
			}
		}
	}

	// Unlock the texture data after making changes
	InTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

	// Update the texture resource to apply the changes
	InTexture->UpdateResource();
}


void UTextureHelperEditorLibrary::ChromaKeyTexture(UTexture2D* InTexture, FColor ChromaColor, float InTolerance)
{
	if (!InTexture || !InTexture->GetPlatformData())
	{
		return;
	}

	//BackupTexture(InTexture);

	if (BufferColorData.Num() == 0)
	{
		BackupTexture(InTexture);
		return;
	}

	float ScaledTolerance = InTolerance * FMath::Sqrt(3.0);

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

			if (InTolerance <= 0.001)
			{
				continue;
			}

			// Normalize the current color values to be in the range [0, 1]
			FVector CurVector(CurColor.R / 255.0f, CurColor.G / 255.0f, CurColor.B / 255.0f);

			// Calculate the distance in RGB color space
			float ColorDiff = FVector::Dist(CurVector, ChromaVector);

			if (ColorDiff <= ScaledTolerance)
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

	TextureMipGenSettings cacheMipGenSettings = InTexture->MipGenSettings;
	InTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	InTexture->UpdateResource();

	
	FColor* InTextureColor = static_cast<FColor*>(InTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	// Backup the original texture data if not already backed up
	if (BufferColorData.Num() == 0)
	{
		BufferColorData.SetNum(TextureWidth * TextureHeight);
		FMemory::Memcpy(BufferColorData.GetData(), InTextureColor, TextureWidth * TextureHeight * sizeof(FColor));
	}
	InTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

	InTexture->MipGenSettings = cacheMipGenSettings;
	InTexture->UpdateResource();
}


UTexture2D* UTextureHelperEditorLibrary::CreateCheckeredTexture()
{
	const int32 TextureWidth = 1920; // Texture dimensions
	const int32 TextureHeight = 1080; // Texture dimensions
	const int32 CheckeredSize = 8; // Size of each checkered block

	// Create a transient UTexture2D
	UTexture2D* NewTexture = UTexture2D::CreateTransient(TextureWidth, TextureHeight, PF_B8G8R8A8);
	if (!NewTexture || !NewTexture->GetPlatformData())
	{
		return nullptr;
	}

	// Ensure the texture has at least one mipmap
	FTexture2DMipMap& Mip = NewTexture->GetPlatformData()->Mips[0];
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
	FString PackageName = TEXT("/Game/Image");
	UPackage* Package = CreatePackage(*PackageName);
	Package->FullyLoad();

	// Create a new UTexture2D object
	UTexture2D* NewTexture = NewObject<UTexture2D>(Package, TEXT("kitty45"), RF_Public | RF_Standalone | RF_MarkAsRootSet);
	NewTexture->AddToRoot();

	// Get the size and format of the working texture
	int32 TextureWidth = WorkingTexture->GetSizeX();
	int32 TextureHeight = WorkingTexture->GetSizeY();
	EPixelFormat PixelFormat = WorkingTexture->GetPlatformData()->PixelFormat;

	// Initialize the platform data for the new texture
	FTexturePlatformData* NewPlatformData = new FTexturePlatformData();
	NewPlatformData->SizeX = TextureWidth;
	NewPlatformData->SizeY = TextureHeight;
	NewPlatformData->PixelFormat = PixelFormat;

	// Create a new mip map for the texture and initialize it
	FTexture2DMipMap* NewMip = new FTexture2DMipMap();
	NewMip->SizeX = TextureWidth;
	NewMip->SizeY = TextureHeight;
	NewPlatformData->Mips.Add(NewMip);

	// Allocate memory for the mip level and initialize it
	int32 MipBytes = CalculateImageBytes(TextureWidth, TextureHeight, 0, PixelFormat);
	NewMip->BulkData.Lock(LOCK_READ_WRITE);
	void* TextureData = NewMip->BulkData.Realloc(MipBytes);
	FMemory::Memzero(TextureData, MipBytes);  // Clear memory to ensure it's initialized
	NewMip->BulkData.Unlock();

	// Assign the platform data to the new texture
	NewTexture->SetPlatformData(NewPlatformData);

	// Copy the texture data from the working texture to the new texture
	CopyTexture(WorkingTexture, NewTexture);

	// Update the resource to apply the copied data
	NewTexture->UpdateResource();
	Package->FullyLoad();

	// Save the new texture as an asset
	TArray<UObject*> SavedObjects;
	TArray<UObject*> ObjectsToSave;
	ObjectsToSave.Add(NewTexture);
	FEditorFileUtils::SaveAssetsAs(ObjectsToSave, SavedObjects);

	if (NewTexture && NewTexture->GetPlatformData())
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
	if (SourceTexture && DestinationTexture)
	{
		// Get the size and format of the source texture
		int32 TextureWidth = SourceTexture->GetSizeX();
		int32 TextureHeight = SourceTexture->GetSizeY();
		ETextureSourceFormat SourceFormat = SourceTexture->Source.GetFormat();
		SourceFormat = SourceFormat == TSF_Invalid ? TSF_BGRA8 : SourceFormat; //TODO: avoid this by ensuring SourceTexture has a Texture Source Format
		EPixelFormat PixelFormat = SourceTexture->GetPlatformData()->PixelFormat;

		// Determine bytes per pixel based on the pixel format
		int32 BytesPerPixel = GPixelFormats[PixelFormat].BlockBytes;

		// Lock the source texture data
		FTexture2DMipMap& SourceMip = SourceTexture->GetPlatformData()->Mips[0];
		const uint8* SourceData = static_cast<const uint8*>(SourceMip.BulkData.Lock(LOCK_READ_ONLY));

		// Calculate the number of bytes needed
		int32 NumBytes = TextureWidth * TextureHeight * BytesPerPixel;

		// Copy the source texture data into a new uint8 array
		uint8* RawData = new uint8[NumBytes];
		FMemory::Memcpy(RawData, SourceData, NumBytes);

		// Unlock the source texture data
		SourceMip.BulkData.Unlock();

		// Initialize the destination texture with the copied data
		DestinationTexture->Source.Init(TextureWidth, TextureHeight, 1, 1, SourceFormat, RawData);

		// Free the allocated memory
		delete[] RawData;

		// Update the destination texture resource
		DestinationTexture->UpdateResource();
	}
}



UTexture2D* UTextureHelperEditorLibrary::DuplicateTexture(UTexture2D* SourceTexture)
{
	if (!SourceTexture)
	{
		UE_LOG(LogTemp, Warning, TEXT("SourceTexture is null"));
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
		UE_LOG(LogTemp, Warning, TEXT("Failed to create NewTexture"));
		return nullptr;
	}

	//Temporarily removing all Mips from the source texture since Mip[0] always returned a nullptr on being locked.
	TEnumAsByte<TextureMipGenSettings> cachedMipGenSettings = SourceTexture->MipGenSettings;
	SourceTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	SourceTexture->UpdateResource();

	// Create new platform data
	NewTexture->SetPlatformData(new FTexturePlatformData());
	FTexturePlatformData* NewPlatformData = NewTexture->GetPlatformData();
	const FTexturePlatformData* SourcePlatformData = SourceTexture->GetPlatformData();

	if (!NewPlatformData || !SourcePlatformData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Platform data is null"));
		return nullptr;
	}

	NewPlatformData->SizeX = SourcePlatformData->SizeX;
	NewPlatformData->SizeY = SourcePlatformData->SizeY;
	NewPlatformData->PixelFormat = SourcePlatformData->PixelFormat;

	// Copy additional properties
	NewTexture->SRGB = SourceTexture->SRGB;
	NewPlatformData->Mips.Reset(SourcePlatformData->Mips.Num());

	int maxNumMips = SourcePlatformData->Mips.Num();

	// Copy mip data
	for (int32 MipIndex = 0; MipIndex < maxNumMips; ++MipIndex)
	{
		const FTexture2DMipMap& SourceMip = SourcePlatformData->Mips[MipIndex];
		FTexture2DMipMap* DestMip = new FTexture2DMipMap();
		NewPlatformData->Mips.Add(DestMip);

		DestMip->SizeX = SourceMip.SizeX;
		DestMip->SizeY = SourceMip.SizeY;

		// Allocate memory for the mip data
		DestMip->BulkData.Lock(LOCK_READ_WRITE);
		void* DestData = DestMip->BulkData.Realloc(SourceMip.BulkData.GetBulkDataSize());
		const void* SourceData = SourceMip.BulkData.LockReadOnly();

		if (!SourceData)
		{
			UE_LOG(LogTemp, Warning, TEXT("SourceData is null for MipIndex %d"), MipIndex);
		}
		if (!DestData)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to allocate DestData for MipIndex %d"), MipIndex);
		}

		if (SourceData && DestData)
		{
			FMemory::Memcpy(DestData, SourceData, SourceMip.BulkData.GetBulkDataSize());
		}

		SourceMip.BulkData.Unlock();
		DestMip->BulkData.Unlock();
	}

	// Update the resource
	NewTexture->UpdateResource();

	//Restoring MipGenSettings
	SourceTexture->MipGenSettings = cachedMipGenSettings;
	SourceTexture->UpdateResource();

	return NewTexture;
}



UEngineWindowController* UTextureHelperEditorLibrary::PickColor()
{
	if (!EngineWindowManager)
	{
		EngineWindowManager = NewObject<UEngineWindowController>();
	}

	EngineWindowManager->OpenColorPickerWindow();

	return EngineWindowManager;
}
