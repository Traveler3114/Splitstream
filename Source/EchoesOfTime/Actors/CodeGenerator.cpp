#include "CodeGenerator.h"
#include "Actors/KeypadScanner/KeypadScanner.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Characters/CivilianCharacter.h"
#include "DataAssets/ItemBase.h"
#include "Net/UnrealNetwork.h"
#include "ActorComponents/InventoryComponent.h"
#include "TimerManager.h"

ACodeGenerator::ACodeGenerator()
{
    PrimaryActorTick.bCanEverTick = false; // Disable Tick for performance!

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    CodeGenMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeskMesh"));
    CodeGenMesh->SetupAttachment(DefaultSceneRoot);

    CodesTextComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CodesTextComp"));
    CodesTextComp->SetupAttachment(CodeGenMesh);

    if (CodesTextComp)
    {
        CodesTextComp->SetText(FText::GetEmpty());
    }
}

void ACodeGenerator::BeginPlay()
{
    Super::BeginPlay();
    UpdateDisplayText();

    if (HasAuthority())
    {
        // Check every 0.1s; adjust interval for your desired "responsiveness"
        GetWorldTimerManager().SetTimer(ExpireTimerHandle, this, &ACodeGenerator::ExpireOldCodes, 0.1f, true);
    }
}

void ACodeGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    GetWorldTimerManager().ClearTimer(ExpireTimerHandle);
}

void ACodeGenerator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACodeGenerator, TargetCivilian);
    DOREPLIFETIME(ACodeGenerator, CodesDisplayText);
}

bool ACodeGenerator::IsCorrectItem_Implementation(UItemBase* Item) const
{
    UItemBase* FP = Cast<UItemBase>(Item);
    return FP && FP->OwnerCivilian == TargetCivilian;
}

void ACodeGenerator::Interact_Implementation(AActor* Interactor)
{
    if (!HasAuthority()) return;
    if (!Interactor || !TargetCivilian)
        return;

    StatusArray.Empty();

    for (AKeypadScanner* Keypad : ManagedKeypads)
    {
        if (!Keypad || !Keypad->HasAuthority()) continue;

        FString NewCode;
        for (int32 i = 0; i < Keypad->CodeLength; ++i)
        {
            NewCode.AppendInt(FMath::RandRange(0, 9));
        }

        float Lifetime = Keypad->CodeLifetime;
        float Expiry = GetWorld()->GetTimeSeconds() + Lifetime;

        Keypad->SetCodeWithExpiry(NewCode, Lifetime);

        StatusArray.Add(FKeypadCodeStatus(NewCode, Expiry, Keypad));
    }

    UpdateDisplayText();
}

void ACodeGenerator::ExpireOldCodes()
{
    if (!HasAuthority())
        return;

    bool bChanged = false;
    float Now = GetWorld()->GetTimeSeconds();

    for (int32 i = StatusArray.Num() - 1; i >= 0; --i)
    {
        if (StatusArray[i].ExpiryTime <= Now)
        {
            StatusArray.RemoveAt(i);
            bChanged = true;
        }
    }

    if (bChanged)
    {
        UpdateDisplayText();
    }
}

void ACodeGenerator::UpdateDisplayText()
{
    FString DisplayText;
    float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    for (const FKeypadCodeStatus& Status : StatusArray)
    {
        float TimeLeft = FMath::Max(0.f, Status.ExpiryTime - Now);
        FString KeypadName = Status.Keypad ? Status.Keypad->GetName() : TEXT("Unknown");
        DisplayText += FString::Printf(TEXT("Keypad: %s\nCode: %s\nTime Left: %.0fs\n\n"),
            *KeypadName, *Status.Code, TimeLeft);
    }

    // Only update/replicate if changed
    if (CodesDisplayText != DisplayText)
    {
        CodesDisplayText = DisplayText;

        if (CodesTextComp && HasAuthority())
        {
            CodesTextComp->SetText(FText::FromString(CodesDisplayText));
        }
    }
}

void ACodeGenerator::OnRep_CodesDisplayText()
{
    if (CodesTextComp)
    {
        CodesTextComp->SetText(FText::FromString(CodesDisplayText));
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