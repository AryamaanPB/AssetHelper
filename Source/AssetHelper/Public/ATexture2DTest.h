// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ATexture2DTest.generated.h"

UCLASS()
class ASSETHELPER_API AATexture2DTest : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AATexture2DTest();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


public:

	UFUNCTION(BlueprintCallable)
	void ModifyTexture();


	UFUNCTION(BlueprintCallable)
	void CreateWhiteTexture();


public:

	UPROPERTY(EditAnywhere)
	UTexture2D* OrigTexture;

};
