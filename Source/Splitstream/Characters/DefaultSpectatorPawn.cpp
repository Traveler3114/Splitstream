#include "DefaultSpectatorPawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpectatorPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"


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

    // Optional: Prevent movement/freelook
    InInputComponent->BindAxis("MoveForward", this, &ADefaultSpectatorPawn::MoveForward);
    InInputComponent->BindAxis("MoveRight", this, &ADefaultSpectatorPawn::MoveRight);
}


void ADefaultSpectatorPawn::BeginPlay()
{
    Super::BeginPlay();
    // On clients: assign camera to first spectatable player
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        UpdateSpectateList();
        SetSpectateTarget(0);
    }
}

void ADefaultSpectatorPawn::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    if (APlayerController* PC = Cast<APlayerController>(NewController))
    {
        UpdateSpectateList();
        SetSpectateTarget(0);
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
    if (SpectatablePlayers.Num() == 0) return;
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

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = Cast<APlayerController>(It->Get());
        if (!PC || PC == LocalPC) continue;

        APawn* Pawn = PC->GetPawn();
        if (!Pawn || Pawn->IsA<ASpectatorPawn>()) continue; // skip spectators/dead

        // Optionally: Check your player is actually alive (add team/status logic here)
        SpectatablePlayers.Add(PC->PlayerState);
    }
}