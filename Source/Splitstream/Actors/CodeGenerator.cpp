#include "CodeGenerator.h"
#include "Actors/KeypadScanner/KeypadScanner.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "DataAssets/ItemBase.h"
#include "Net/UnrealNetwork.h"
#include "Characters/CivilianCharacter.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"


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
    DOREPLIFETIME(ACodeGenerator, CodesDisplayText);

    UE_LOG(LogTemp, Display, TEXT("[%s][%s] Lifetime replication set."), *GetNameSafe(this), TEXT(__FUNCTION__));
}

bool ACodeGenerator::IsCorrectItem_Implementation(const FInventorySlot& Slot) const
{
    if (!Slot.ItemAsset || !Slot.OwnerCivilian) return false;
    return Slot.OwnerCivilian->GetClass() == RequiredCivilianClass.Get();
}

void ACodeGenerator::Interact_Implementation(AActor* Interactor)
{

    if (!HasAuthority()) {
        return;
    }
    if (!Interactor) {
        return;
    }

    StatusArray.Empty();

    int32 CodesCreated = 0;
    for (AKeypadScanner* Keypad : ManagedKeypads)
    {
        if (!Keypad)
        {
            continue;
        }
        if (!Keypad->HasAuthority())
        {
            continue;
        }

        FString NewCode;
        for (int32 i = 0; i < Keypad->CodeLength; ++i)
        {
            NewCode.AppendInt(FMath::RandRange(0, 9));
        }

        float Lifetime = Keypad->CodeLifetime;
        float Expiry = GetWorld()->GetTimeSeconds() + Lifetime;

        Keypad->SetCodeWithExpiry(NewCode, Lifetime);

        StatusArray.Add(FKeypadCodeStatus(NewCode, Expiry, Keypad));

        ++CodesCreated;
    }

    UpdateDisplayText();
}

void ACodeGenerator::ExpireOldCodes()
{
    if (!HasAuthority())
        return;

    float Now = GetWorld()->GetTimeSeconds();

    // Print countdowns and status
    FString LogSummary = FString::Printf(TEXT("ExpireOldCodes: StatusArray.Num=%d"), StatusArray.Num());
    for (int32 i = 0; i < StatusArray.Num(); ++i)
    {
        FString Code = StatusArray[i].Code;
        float Expiry = StatusArray[i].ExpiryTime;
        float TimeLeft = FMath::Max(0.f, Expiry - Now);
        FString KeypadName = StatusArray[i].Keypad ? StatusArray[i].Keypad->GetName() : TEXT("Unknown");
        LogSummary += FString::Printf(TEXT("\n  [%d] Keypad: %s, Code: %s, TimeLeft: %.2fs, Expiry: %.2f"),
            i, *KeypadName, *Code, TimeLeft, Expiry);
    }

    bool bChanged = false;

    for (int32 i = StatusArray.Num() - 1; i >= 0; --i)
    {
        if (StatusArray[i].ExpiryTime <= Now)
        {
            FString ExpiredKeypad = StatusArray[i].Keypad ? StatusArray[i].Keypad->GetName() : TEXT("Unknown");
            StatusArray.RemoveAt(i);
            bChanged = true;
        }
    }
    UpdateDisplayText();
}

void ACodeGenerator::UpdateDisplayText()
{
    FString DisplayText;
    float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    for (const FKeypadCodeStatus& Status : StatusArray)
    {
        float TimeLeft = FMath::Max(0.f, Status.ExpiryTime - Now);
        FString KeypadName = Status.Keypad ? Status.Keypad->GetName() : TEXT("Unknown");
        DisplayText += FString::Printf(TEXT("Keypad: %s\nCode: %s\nTime Left: %.1fs\n\n"),
            *KeypadName, *Status.Code, TimeLeft);
    }
    CodesDisplayText = DisplayText;

    if (CodesTextComp && HasAuthority())
    {
        CodesTextComp->SetText(FText::FromString(CodesDisplayText));
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
