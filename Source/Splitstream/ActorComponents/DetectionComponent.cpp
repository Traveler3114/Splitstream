#include "DetectionComponent.h"
#include "Net/UnrealNetwork.h"
#include "Interfaces/IDetectable.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Player/Characters/DefaultCharacter.h"
#include "Player/Controllers/DefaultPlayerController.h"

UDetectionComponent::UDetectionComponent()
{
    SetIsReplicatedByDefault(true);
    PrimaryComponentTick.bCanEverTick = true;
}

void UDetectionComponent::BeginPlay()
{
    Super::BeginPlay();
    DetectionStates.Empty();
    LastSentStates.Empty();
    CachedOwner = Cast<ADefaultCharacter>(GetOwner());
}

void UDetectionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    DetectionStates.Empty();
    LastSentStates.Empty();
}

void UDetectionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UDetectionComponent::StartDetection(AActor* Detector)
{
    if (!GetOwner() || !Detector) return;
    if (GetOwnerRole() != ROLE_Authority) return;

    FDetectionState* ExistingState = DetectionStates.Find(Detector);
    if (ExistingState)
    {
        if (ExistingState->bFullyDetected)
            return;
        ExistingState->bDetectionInProgress = true;
        ExistingState->bFullyDetected = false;
        ExistingState->Direction = 1; // building
        // Do NOT reset progress: ExistingState->Progress stays as is
    }
    else
    {
        // Only for new detectors, start fresh
        FDetectionState& NewState = DetectionStates.FindOrAdd(Detector);
        NewState.bDetectionInProgress = true;
        NewState.bFullyDetected = false;
        NewState.Progress = 0.f;
        NewState.Direction = 1;
    }

    SetComponentTickEnabled(true);
    OnDetectionBegan.Broadcast(GetOwner());
}

void UDetectionComponent::StopDetection(AActor* Detector)
{
    if (!Detector) return;
    if (GetOwnerRole() != ROLE_Authority) return;

    FDetectionState* State = DetectionStates.Find(Detector);
    if (State)
    {
        State->bDetectionInProgress = false;
        State->Direction = -1; // decay mode
        SetComponentTickEnabled(true);
        OnDetectionEnded.Broadcast(GetOwner());
    }
}

void UDetectionComponent::ForceImmediateDetectionEnd(AActor* Detector)
{
    if (GetOwnerRole() != ROLE_Authority) return;

    if (Detector)
    {
        // Instantly notify all clients to hide widget
        UWorld* World = GetWorld();
        if (World)
        {
            for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
            {
                if (APlayerController* PC = It->Get())
                {
                    if (ADefaultPlayerController* DefaultPC = Cast<ADefaultPlayerController>(PC))
                    {
                        DefaultPC->ClientUpdateDetectionWidget(Detector, 0.0f, false); // Hide widget!
                    }
                }
            }
        }

        DetectionStates.Remove(Detector); // Remove state for this detector immediately
        LastSentStates.Remove(Detector);  // Clean up throttling state as well
        SetComponentTickEnabled(DetectionStates.Num() > 0);
    }

    OnDetectionEnded.Broadcast(GetOwner());
}

float UDetectionComponent::GetDetectionProgress(AActor* Detector) const
{
    const FDetectionState* State = DetectionStates.Find(Detector);
    if (!State) return 0.f;
    return FMath::Clamp(State->Progress / FMath::Max(DetectionDuration, 0.01f), 0.f, 1.f);
}

bool UDetectionComponent::IsDetectionInProgress(AActor* Detector) const
{
    const FDetectionState* State = DetectionStates.Find(Detector);
    return State ? State->bDetectionInProgress : false;
}

bool UDetectionComponent::IsFullyDetected(AActor* Detector) const
{
    const FDetectionState* State = DetectionStates.Find(Detector);
    return State ? State->bFullyDetected : false;
}

TArray<AActor*> UDetectionComponent::GetActiveDetectors() const
{
    TArray<AActor*> Result;
    for (const TPair<AActor*, FDetectionState>& Pair : DetectionStates)
    {
        Result.Add(Pair.Key);
    }
    return Result;
}

