#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HackComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHackComplete);

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPLITSTREAM_API UHackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHackComponent();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(BlueprintAssignable, Category = "Hacking")
    FOnHackComplete OnHackComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hacking")
    float HackDuration = 10.f;

    UPROPERTY(ReplicatedUsing = OnRep_Hacked, EditAnywhere, BlueprintReadWrite, Category = "Hacking")
    bool bHacked = false;

    UFUNCTION()
    void OnRep_Hacked();

    void SetHacked();

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void Interact(AActor* Interactor);

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void CancelInteract(AActor* Interactor);

protected:
    virtual void BeginPlay() override;
};