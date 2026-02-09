#pragma once

#include "CoreMinimal.h"
#include "Actors/DoorBase.h"
#include "DoubleDoorBase.generated.h"

UCLASS(Abstract)
class SPLITSTREAM_API ADoubleDoorBase : public ADoorBase
{
    GENERATED_BODY()

public:
    ADoubleDoorBase();

    // Double doors have two meshes rather than one
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* DoorLeftMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* DoorRightMesh;

    // Override highlighting to apply to both doors.
    virtual void SetHighlighted_Implementation(bool bHighlight) override;
};