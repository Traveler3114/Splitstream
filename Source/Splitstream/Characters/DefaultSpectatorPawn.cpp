#include "DefaultSpectatorPawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpectatorPawn.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADefaultSpectatorPawn::ADefaultSpectatorPawn()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

// Bind A/D for cycling
void ADefaultSpectatorPawn::SetupPlayerInputComponent(UInputComponent* InInputComponent)
{
    Super::SetupPlayerInputComponent(InInputComponent);
    if (!InInputComponent) return;

    InInputComponent->BindKey(EKeys::A, IE_Pressed, this, &ADefaultSpectatorPawn::SpectatePrevious);
    InInputComponent->BindKey(EKeys::D, IE_Pressed, this, &ADefaultSpectatorPawn::SpectateNext);
    InInputComponent->BindAxis("MoveForward", this, &ADefaultSpectatorPawn::MoveForward);
    InInputComponent->BindAxis("MoveRight", this, &ADefaultSpectatorPawn::MoveRight);
}

void ADefaultSpectatorPawn::BeginPlay()
{
    Super::BeginPlay();
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        UpdateSpectateList();
        if (SpectatablePlayers.Num() > 0)
        {
            SetSpectateTarget(0);
        }
    }
}

void ADefaultSpectatorPawn::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (APlayerController* PC = Cast<APlayerController>(NewController))
    {
        UpdateSpectateList();
        if (SpectatablePlayers.Num() > 0)
        {
            SetSpectateTarget(0);
        }
    }
}


void ADefaultSpectatorPawn::SpectateNext()
{
    UpdateSpectateList();
    if (SpectatablePlayers.Num() == 0) return;
    SetSpectateTarget((SpectateTargetIndex + 1) % SpectatablePlayers.Num());
}

void ADefaultSpectatorPawn::SpectatePrevious()
{
    UpdateSpectateList();
    if (SpectatablePlayers.Num() == 0) return;
    SetSpectateTarget((SpectateTargetIndex - 1 + SpectatablePlayers.Num()) % SpectatablePlayers.Num());
}

void ADefaultSpectatorPawn::SetSpectateTarget(int32 Index)
{
    UpdateSpectateList();
    if (SpectatablePlayers.Num() == 0)
    {
        return;
    }

    SpectateTargetIndex = Index % SpectatablePlayers.Num();
    if (SpectateTargetIndex < 0) SpectateTargetIndex += SpectatablePlayers.Num();

    APlayerState* TargetPS = SpectatablePlayers[SpectateTargetIndex];
    if (!TargetPS) return;
    APawn* TargetPawn = TargetPS->GetPawn();
    if (!TargetPawn) return;

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->SetViewTargetWithBlend(TargetPawn, 0.25f);
    }
}

void ADefaultSpectatorPawn::UpdateSpectateList()
{
    SpectatablePlayers.Empty();
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* LocalPC = Cast<APlayerController>(GetController());

    // Use PlayerArray from GameState for replicated player list
    AGameStateBase* GS = World->GetGameState<AGameStateBase>();
    if (!GS) return;

    for (APlayerState* PS : GS->PlayerArray)
    {
        if (!PS) continue;
        if (LocalPC && PS == LocalPC->PlayerState) continue;

        APawn* Pawn = PS->GetPawn();
        if (!Pawn || Pawn->IsA<ASpectatorPawn>()) continue; // skip spectators/dead

        SpectatablePlayers.Add(PS);
    }
}

// Prevent movement (overwrite movement input handler)
void ADefaultSpectatorPawn::MoveForward(float) {}
void ADefaultSpectatorPawn::MoveRight(float) {}