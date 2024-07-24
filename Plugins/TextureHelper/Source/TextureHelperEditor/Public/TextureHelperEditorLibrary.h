// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityLibrary.h"
#include "EngineWindowController.h"
#include "TextureHelperEditorLibrary.generated.h"

UENUM(BlueprintType)
enum class EBlendModes : uint8
{
	BLEND_Opaque UMETA(DisplayName = "Opaque"),
	BLEND_Masked UMETA(DisplayName = "Masked"),
	BLEND_Translucent UMETA(DisplayName = "Translucent"),
	BLEND_Additive UMETA(DisplayName = "Additive"),
	BLEND_Modulate UMETA(DisplayName = "Modulate"),
	BLEND_AlphaComposite UMETA(DisplayName = "Alpha Composite"),
	BLEND_AlphaHoldout UMETA(DisplayName = "Alpha Holdout"),
	BLEND_MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ERotationMode : uint8
{
	ROTATE_CLOCKWISE UMETA(DisplayName = "Clockwise"),
	ROTATE_ANTICLOCKWISE UMETA(DisplayName = "Anti-clockwise"),
};

USTRUCT(BlueprintType)
struct FMipPixelData {
	GENERATED_BODY()
public:

	TArray<FColor> MipPixelColor;

	FColor operator[] (int32 i) {
		return MipPixelColor[i];
	}

	void Add(FColor color) {
		MipPixelColor.Add(color);
	}
};

/**
 * 
 */
UCLASS()
class TEXTUREHELPEREDITOR_API UTextureHelperEditorLibrary : public UEditorUtilityLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Asset Helper|Texture Helper")
	static void Grayscale(UTexture2D* InTexture);

	UFUNCTION(BlueprintCallable, Category = "Asset Helper|Texture Helper")
	static void Negative(UTexture2D* InTexture);

	UFUNCTION(BlueprintCallable, Category = "Asset Helper|Texture Helper")
	static void RotateTextureInPlace(UTexture2D* InTexture, ERotationMode RotationMode);

	UFUNCTION(BlueprintCallable, Category = "Asset Helper|Texture Helper")
	static void ChromaKeyTexture(UTexture2D* InTexture, FColor ChromaColor, float InTolerance);

	UFUNCTION(BlueprintCallable, Category = "Asset Helper|Texture Helper")
	static void BackupTexture(UTexture2D* InTexture);

	UFUNCTION(BlueprintCallable, Category = "Asset Helper|Texture Helper")
	static UTexture2D* CreateCheckeredTexture();

	UFUNCTION(BlueprintCallable, Category = "Asset Helper|Texture Helper")
	static void SaveTexture(UTexture2D* TextureAsset, UTexture2D* WorkingTexture);

	UFUNCTION(BlueprintCallable, Category = "Asset Helper|Texture Helper")
	static void SaveAsTexture(UTexture2D* WorkingTexture);

	UFUNCTION(BlueprintCallable, Category = "Asset Helper|Texture Helper")
	static void Clear();

	UFUNCTION(BlueprintCallable, Category = "Asset Helper|Texture Helper")
	static void Undo();

	UFUNCTION(BlueprintCallable, Category = "Asset Helper|Texture Helper")
	static UTexture2D* DisplayTexture(UTexture2D* TextureAsset);

	UFUNCTION(BlueprintCallable, Category = "Asset Helper|Texture Helper")
	static void CopyTexture(UTexture2D* SourceTexture, UTexture2D* DestinationTexture);

	UFUNCTION(BlueprintCallable, Category = "Asset Helper|Texture Helper")
	static UTexture2D* DuplicateTexture(UTexture2D* SourceTexture);

	UFUNCTION(BlueprintCallable, Category = "Asset Helper|Texture Helper")
	static UEngineWindowController* PickColor();

private:

	static TArray<FColor> BufferColorData;

	static UEngineWindowController* EngineWindowManager;
};
