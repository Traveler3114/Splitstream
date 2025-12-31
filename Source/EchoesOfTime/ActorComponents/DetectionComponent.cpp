#include "DetectionComponent.h"
#include "Net/UnrealNetwork.h"
#include "Interfaces/IDetectable.h"
#include "DetectionRegistry.h"
#include "GameFramework/Actor.h"

UDetectionComponent::UDetectionComponent()
{
    SetIsReplicatedByDefault(true);
    PrimaryComponentTick.bCanEverTick = true;
}

void UDetectionComponent::BeginPlay()
{
    Super::BeginPlay();

    DetectionElapsed = 0.f;
    bFullyDetected = false;
    bDetectionInProgress = false;
    CurrentDetector = nullptr;
}

void UDetectionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // Unregister from registry if still present
    MulticastUpdateRegistry(false);
}

void UDetectionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UDetectionComponent, bDetectionInProgress);
    DOREPLIFETIME(UDetectionComponent, bFullyDetected);
    DOREPLIFETIME(UDetectionComponent, CurrentDetector);
}

void UDetectionComponent::StartDetection(AActor* Detector)
{
    if (!GetOwner() || !Detector) return;
    if (GetOwnerRole() != ROLE_Authority) return;

    CurrentDetector = Detector;
    bFullyDetected = false;
    bDetectionInProgress = true;
    MulticastResetDetectionElapsed();
    SetComponentTickEnabled(true);

    MulticastUpdateRegistry(true);
    OnDetectionBegan.Broadcast(GetOwner());
}

void UDetectionComponent::StopDetection(AActor* Detector)
{
    if (GetOwnerRole() != ROLE_Authority) return;

    bDetectionInProgress = false;
    SetComponentTickEnabled(true);
    OnDetectionEnded.Broadcast(GetOwner());
}

void UDetectionComponent::ForceImmediateDetectionEnd(AActor* Detector)
{
    // Instantly kill detection with NO reversal/decay, unregister and UI/registry cleanup
    if (GetOwnerRole() != ROLE_Authority) return;

    bDetectionInProgress = false;
    bFullyDetected = false;
    DetectionElapsed = 0.f;
    SetComponentTickEnabled(false); // we’re done ticking now!

    MulticastUpdateRegistry(false); // removes from DetectionRegistry, client clears widget
    OnDetectionEnded.Broadcast(GetOwner());
}

void UDetectionComponent::MulticastResetDetectionElapsed_Implementation()
{
    DetectionElapsed = 0.f;
    SetComponentTickEnabled(true);
}

float UDetectionComponent::GetDetectionProgress() const
{
    return FMath::Clamp(DetectionElapsed / FMath::Max(DetectionDuration, 0.01f), 0.f, 1.f);
}

void UDetectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    bool bShow = false;
    if (bDetectionInProgress)
    {
        DetectionElapsed += DeltaTime;
        if (DetectionElapsed >= DetectionDuration)
        {
            DetectionElapsed = DetectionDuration;
            bDetectionInProgress = false;
            bFullyDetected = true;
            FullyDetectedElapsed = 0.f; // Start "hold bar on full" timer
            SetComponentTickEnabled(true); // Keep ticking until bar hide
            HandleFullyDetected();
        }
        bShow = true;
    }
    else if (bFullyDetected)
    {
        FullyDetectedElapsed += DeltaTime;
        DetectionElapsed = DetectionDuration; // force bar to stay full
        bShow = true;
        if (FullyDetectedElapsed >= DetectionBarHideDelay)
        {
            bFullyDetected = false;
            DetectionElapsed = 0.f;
            FullyDetectedElapsed = 0.f;
            SetComponentTickEnabled(false);
            bShow = false;
            MulticastUpdateRegistry(false);
        }
    }
    else
    {
        DetectionElapsed = FMath::Max(0.f, DetectionElapsed - DeltaTime);
        if (DetectionElapsed > 0.f)
            bShow = true;
        else
            SetComponentTickEnabled(false);
    }

    // NO WidgetComponent calls, UI is handled by HUD/player via registry
}

void UDetectionComponent::MulticastUpdateRegistry_Implementation(bool bRegister)
{
    if (GetWorld())
    {
        UDetectionRegistry* Registry = GetWorld()->GetSubsystem<UDetectionRegistry>();
        if (Registry)
        {
            if (bRegister)
            {
                Registry->RegisterDetectedActor(GetOwner());
            }
            else
            {
                Registry->UnregisterDetectedActor(GetOwner());
            }
        }
    }
}

void UDetectionComponent::HandleFullyDetected()
{
    if (CurrentDetector && CurrentDetector->GetClass()->ImplementsInterface(UDetectable::StaticClass()))
    {
        IDetectable::Execute_OnFullyDetected(CurrentDetector, GetOwner());
    }
    // Registry and OnDetectionBegan still already notified
}