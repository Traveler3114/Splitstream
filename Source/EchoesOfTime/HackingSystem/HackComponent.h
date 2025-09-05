#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "HackComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHackComplete);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ECHOESOFTIME_API UHackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHackComponent();

    UPROPERTY(BlueprintAssignable, Category = "Hacking")
    FOnHackComplete OnHackComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hacking")
    float HackDuration = 10.f; // seconds

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hacking")
    bool bHackingInProgress = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hacking")
    bool bHacked = false;

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void StartHacking(float Duration);

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void CancelHacking();

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    float GetHackProgress() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    float HackElapsed = 0.f;
};