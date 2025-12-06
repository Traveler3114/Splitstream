#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SearchComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSearchComplete);

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ECHOESOFTIME_API USearchComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USearchComponent();

    // ============================================
    // Unreal Engine Overrides
    // ============================================
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ============================================
    // Configuration
    // ============================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Searching")
    float SearchDuration = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Searching")
    bool bAllowMultipleSearches = false;

    // ============================================
    // Search State
    // ============================================
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Searching")
    bool bSearchingInProgress = false;

    UPROPERTY(ReplicatedUsing = OnRep_Searched, VisibleAnywhere, BlueprintReadOnly, Category = "Searching")
    bool bSearched = false;

    TWeakObjectPtr<AActor> LastInteractor;

    // ============================================
    // Events
    // ============================================
    UPROPERTY(BlueprintAssignable, Category = "Searching")
    FOnSearchComplete OnSearchComplete;

    // ============================================
    // Search Actions
    // ============================================
    UFUNCTION(BlueprintCallable, Category = "Searching")
    void StartSearching();

    UFUNCTION(BlueprintCallable, Category = "Searching")
    void CancelSearching();

    UFUNCTION(BlueprintCallable, Category = "Searching")
    void Interact(AActor* Interactor);

    UFUNCTION(BlueprintCallable, Category = "Searching")
    void CancelInteract(AActor* Interactor);

    // ============================================
    // Query Functions
    // ============================================
    UFUNCTION(BlueprintCallable, Category = "Searching")
    float GetSearchProgress() const;

    // ============================================
    // Network RPCs
    // ============================================
    UFUNCTION(NetMulticast, Reliable)
    void MulticastResetSearchElapsed();

    UFUNCTION()
    void OnRep_Searched();

protected:
    // ============================================
    // Internal State
    // ============================================
    float SearchElapsed = 0.f;
};