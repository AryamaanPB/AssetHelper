// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EngineWindowController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnColorPicked, FLinearColor, ColorPicked);

/**
 * 
 */
UCLASS()
class TEXTUREHELPEREDITOR_API UEngineWindowController : public UObject
{
	GENERATED_BODY()

public:

	void OnSetColorFromColorPicker(FLinearColor NewColor);

	void OpenColorPickerWindow();

public:

	UPROPERTY(BlueprintAssignable)
	FOnColorPicked OnColorPickedEvent;
};
