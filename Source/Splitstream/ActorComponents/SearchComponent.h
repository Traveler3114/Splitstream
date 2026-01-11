#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SearchComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSearchComplete);

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPLITSTREAM_API USearchComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USearchComponent();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(BlueprintAssignable, Category = "Searching")
    FOnSearchComplete OnSearchComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Searching")
    float SearchDuration = 10.f; // seconds

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Searching")
    bool bSearchingInProgress = false;

    UPROPERTY(ReplicatedUsing = OnRep_Searched, VisibleAnywhere, BlueprintReadOnly, Category = "Searching")
    bool bSearched = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Searching")
    bool bAllowMultipleSearches = false;

    UFUNCTION()
    void OnRep_Searched();

    UFUNCTION(BlueprintCallable, Category = "Searching")
    void StartSearching();


    UFUNCTION(BlueprintCallable, Category = "Searching")
    void CancelSearching();

    UFUNCTION(BlueprintCallable, Category = "Searching")
    float GetSearchProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Searching")
    void Interact(AActor* Interactor);

    UFUNCTION(BlueprintCallable, Category = "Searching")
    void CancelInteract(AActor* Interactor);

    TWeakObjectPtr<AActor> LastInteractor;

    UFUNCTION(NetMulticast, Reliable)
    void MulticastResetSearchElapsed();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    float SearchElapsed = 0.f;
};