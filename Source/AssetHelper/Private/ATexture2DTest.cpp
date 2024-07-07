// Fill out your copyright notice in the Description page of Project Settings.


#include "ATexture2DTest.h"
#include "AssetRegistry/AssetRegistryModule.h"

// Sets default values
AATexture2DTest::AATexture2DTest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AATexture2DTest::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AATexture2DTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AATexture2DTest::Grayscale(UTexture2D* InTexture)
{
	if (InTexture)
	{
		int32 row = InTexture->GetSizeY();
		int32 col = InTexture->GetSizeX();
		FColor* InTextureColor = static_cast<FColor*>(InTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
		
		uint8* Pixels = new uint8[col * row * 4];
		for (int r = 0; r <= row; r++)
		{
			for (int c = 0; c <= col; c++)
			{

				FColor& CurColor = InTextureColor[(c + (r * col))];

				uint8 avg = (CurColor.R + CurColor.G + CurColor.B) / 3;
				//	change color

				int32 curPixelIndex = ((r * col) + c);
				Pixels[4 * curPixelIndex] = avg;
				Pixels[4 * curPixelIndex + 1] = avg;
				Pixels[4 * curPixelIndex + 2] = avg;
				Pixels[4 * curPixelIndex + 3] = 255;
				
			}
		}

		InTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

		InTexture->UpdateResource();

		InTexture->Source.Init(col, row, 1, 1, InTexture->Source.GetFormat(), Pixels);

		UPackage* Package = InTexture->GetPackage();
		Package->FullyLoad();

		FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());

		if (UPackage::SavePackage(Package, InTexture, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName))
		{
			UE_LOG(LogTemp, Log, TEXT("Texture saved successfully."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save the texture."));
		}
	}
}

void AATexture2DTest::CreateNewTexture()
{
	FString PackageName = TEXT("/Game/ProceduralTextures/");
	FString TextureName = TEXT("ProcTex");
	PackageName += TextureName;
	UPackage* Package = CreatePackage(*PackageName);
	Package->FullyLoad();

	UTexture2D* NewTexture = NewObject<UTexture2D>(Package, *TextureName, RF_Public | RF_Standalone | RF_MarkAsRootSet);
	int32 TextureWidth = 1920;
	int32 TextureHeight = 1080;
	NewTexture->AddToRoot();				
	NewTexture->PlatformData = new FTexturePlatformData();	
	NewTexture->PlatformData->SizeX = TextureWidth;
	NewTexture->PlatformData->SizeY = TextureHeight;
	NewTexture->PlatformData->SetNumSlices(1);
	NewTexture->PlatformData->PixelFormat = EPixelFormat::PF_B8G8R8A8;

	uint8* Pixels = new uint8[TextureWidth * TextureHeight * 4];
	for (int32 y = 0; y < TextureHeight; y++)
	{
		for (int32 x = 0; x < TextureWidth; x++)
		{
			int32 curPixelIndex = ((y * 1920) + x);
			Pixels[4 * curPixelIndex] = 255;
			Pixels[4 * curPixelIndex + 1] = 255;
			Pixels[4 * curPixelIndex + 2] = 255;
			Pixels[4 * curPixelIndex + 3] = 255;
		}
	}

	if (NewTexture && NewTexture->PlatformData)
	{
		// Create a new mip map
		FTexture2DMipMap* Mip = new FTexture2DMipMap();

		// Configure the mip map properties (e.g., size, allocation, etc.)
		Mip->SizeX = NewTexture->GetSizeX();
		Mip->SizeY = NewTexture->GetSizeY();
		Mip->BulkData.Lock(LOCK_READ_WRITE);

		// Add the mip map to the texture's PlatformData
		NewTexture->PlatformData->Mips.Add(Mip);

		// Perform operations on the mip map
		uint8* TextureData = (uint8*)Mip->BulkData.Realloc(TextureWidth * TextureHeight * 4);
		FMemory::Memcpy(TextureData, Pixels, sizeof(uint8) * TextureHeight * TextureWidth * 4);

		// Unlock the bulk data
		Mip->BulkData.Unlock();

		// Update the texture resource
		NewTexture->UpdateResource();

		NewTexture->Source.Init(TextureWidth, TextureHeight, 1, 1, ETextureSourceFormat::TSF_BGRA8, Pixels);

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

		delete[] Pixels;	// Don't forget to free the memory here
	}
}

