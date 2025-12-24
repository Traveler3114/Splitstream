#include "DetectionComponent.h"
#include "Net/UnrealNetwork.h"
#include "Interfaces/IDetectable.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/DetectionActorWidget.h"

UDetectionComponent::UDetectionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    DetectionDuration = 2.5f;
    DetectionProgress = 0.0f;
    bIsFullyDetected = false;
    bIsLosingSight = false;
    CurrentDetector = nullptr;

    DetectionWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("DetectionWidgetComp"));
    DetectionWidgetComponent->SetIsReplicated(true);
}

void UDetectionComponent::BeginPlay()
{
    Super::BeginPlay();

    if (DetectionWidgetComponent && GetOwner())
    {
        USceneComponent* RootComp = GetOwner()->GetRootComponent();
        if (RootComp)
        {
            DetectionWidgetComponent->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);
            DetectionWidgetComponent->SetRelativeLocation(FVector(0, 0, 120));
        }
        if (DetectionWidgetClass)
        {
            DetectionWidgetComponent->SetWidgetClass(DetectionWidgetClass);
        }
    }

    if (GetOwner() && GetOwner()->HasAuthority())
    {
        GetWorld()->GetTimerManager().SetTimer(DetectionTickHandle, this, &UDetectionComponent::DetectionProgressTick, 0.05f, true);
    }
}

void UDetectionComponent::OnFullyDetectedEvent()
{
    if (CurrentDetector)
    {
        if (CurrentDetector->GetClass()->ImplementsInterface(UDetectable::StaticClass()))
        {
            IDetectable::Execute_OnFullyDetected(CurrentDetector, GetOwner());
        }
    }
}

void UDetectionComponent::SetDetectionWidgetClass(TSubclassOf<UUserWidget> WidgetClass)
{
    DetectionWidgetClass = WidgetClass;
    if (DetectionWidgetComponent && DetectionWidgetClass)
    {
        DetectionWidgetComponent->SetWidgetClass(DetectionWidgetClass);
    }
}

void UDetectionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UDetectionComponent, DetectionWidgetComponent); // Note: this is actually not necessary, see below
    DOREPLIFETIME(UDetectionComponent, CurrentDetector);
    DOREPLIFETIME_CONDITION_NOTIFY(UDetectionComponent, DetectionProgress, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UDetectionComponent, bIsFullyDetected, COND_None, REPNOTIFY_Always);
}

void UDetectionComponent::OnRep_DetectionProgress()
{
    UpdateDetectionWidget();
}

void UDetectionComponent::OnRep_IsFullyDetected()
{
    UpdateDetectionWidget();
}

void UDetectionComponent::OnDetected(AActor* Detector)
{
    if (!Detector)
        return;
    if (CurrentDetector == nullptr || CurrentDetector == Detector)
    {
        CurrentDetector = Detector;
        bIsLosingSight = false;
    }
}

void UDetectionComponent::OnLost(AActor* Detector)
{
    if (CurrentDetector && Detector == CurrentDetector)
    {
        bIsLosingSight = true;
    }
}

void UDetectionComponent::DetectionProgressTick()
{
    if (CurrentDetector)
    {
        bool bCanDetect = false;
        if (CurrentDetector->GetClass()->ImplementsInterface(UDetectable::StaticClass()))
        {
            bCanDetect = IDetectable::Execute_IsActorAlreadyDetected(CurrentDetector, GetOwner());
        }

        float dt = 0.05f;
        float rate = dt / FMath::Max(DetectionDuration, 0.01f);

        if (bCanDetect && !bIsLosingSight && !bIsFullyDetected)
        {
            DetectionProgress = FMath::Min(DetectionProgress + rate, 1.0f);
            if (DetectionProgress >= 1.0f && !bIsFullyDetected)
            {
                DetectionProgress = 1.0f;
                bIsFullyDetected = true;
                OnFullyDetectedEvent();
            }
        }
        else if ((!bCanDetect || bIsLosingSight) && !bIsFullyDetected)
        {
            DetectionProgress = FMath::Max(DetectionProgress - rate * 0.5f, 0.0f);
            if (DetectionProgress <= 0.0f)
            {
                DetectionProgress = 0.0f;
                CurrentDetector = nullptr;
                bIsLosingSight = false;
            }
        }
    }
    else
    {
        DetectionProgress = 0.f;
        bIsLosingSight = false;
    }

    if (DetectionWidgetComponent)
    {
        DetectionWidgetComponent->SetVisibility(ShouldShowDetectionWidget());
    }

    UpdateDetectionWidget();
}

void UDetectionComponent::UpdateDetectionWidget()
{
    if (!DetectionWidgetComponent)
        return;
    UUserWidget* Widget = DetectionWidgetComponent->GetWidget();
    if (!Widget)
        return;

    UDetectionActorWidget* DetectionWidget = Cast<UDetectionActorWidget>(Widget);
    if (!DetectionWidget)
        return;

    DetectionWidget->SetDetectionProgress(DetectionProgress, bIsFullyDetected);
}

bool UDetectionComponent::ShouldShowDetectionWidget() const
{
    return DetectionProgress > 0.01f || bIsFullyDetected;
}

float UDetectionComponent::GetDetectionProgressPercent() const
{
    return DetectionProgress;
}

FLinearColor UDetectionComponent::GetDetectionBarColor() const
{
    return DetectionProgress >= 1.0f ? FLinearColor::Red : FLinearColor(1.f, 0.85f, 0.15f);
}