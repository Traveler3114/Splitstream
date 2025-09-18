// Fill out your copyright notice in the Description page of Project Settings.


#include "CodeGenerator.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Characters/CivilianCharacter.h"
#include "DataAssets/ItemBase.h"
#include "DataAssets/FingerprintItem.h"
#include "Net/UnrealNetwork.h"
#include "ActorComponents/InventoryComponent.h"

// Sets default values
ACodeGenerator::ACodeGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	CodeGenMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeskMesh"));
    CodeGenMesh->SetupAttachment(DefaultSceneRoot);

}

// Called when the game starts or when spawned
void ACodeGenerator::BeginPlay()
{
	Super::BeginPlay();
}

void ACodeGenerator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACodeGenerator, TargetCivilian);
}

void ACodeGenerator::Interact_Implementation(AActor* Interactor)
{
    if (!Interactor || !TargetCivilian)
        return;

    UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();
    if (!Inventory)
        return;

    FInventorySlot ActiveSlot = Inventory->GetActiveItem();
    UItemBase* ActiveItem = ActiveSlot.ItemAsset;

    UFingerprintItem* FPItem = Cast<UFingerprintItem>(ActiveItem);
    if (FPItem && FPItem->OwnerCivilian == TargetCivilian)
    {
        // Fingerprint matches!
        FPItem->OnUsed(Interactor);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Correct fingerprint!"));
        }
        // TODO: Generate code, set code on keypad, start timer here
    }
    else if (ActiveItem && ActiveItem->ItemType == EItemType::Fingerprint)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Incorrect fingerprint!"));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Incorrect item!"));
        }
    }
}

void ACodeGenerator::SetHighlighted_Implementation(bool bHighlight)
{
    if (CodeGenMesh)
    {
        CodeGenMesh->SetRenderCustomDepth(bHighlight);
        CodeGenMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}

