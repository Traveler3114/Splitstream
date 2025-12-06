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
    PrimaryActorTick.bCanEverTick = false; // Use timer instead of Tick for better performance

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

    // Set up timer to check for expired codes at regular intervals instead of every tick
    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(CodeUpdateTimerHandle, this, &ACodeGenerator::CheckExpiredCodes, CodeCheckInterval, true);
    }
}

void ACodeGenerator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACodeGenerator, TargetCivilian);
    DOREPLIFETIME(ACodeGenerator, CodesDisplayText);
}

bool ACodeGenerator::IsCorrectItem_Implementation(UItemBase* Item) const
{
    // Accept only correct fingerprint for the assigned civilian
    UItemBase* FP = Cast<UItemBase>(Item);
    return FP && FP->OwnerCivilian == TargetCivilian;
}

void ACodeGenerator::Interact_Implementation(AActor* Interactor)
{
    if (!HasAuthority()) return;

    if (!Interactor || !TargetCivilian)
        return;


    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Correct fingerprint!"));
    }

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

void ACodeGenerator::CheckExpiredCodes()
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

    // Always update display text when codes are active to show countdown
    if (bChanged || StatusArray.Num() > 0)
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

    // Always update the replicated string (server sets, clients receive)
    CodesDisplayText = DisplayText;

    // Only server should set the actual text component directly
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