#include "LobbyPlatformActor.h"
#include "Components/SceneComponent.h"
#include "Widgets/Lobby/OpenFriendsListButton.h"
#include "Widgets/Lobby/FriendList.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerState.h"
#include "Components/WidgetComponent.h"
#include "Characters/DefaultCharacter.h"
#include "Widgets/Lobby/PlayerLobbyInfo.h"
#include "Components/ArrowComponent.h"
#include "Engine/World.h"
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
    FTransform SpawnTransform = SpawnPoint->GetComponentTransform();
    APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(CharacterClassToSpawn, SpawnTransform);
    OccupyingPawn = SpawnedPawn;
    ADefaultCharacter* DefaultChar = Cast<ADefaultCharacter>(OccupyingPawn);
    DefaultChar->PlayerInfoWidget->InitWidget(); // Ensure widget is created
    UUserWidget* UserWidget = DefaultChar->PlayerInfoWidget->GetUserWidgetObject();
    UPlayerLobbyInfo* LobbyInfo = Cast<UPlayerLobbyInfo>(UserWidget);
    FString PlayerName = NewController->PlayerState->GetPlayerName();
    LobbyInfo->SetPlayerName(FText::FromString(PlayerName));
	LobbyInfo->SetAvatarTexture(GetPlayerAvatar(NewController)); // Set avatar texture
    LobbyInfo->SetKickButtonVisible(HasAuthority());
    if (DefaultChar)
    {
        DefaultChar->ReplicatedPlayerName = PlayerName;
        DefaultChar->ReplicatedAvatarTexture = GetPlayerAvatar(NewController);
        // Optionally call OnRep_PlayerInfo() on the server to update the server's widget immediately
        DefaultChar->OnRep_PlayerInfo();
    }
    bIsOccupied = true;
    OnRep_IsOccupied(); // Update server immediately
    return SpawnedPawn;
}
UTexture2D* ALobbyPlatformActor::GetPlayerAvatar_Implementation(AController* NewController)
{
    // Provide your default logic here, or just return nullptr if you only want Blueprint to handle it
    return nullptr;
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