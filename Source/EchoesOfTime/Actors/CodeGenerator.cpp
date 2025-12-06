#include "CodeGenerator.h"
#include "Actors/KeypadScanner/KeypadScanner.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Characters/CivilianCharacter.h"
#include "DataAssets/ItemBase.h"
#include "Net/UnrealNetwork.h"
#include "ActorComponents/InventoryComponent.h"

ACodeGenerator::ACodeGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

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
    
    // Start timer to check for expired codes and update display (0.5 second intervals)
    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(DisplayUpdateTimerHandle, this, &ACodeGenerator::CheckExpiredCodes, 0.5f, true);
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
    
    // Start/restart the timer when new codes are generated
    if (!GetWorldTimerManager().IsTimerActive(DisplayUpdateTimerHandle))
    {
        GetWorldTimerManager().SetTimer(DisplayUpdateTimerHandle, this, &ACodeGenerator::CheckExpiredCodes, 0.5f, true);
    }
}

void ACodeGenerator::CheckExpiredCodes()
{
    if (!HasAuthority()) return;

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

    // Only update display when something changed or when codes are active (for countdown)
    if (bChanged)
    {
        UpdateDisplayText();
        
        // Stop the timer if no codes are active
        if (StatusArray.Num() == 0)
        {
            GetWorldTimerManager().ClearTimer(DisplayUpdateTimerHandle);
        }
    }
    else if (StatusArray.Num() > 0)
    {
        // Update display for countdown even when no codes expired
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