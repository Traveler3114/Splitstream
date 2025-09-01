// Fill out your copyright notice in the Description page of Project Settings.


#include "KeypadButton.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"

// Sets default values
AKeypadButton::AKeypadButton()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    // Create static mesh component and attach to root
    KeypadButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeypadButtonMesh"));
    KeypadButtonMesh->SetupAttachment(DefaultSceneRoot);

    // Create text render component and attach to root (or mesh, if desired)
    NumberTextRenderComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NumberTextRenderComp"));
    NumberTextRenderComp->SetupAttachment(KeypadButtonMesh);

    // (Optional) Set text properties
    NumberTextRenderComp->SetText(FText::FromString(TEXT("n")));
    // (Optional) Set relative location, scale, etc.
    // TextRenderComp->SetRelativeLocation(FVector(0, 0, 100));
}

// Called when the game starts or when spawned
void AKeypadButton::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AKeypadButton::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

