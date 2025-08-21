#pragma once

#include "CoreMinimal.h"
#include "Actors/DoorBase.h"
#include "PastDoor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDoorStateChanged, bool, bIsOpen);

UCLASS()
class ECHOESOFTIME_API APastDoor : public ADoorBase
{
    GENERATED_BODY()

public:
    APastDoor();

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(BlueprintAssignable, Category = "PastDoor")
    FOnDoorStateChanged OnDoorStateChanged;

    // If you want unique meshes add here, but you can also use inherited DoorMesh
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* StaticMesh1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* StaticMesh2;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* StaticMesh3;

    void OnRep_IsOpen() override;
};