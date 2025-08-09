#include "LobbyPlatformActor.h"
#include "Components/SceneComponent.h"
#include "Widgets/Lobby/OpenFriendsListButton.h"
#include "Widgets/Lobby/FriendList.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/ArrowComponent.h"
#include "Net/UnrealNetwork.h"

ALobbyPlatformActor::ALobbyPlatformActor()
{
    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;

    PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
    PlatformMesh->SetupAttachment(RootComponent);

    OpenFriendsListButtonWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OpenFriendsListButtonWidget"));
    OpenFriendsListButtonWidget->SetupAttachment(RootComponent);
    OpenFriendsListButtonWidget->SetWidgetSpace(EWidgetSpace::Screen);

    FriendListWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("FriendListWidget"));
    FriendListWidget->SetupAttachment(RootComponent);
    FriendListWidget->SetWidgetSpace(EWidgetSpace::Screen);

    // Add arrow component for spawn location
    SpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint"));
    SpawnPoint->SetupAttachment(RootComponent);


    OpenFriendsListButtonWidget->SetVisibility(true);
    FriendListWidget->SetVisibility(false);
}

void ALobbyPlatformActor::BeginPlay()
{
    Super::BeginPlay();

    if (UOpenFriendsListButton* ButtonWidget = Cast<UOpenFriendsListButton>(OpenFriendsListButtonWidget->GetUserWidgetObject()))
    {
        ButtonWidget->OnShowFriendListRequested.AddDynamic(this, &ALobbyPlatformActor::ShowFriendList);
    }

    if (UFriendList* FriendList = Cast<UFriendList>(FriendListWidget->GetUserWidgetObject()))
    {
        FriendList->OnShowButtonRequested.AddDynamic(this, &ALobbyPlatformActor::ShowButton);
    }
}

void ALobbyPlatformActor::ShowFriendList()
{
    if (OpenFriendsListButtonWidget) OpenFriendsListButtonWidget->SetVisibility(false);
    if (FriendListWidget) FriendListWidget->SetVisibility(true);
}

void ALobbyPlatformActor::ShowButton()
{
    if (OpenFriendsListButtonWidget) OpenFriendsListButtonWidget->SetVisibility(true);
    if (FriendListWidget) FriendListWidget->SetVisibility(false);
}

APawn* ALobbyPlatformActor::SpawnCharacterAtPlatform(AController* NewController)
{
    if (!CharacterClassToSpawn || !SpawnPoint || !GetWorld())
        return nullptr;

    FTransform SpawnTransform = SpawnPoint->GetComponentTransform();
    APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(CharacterClassToSpawn, SpawnTransform);
    OccupyingPawn = SpawnedPawn;

    bIsOccupied = true;
    OnRep_IsOccupied(); // Update server immediately

    return SpawnedPawn;
}


// Replication setup
void ALobbyPlatformActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALobbyPlatformActor, bIsOccupied);
}

void ALobbyPlatformActor::OnRep_IsOccupied()
{
    if (OpenFriendsListButtonWidget)
    {
        OpenFriendsListButtonWidget->SetVisibility(!bIsOccupied);
    }
}