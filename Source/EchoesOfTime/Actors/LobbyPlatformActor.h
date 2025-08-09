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

    UFUNCTION(BlueprintCallable, Category = "Platform")
    APawn* SpawnCharacterAtPlatform(AController* NewController);

    APawn* OccupyingPawn;


protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Platform")
    USceneComponent* RootScene;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Platform")
    UStaticMeshComponent* PlatformMesh;

    UPROPERTY(VisibleAnywhere)
    UWidgetComponent* OpenFriendsListButtonWidget;

    UPROPERTY(VisibleAnywhere)
    UWidgetComponent* FriendListWidget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
    class UArrowComponent* SpawnPoint;

    // Character class to spawn, assignable in Blueprint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
    TSubclassOf<APawn> CharacterClassToSpawn;

    // Add to your ALobbyPlatformActor class
    UPROPERTY(ReplicatedUsing = OnRep_IsOccupied)
    bool bIsOccupied = false;

    UFUNCTION()
    void OnRep_IsOccupied();


};