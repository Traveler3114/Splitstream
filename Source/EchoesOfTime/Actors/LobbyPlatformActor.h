#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyPlatformActor.generated.h"

UCLASS()
class ECHOESOFTIME_API ALobbyPlatformActor : public AActor
{
    GENERATED_BODY()

public:
    ALobbyPlatformActor();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Platform")
    USceneComponent* RootScene;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Platform")
    UStaticMeshComponent* PlatformMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Platform")
    class UWidgetComponent* InviteWidget;
};