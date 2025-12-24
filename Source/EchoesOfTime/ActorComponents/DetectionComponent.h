#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetComponent.h"
#include "DetectionComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ECHOESOFTIME_API UDetectionComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UDetectionComponent();

    /** The Widget Component displaying the detection bar */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category="Detection")
    UWidgetComponent* DetectionWidgetComponent;

    /** Who is currently detecting this actor */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
    AActor* CurrentDetector = nullptr;

    /** Progress for detection bar [0..1] */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_DetectionProgress)
    float DetectionProgress = 0.f;

    /** True if detection is fully completed */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_IsFullyDetected)
    bool bIsFullyDetected = false;

    /** Seconds to fully detect */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Detection")
    float DetectionDuration = 2.5f;

    /** Widget class to use for the detection bar */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Detection")
    TSubclassOf<UUserWidget> DetectionWidgetClass;

    /** Set the widget class for the detection bar */
    UFUNCTION(BlueprintCallable, Category="Detection")
    void SetDetectionWidgetClass(TSubclassOf<UUserWidget> WidgetClass);

    /** Should detection progress widget be shown? */
    UFUNCTION(BlueprintCallable, Category="Detection")
    bool ShouldShowDetectionWidget() const;

    /** Detection progress [0..1] */
    UFUNCTION(BlueprintCallable, Category="Detection")
    float GetDetectionProgressPercent() const;

    /** Detection bar color */
    UFUNCTION(BlueprintCallable, Category="Detection")
    FLinearColor GetDetectionBarColor() const;

    /** Call when someone starts detecting this actor */
    UFUNCTION(BlueprintCallable, Category="Detection")
    void OnDetected(AActor* Detector);

    /** Call when someone stops detecting this actor */
    UFUNCTION(BlueprintCallable, Category="Detection")
    void OnLost(AActor* Detector);

    /** Directly updates the widget progress bar (C++-only) */
    void UpdateDetectionWidget();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Timer logic
    FTimerHandle DetectionTickHandle;
    bool bIsLosingSight = false;
    void DetectionProgressTick();

    void OnFullyDetectedEvent();

    // OnRep functions for replicated variables
    UFUNCTION()
    void OnRep_DetectionProgress();

    UFUNCTION()
    void OnRep_IsFullyDetected();
};