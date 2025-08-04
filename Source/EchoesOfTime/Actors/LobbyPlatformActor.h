#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyPlatformActor.generated.h"


class UWidgetComponent;
UCLASS()
class ECHOESOFTIME_API ALobbyPlatformActor : public AActor
{
    GENERATED_BODY()

public:
    ALobbyPlatformActor();
	virtual void BeginPlay() override;
    UFUNCTION()
    void ShowFriendList();
    UFUNCTION()
    void ShowButton();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Platform")
    USceneComponent* RootScene;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Platform")
    UStaticMeshComponent* PlatformMesh;

    UPROPERTY(VisibleAnywhere)
    UWidgetComponent* OpenFriendsListButtonWidget;

    UPROPERTY(VisibleAnywhere)
    UWidgetComponent* FriendListWidget;
};