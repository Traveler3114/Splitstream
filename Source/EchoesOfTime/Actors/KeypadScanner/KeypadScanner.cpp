#include "KeypadScanner.h"
#include "KeypadButton.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Interfaces/IKeycardUnlockable.h"
#include "Net/UnrealNetwork.h"

AKeypadScanner::AKeypadScanner()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    NetUpdateFrequency = 10.f; // Keypad code changes infrequently

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    KeypadScannerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeypadScannerMesh"));
    KeypadScannerMesh->SetupAttachment(DefaultSceneRoot);

    CodeTextRenderComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CodeTextRenderComp"));
    CodeTextRenderComp->SetupAttachment(KeypadScannerMesh);

    LinkedActor = nullptr;
    EnteredCode = "";
    bCodeCorrect = false;
    bUnlocked = false;
}

void AKeypadScanner::BeginPlay()
{
    Super::BeginPlay();
    SpawnKeypadButtons();
    OnRep_EnteredCode();
}

void AKeypadScanner::SpawnKeypadButtons()
{
    if (!KeypadButtonClass) return;

    const int Rows = 4;
    const int Cols = 3;
    const float ButtonSpacing = 60.f;
    FVector StartLocation = ButtonGridOffset;

    FString ButtonLabels[Rows][Cols] = {
        {TEXT("1"), TEXT("2"), TEXT("3")},
        {TEXT("4"), TEXT("5"), TEXT("6")},
        {TEXT("7"), TEXT("8"), TEXT("9")},
        {TEXT("*"), TEXT("0"), TEXT("#")}
    };

    for (int row = 0; row < Rows; ++row)
    {
        for (int col = 0; col < Cols; ++col)
        {
            FVector LocalOffset = StartLocation + FVector(col * ButtonSpacing, 0, -row * ButtonSpacing);

            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            AKeypadButton* NewButton = GetWorld()->SpawnActor<AKeypadButton>(
                KeypadButtonClass,
                FVector::ZeroVector,
                FRotator::ZeroRotator,
                SpawnParams
            );
            if (NewButton && NewButton->NumberTextRenderComp)
            {
                FString Symbol = ButtonLabels[row][col];
                NewButton->NumberTextRenderComp->SetText(FText::FromString(Symbol));
                NewButton->ButtonSymbol = Symbol;

                NewButton->AttachToComponent(
                    KeypadScannerMesh,
                    FAttachmentTransformRules::KeepRelativeTransform
                );
                NewButton->SetActorRelativeLocation(LocalOffset);
                NewButton->SetActorRelativeRotation(FRotator::ZeroRotator);

                NewButton->OnButtonPressed.AddDynamic(this, &AKeypadScanner::AppendCodeSymbol);
                KeypadButtons.Add(NewButton);
            }
        }
    }
}

void AKeypadScanner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AKeypadScanner, EnteredCode);
}

void AKeypadScanner::OnRep_EnteredCode()
{
    if (CodeTextRenderComp)
    {
        CodeTextRenderComp->SetText(FText::FromString(EnteredCode));
    }
}

void AKeypadScanner::SetEnteredCodeAndUpdateText(const FString& NewCode)
{
    if (HasAuthority())
    {
        EnteredCode = NewCode;
        if (CodeTextRenderComp)
        {
            CodeTextRenderComp->SetText(FText::FromString(EnteredCode));
        }
    }
}

// --- Your original function ---
void AKeypadScanner::SetCorrectCode(const FString& Code)
{
    CorrectCode = Code;
}

void AKeypadScanner::SetCodeWithExpiry(const FString& NewCode, float LifetimeSeconds)
{
    if (!HasAuthority()) return;

    SetCorrectCode(NewCode);
    SetEnteredCodeAndUpdateText(""); // Clear display for new attempt
    bCodeCorrect = false;
    bUnlocked = false;

    GetWorldTimerManager().ClearTimer(CodeExpiryHandle);

    if (LifetimeSeconds > 0.f)
    {
        GetWorldTimerManager().SetTimer(CodeExpiryHandle, this, &AKeypadScanner::ClearCode, LifetimeSeconds, false);
    }
}

void AKeypadScanner::ClearCode()
{
    if (!HasAuthority()) return;
    SetCorrectCode(TEXT(""));
    bCodeCorrect = false;
    SetEnteredCodeAndUpdateText(""); // Optionally clear display
}

void AKeypadScanner::AppendCodeSymbol(const FString& Symbol)
{
    if (!HasAuthority()) return;

    if (Symbol == "*")
    {
        SetEnteredCodeAndUpdateText("");
        bCodeCorrect = false;
        return;
    }

    if (Symbol == "#")
    {
        if (EnteredCode == CorrectCode)
        {
            bCodeCorrect = true;
            SetEnteredCodeAndUpdateText(TEXT("READY"));
        }
        else
        {
            bCodeCorrect = false;
            SetEnteredCodeAndUpdateText(TEXT("WRONG"));
            FTimerHandle ClearHandle;
            GetWorldTimerManager().SetTimer(ClearHandle, [this]()
                {
                    SetEnteredCodeAndUpdateText("");
                }, 1.0f, false);
        }
        return;
    }

    SetEnteredCodeAndUpdateText(EnteredCode + Symbol);
}

void AKeypadScanner::TryUnlock(AActor* Interactor)
{
    if (bUnlocked || !bCodeCorrect || !LinkedActor || !Interactor) return;

    // At this point, the character has already checked and used the required item!

    bUnlocked = true;

    if (LinkedActor->GetClass()->ImplementsInterface(UKeycardUnlockable::StaticClass()))
    {
        IKeycardUnlockable::Execute_UnlockWithKeycard(LinkedActor, Interactor);
    }
    SetEnteredCodeAndUpdateText(TEXT("UNLOCKED"));
}

// KeypadScanner.cpp
bool AKeypadScanner::IsCorrectItem_Implementation(UItemBase* Item) const
{
    // Only allow if the item is not null and is the right keycard type
    return Item && Item->ItemType == RequiredKeycardType;
}

void AKeypadScanner::Interact_Implementation(AActor* Interactor)
{
    TryUnlock(Interactor);
}

void AKeypadScanner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    for (AKeypadButton* Button : KeypadButtons)
    {
        if (Button)
        {
            Button->OnButtonPressed.RemoveDynamic(this, &AKeypadScanner::AppendCodeSymbol);
        }
    }
    Super::EndPlay(EndPlayReason);
}

void AKeypadScanner::SetHighlighted_Implementation(bool bHighlight)
{
    if (KeypadScannerMesh)
    {
        KeypadScannerMesh->SetRenderCustomDepth(bHighlight);
        KeypadScannerMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}