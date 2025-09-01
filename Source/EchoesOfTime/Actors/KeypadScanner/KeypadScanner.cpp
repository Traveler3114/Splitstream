#include "KeypadScanner.h"
#include "KeypadButton.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Interfaces/IKeycardUnlockable.h"
#include "InventorySystem/InventoryComponent.h"
#include "InventorySystem/ItemBase.h"

AKeypadScanner::AKeypadScanner()
{
    PrimaryActorTick.bCanEverTick = true;

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
}

void AKeypadScanner::SpawnKeypadButtons()
{
    if (!KeypadButtonClass) return;

    const int Rows = 4;
    const int Cols = 3;
    const float ButtonSpacing = 60.f;

    FVector Scale = GetActorScale3D();
    FVector StartLocation = GetActorLocation() + ButtonGridOffset * Scale;
    float ScaledButtonSpacing = ButtonSpacing * Scale.X;

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
            FVector Location = StartLocation + FVector(col * ScaledButtonSpacing, 0, -row * ScaledButtonSpacing);

            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            AKeypadButton* NewButton = GetWorld()->SpawnActor<AKeypadButton>(KeypadButtonClass, Location, FRotator::ZeroRotator, SpawnParams);
            if (NewButton && NewButton->NumberTextRenderComp)
            {
                FString Symbol = ButtonLabels[row][col];
                NewButton->NumberTextRenderComp->SetText(FText::FromString(Symbol));
                NewButton->ButtonSymbol = Symbol;
                NewButton->SetActorScale3D(Scale);
                NewButton->OnButtonPressed.AddDynamic(this, &AKeypadScanner::AppendCodeSymbol);
                KeypadButtons.Add(NewButton);
            }
        }
    }
}

void AKeypadScanner::AppendCodeSymbol(const FString& Symbol)
{
    if (!CodeTextRenderComp) return;

    // Handle clear (*) and submit (#)
    if (Symbol == "*")
    {
        EnteredCode.Empty();
        CodeTextRenderComp->SetText(FText::FromString(""));
        bCodeCorrect = false;
        return;
    }

    if (Symbol == "#")
    {
        if (EnteredCode == CorrectCode)
        {
            bCodeCorrect = true;
            CodeTextRenderComp->SetText(FText::FromString("READY"));
        }
        else
        {
            bCodeCorrect = false;
            CodeTextRenderComp->SetText(FText::FromString("WRONG"));
            EnteredCode.Empty();
        }
        return;
    }

    // Append digit
    EnteredCode += Symbol;
    CodeTextRenderComp->SetText(FText::FromString(EnteredCode));
}

void AKeypadScanner::TryUnlock(AActor* Interactor)
{
    if (bUnlocked || !bCodeCorrect || !LinkedActor || !Interactor) return;

    // Check for keycard in Interactor's inventory
    UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();
    if (!Inventory) return;

    FInventorySlot ActiveSlot = Inventory->GetActiveItem();
    UItemBase* ActiveItem = ActiveSlot.ItemAsset;
    if (ActiveItem && ActiveItem->ItemType == EItemType::Keycard)
    {
        // Use the keycard (trigger OnUsed logic)
        ActiveItem->OnUsed(Interactor);
        bUnlocked = true;

        if (LinkedActor->GetClass()->ImplementsInterface(UKeycardUnlockable::StaticClass()))
        {
            IKeycardUnlockable::Execute_UnlockWithKeycard(LinkedActor, Interactor);
        }
        CodeTextRenderComp->SetText(FText::FromString("UNLOCKED"));
    }
    else
    {
        // Optional: feedback for no keycard
        CodeTextRenderComp->SetText(FText::FromString("NEED CARD"));
    }
}

void AKeypadScanner::Interact_Implementation(AActor* Interactor)
{
    // Called when the player "scans" their card on the scanner
    TryUnlock(Interactor);
}

void AKeypadScanner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
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