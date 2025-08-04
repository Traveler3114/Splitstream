#include "LobbyPlatformActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"

ALobbyPlatformActor::ALobbyPlatformActor()
{
    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;

    PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
    PlatformMesh->SetupAttachment(RootComponent);

    InviteWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InviteWidget"));
    InviteWidget->SetupAttachment(RootComponent);
    InviteWidget->SetWidgetSpace(EWidgetSpace::Screen); // Or World, as needed
    // Set the widget class in Blueprint or here if you have a reference
}