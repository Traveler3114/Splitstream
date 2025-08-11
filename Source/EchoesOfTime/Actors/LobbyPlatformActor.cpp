#include "LobbyPlatformActor.h"
#include "Components/SceneComponent.h"
#include "Widgets/Lobby/OpenFriendsListButton.h"
#include "Widgets/Lobby/FriendList.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerState.h"
#include "Components/WidgetComponent.h"
#include "Widgets/Lobby/PlayerLobbyInfo.h"
#include "Components/ArrowComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTagContainer.h"

#include "GameModes/LobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "DefaultPlayerState.h"
#include "AbilitySystemComponent.h"

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

    SpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPoint"));
    SpawnPoint->SetupAttachment(RootComponent);

    OpenFriendsListButtonWidget->SetVisibility(true);
    FriendListWidget->SetVisibility(false);

    PlayerInfoWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerInfoWidget"));
    PlayerInfoWidget->SetupAttachment(RootComponent);
    PlayerInfoWidget->SetWidgetSpace(EWidgetSpace::Screen);
    PlayerInfoWidget->SetDrawSize(FVector2D(500.f, 500.f));
    PlayerInfoWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
    PlayerInfoWidget->SetPivot(FVector2D(0.5f, 0.5f));
    PlayerInfoWidget->SetBlendMode(EWidgetBlendMode::Transparent);
    PlayerInfoWidget->SetTwoSided(true);


    SetNetUpdateFrequency(20.f);
    SetMinNetUpdateFrequency(20.f);
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
    if (UPlayerLobbyInfo* PlayerInfo = Cast<UPlayerLobbyInfo>(PlayerInfoWidget->GetUserWidgetObject()))
    {
        // Bind the widget's delegate to the platform's handler (NO parentheses!)
        PlayerInfo->OnKickRequested.AddDynamic(this, &ALobbyPlatformActor::HandleKickRequested);
    }
}

void ALobbyPlatformActor::HandleKickRequested()
{
    // Broadcast to game mode (or whoever is listening)
    OnKickRequestedPlatform.Broadcast(this);
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
    FTransform SpawnTransform = SpawnPoint->GetComponentTransform();
    APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(CharacterClassToSpawn, SpawnTransform);
    OccupyingPawn = SpawnedPawn;

    if (OccupyingPawn && PlayerInfoWidget)
    {
        FVector PawnLocation = OccupyingPawn->GetActorLocation();
        FVector WidgetLocation = PawnLocation + FVector(0.f, 0.f, 120.f);
        PlayerInfoWidget->SetWorldLocation(WidgetLocation);
        PlayerInfoWidget->InitWidget(); // ensure widget instance exists on clients
    }

    bIsOccupied = true;
    OnRep_IsOccupied();

    // Push replication right away so widgets become visible promptly
    ForceNetUpdate();

    return SpawnedPawn;
}

void ALobbyPlatformActor::OnRep_IsOccupied()
{
    if (OpenFriendsListButtonWidget)
    {
        OpenFriendsListButtonWidget->SetVisibility(!bIsOccupied);
    }
    if (PlayerInfoWidget)
    {
        PlayerInfoWidget->SetVisibility(bIsOccupied);
    }
}
void ALobbyPlatformActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALobbyPlatformActor, bIsOccupied);
}