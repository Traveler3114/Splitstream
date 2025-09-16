#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HackComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHackComplete);

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ECHOESOFTIME_API UHackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHackComponent();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(BlueprintAssignable, Category = "Hacking")
    FOnHackComplete OnHackComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hacking")
    float HackDuration = 10.f; // seconds

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Hacking")
    bool bHackingInProgress = false;

    UPROPERTY(ReplicatedUsing = OnRep_Hacked, VisibleAnywhere, BlueprintReadOnly, Category = "Hacking")
    bool bHacked = false;

    UFUNCTION()
    void OnRep_Hacked();

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void StartHacking();


    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void CancelHacking();

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    float GetHackProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void Interact(AActor* Interactor);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    float HackElapsed = 0.f;
};