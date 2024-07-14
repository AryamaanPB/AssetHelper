// Fill out your copyright notice in the Description page of Project Settings.


#include "EngineWindowController.h"
#include "Widgets/Colors/SColorPicker.h"

void UEngineWindowController::OnSetColorFromColorPicker(FLinearColor NewColor)
{
    OnColorPickedEvent.Broadcast(NewColor);
}

void UEngineWindowController::OpenColorPickerWindow()
{
    FColorPickerArgs PickerArgs;
    {
        PickerArgs.bOnlyRefreshOnOk = true;
        PickerArgs.DisplayGamma = TAttribute<float>::Create(TAttribute<float>::FGetter::CreateUObject(GEngine, &UEngine::GetDisplayGamma));
        PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateUObject(this, &UEngineWindowController::OnSetColorFromColorPicker);
    }

    OpenColorPicker(PickerArgs);
}
