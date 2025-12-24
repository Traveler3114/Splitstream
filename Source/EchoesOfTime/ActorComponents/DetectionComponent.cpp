#include "DetectionComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/WidgetComponent.h"
#include "Widgets/DetectionActorWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Actor.h"

#define DETECTION_LOG(Verbosity, Format, ...) \
    UE_LOG(LogTemp, Verbosity, TEXT("[%s][%s][Role: %s] " Format), \
    *GetNameSafe(GetOwner()), *GetName(), \
    (GetOwnerRole() == ROLE_Authority ? TEXT("Authority") : (GetOwnerRole() == ROLE_AutonomousProxy ? TEXT("AutonomousProxy") : (GetOwnerRole() == ROLE_SimulatedProxy ? TEXT("SimulatedProxy") : TEXT("Unknown")))), \
    ##__VA_ARGS__ )

UDetectionComponent::UDetectionComponent()
{
    SetIsReplicatedByDefault(true);
    PrimaryComponentTick.bCanEverTick = true;

    DetectionWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("DetectionWidgetComp"));
    DetectionWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    DetectionWidgetComponent->SetDrawAtDesiredSize(true);
}

void UDetectionComponent::BeginPlay()
{
    Super::BeginPlay();

    if (DetectionWidgetComponent && GetOwner())
    {
        USceneComponent* Root = GetOwner()->GetRootComponent();
        DetectionWidgetComponent->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
        DetectionWidgetComponent->SetRelativeLocation(FVector(0, 0, 120));
        if (DetectionWidgetClass)
        {
            DetectionWidgetComponent->SetWidgetClass(DetectionWidgetClass);
        }
        DetectionWidgetComponent->SetVisibility(false, true);
    }

    DetectionElapsed = 0.f;
    bFullyDetected = false;
    bDetectionInProgress = false;
    CurrentDetector = nullptr;
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
}

void UDetectionComponent::StopDetection(AActor* Detector)
{
    if (GetOwnerRole() != ROLE_Authority) return;

    bDetectionInProgress = false; // Will now fall/drain in Tick
    // Don't reset DetectionElapsed here!
    SetComponentTickEnabled(true);
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
        }
    }
    else
    {
        // the fallback draining logic
        DetectionElapsed = FMath::Max(0.f, DetectionElapsed - DeltaTime);
        if (DetectionElapsed > 0.f)
            bShow = true;
        else
            SetComponentTickEnabled(false);
    }

    if (DetectionWidgetComponent)
        DetectionWidgetComponent->SetVisibility(bShow, true);

    UpdateWidget();
}

void UDetectionComponent::UpdateWidget()
{
    if (!DetectionWidgetComponent) return;
    if (!CachedWidget)
    {
        if (UUserWidget* BaseWidget = DetectionWidgetComponent->GetWidget())
            CachedWidget = Cast<UDetectionActorWidget>(BaseWidget);
    }
    if (!CachedWidget) return;

    float Progress = GetDetectionProgress();
    CachedWidget->SetDetectionProgress(Progress, bFullyDetected);
}

void UDetectionComponent::HandleFullyDetected()
{
    if (CurrentDetector && CurrentDetector->GetClass()->ImplementsInterface(UDetectable::StaticClass()))
    {
        IDetectable::Execute_OnFullyDetected(CurrentDetector, GetOwner());
    }
}