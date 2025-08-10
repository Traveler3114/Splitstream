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
    FTransform SpawnTransform = SpawnPoint->GetComponentTransform();
    APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(CharacterClassToSpawn, SpawnTransform);
    OccupyingPawn = SpawnedPawn;

    if (NewController && NewController->PlayerState)
    {
        ReplicatedPlayerName = NewController->PlayerState->GetPlayerName();
        ReplicatedAvatarTexture = GetPlayerAvatar(NewController);
    }

    if (OccupyingPawn && PlayerInfoWidget)
    {
        FVector PawnLocation = OccupyingPawn->GetActorLocation();
        FVector WidgetLocation = PawnLocation + FVector(0.f, 0.f, 120.f);
        PlayerInfoWidget->SetWorldLocation(WidgetLocation);
    }

    OnRep_PlayerInfo();

    bIsOccupied = true;
    OnRep_IsOccupied();

    return SpawnedPawn;
}

UTexture2D* ALobbyPlatformActor::GetPlayerAvatar_Implementation(AController* NewController)
{
    return nullptr;
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

void ALobbyPlatformActor::OnRep_PlayerInfo()
{
    if (PlayerInfoWidget)
    {
        PlayerInfoWidget->InitWidget();
        UUserWidget* UserWidget = PlayerInfoWidget->GetUserWidgetObject();
        if (UPlayerLobbyInfo* LobbyInfo = Cast<UPlayerLobbyInfo>(UserWidget))
        {
            LobbyInfo->SetPlayerName(FText::FromString(ReplicatedPlayerName));
            LobbyInfo->SetAvatarTexture(ReplicatedAvatarTexture);
            LobbyInfo->SetKickButtonVisible(HasAuthority());
            LobbyInfo->SetTeamTag(TeamTag);
        }
    }
}


void ALobbyPlatformActor::SetPlayerReadyState(bool bReady)
{
    if (HasAuthority())
    {
        bIsReady = bReady;
        OnRep_ReadyState(); // Update server's own UI immediately
    }
}

void ALobbyPlatformActor::OnRep_ReadyState()
{
    if (PlayerInfoWidget)
    {
        PlayerInfoWidget->InitWidget();
        UUserWidget* UserWidget = PlayerInfoWidget->GetUserWidgetObject();
        if (UPlayerLobbyInfo* LobbyInfo = Cast<UPlayerLobbyInfo>(UserWidget))
        {
            LobbyInfo->SetReadyState(bIsReady);
        }
    }
}

void ALobbyPlatformActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALobbyPlatformActor, bIsOccupied);
    DOREPLIFETIME(ALobbyPlatformActor, ReplicatedPlayerName);
    DOREPLIFETIME(ALobbyPlatformActor, ReplicatedAvatarTexture);
    DOREPLIFETIME(ALobbyPlatformActor, bIsReady); 
    DOREPLIFETIME(ALobbyPlatformActor, TeamTag);
}
