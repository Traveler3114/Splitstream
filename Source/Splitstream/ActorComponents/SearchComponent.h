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
    float SearchDuration = 10.f;

    UPROPERTY(ReplicatedUsing = OnRep_Searched, VisibleAnywhere, BlueprintReadOnly, Category = "Searching")
    bool bSearched = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Searching")
    bool bAllowMultipleSearches = false;

    UFUNCTION()
    void OnRep_Searched();

    void SetSearched();

    UFUNCTION(BlueprintCallable, Category = "Searching")
    void Interact(AActor* Interactor);

    UFUNCTION(BlueprintCallable, Category = "Searching")
    void CancelInteract(AActor* Interactor);

    TWeakObjectPtr<AActor> LastInteractor;

protected:
    virtual void BeginPlay() override;
};