// --- Distance-based detection speed multiplier ---
float UDetectionComponent::GetDetectionSpeedMultiplier(AActor* Detector) const
{
    if (!Detector || !GetOwner())
        return 1.0f;

    float Distance = FVector::Dist(Detector->GetActorLocation(), GetOwner()->GetActorLocation());

    // Avoid division by zero, and never go "infinite" speed
    float EffectiveDistance = FMath::Max(Distance, 1.0f);

    // Simple inverse formula: speed is higher when closer, slower when further
    float Speed = BaseDetectionSpeedMultiplier / EffectiveDistance;

    // Optional: Clamp to minimum/maximum if you want to be sure it's reasonable
    return FMath::Clamp(Speed, 0.1f, 10.0f);
}
// -------------------------------------------------

void UDetectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    bool bAnyActive = false;

    for (auto It = DetectionStates.CreateIterator(); It; ++It)
    {
        AActor* Detector = It.Key();
        FDetectionState& State = It.Value();

        // If detection in progress and not fully detected
        if (State.bDetectionInProgress && !State.bFullyDetected)
        {
            // Use distance-based detection speed
            float DetectionSpeedMultiplier = GetDetectionSpeedMultiplier(Detector);
            State.Progress += DeltaTime * DetectionSpeedMultiplier;

            if (State.Progress >= DetectionDuration)
            {
                State.Progress = DetectionDuration;
                State.bDetectionInProgress = false;
                State.bFullyDetected = true;
                State.Direction = 0;
                HandleFullyDetected(Detector);
            }
            bAnyActive = true;
        }
        // Decay phase
        else if (!State.bDetectionInProgress && !State.bFullyDetected)
        {
            if (State.Progress > 0.f)
            {
                State.Progress = FMath::Max(0.f, State.Progress - DeltaTime);
                State.Direction = (State.Progress > 0.f) ? -1 : 0;
                if (State.Progress > 0.f) bAnyActive = true;
            }
            else
            {
                State.Direction = 0;
            }
        }
        // Fully detected phase (bar stays full forever, or until force-removed)
        else if (State.bFullyDetected)
        {
            State.Progress = DetectionDuration; // force bar full
            bAnyActive = true;
        }

        // --- THROTTLE NETWORK: Only send to client if changed (≥1%) or lock state flips ----
        float ProgressPct = FMath::Clamp(State.Progress / FMath::Max(DetectionDuration, 0.01f), 0.f, 1.f);
        bool bIsLocked = State.bFullyDetected;

        FLastSentState& LastSent = LastSentStates.FindOrAdd(Detector);
        bool bSendUpdate = false;

        if (FMath::Abs(LastSent.Progress - ProgressPct) >= 0.01f || LastSent.bIsLocked != bIsLocked)
        {
            LastSent.Progress = ProgressPct;
            LastSent.bIsLocked = bIsLocked;
            bSendUpdate = true;
        }

        if (bSendUpdate)
        {
            if (!CachedOwner.IsValid())
            {
                CachedOwner = Cast<ADefaultCharacter>(GetOwner());
                CachedController = nullptr;
            }
            if (CachedOwner.IsValid())
            {
                ADefaultPlayerController* DefaultPC = CachedController.IsValid() ? CachedController.Get() : nullptr;
                if (!DefaultPC)
                {
                    DefaultPC = Cast<ADefaultPlayerController>(CachedOwner->GetController());
                    CachedController = DefaultPC;
                }
                if (DefaultPC)
                {
                    DefaultPC->ClientUpdateDetectionWidget(Detector, ProgressPct, bIsLocked);
                }
            }
        }

        // Clean up the throttling entry if this detector is cleared
        if (ProgressPct <= 0.001f && !bIsLocked)
        {
            LastSentStates.Remove(Detector); // So the next detection start will always resend
        }
    }

    SetComponentTickEnabled(bAnyActive);
}

void UDetectionComponent::HandleFullyDetected(AActor* Detector)
{
    if (Detector && Detector->GetClass()->ImplementsInterface(UDetectable::StaticClass()))
    {
        IDetectable::Execute_OnFullyDetected(Detector, GetOwner());
    }
    // OnDetectionBegan already fired at start, OnDetectionEnded already fires on StopDetection
}
