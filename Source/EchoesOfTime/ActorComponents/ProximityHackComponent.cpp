// ProximityHackComponent.cpp
#include "ProximityHackComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Widgets/ProximityHackWidget.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "ActorComponents/InventoryComponent.h"
#include "DataAssets/ItemBase.h"

UProximityHackComponent::UProximityHackComponent()
{
    SetIsReplicatedByDefault(true);

    // Base UHackComponent handles HackElapsed / completion.
    // We tick as well only to update the widget & run server proximity logic.
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.SetTickFunctionEnable(true);

    ProximityHackingTag = TAG_Character_Status_ProximityHacking;
}

void UProximityHackComponent::BeginPlay()
{
    Super::BeginPlay();
    OnHackComplete.AddDynamic(this, &UProximityHackComponent::HandleHackComplete);
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // --- Detection sphere (server authoritative) ---
    if (GetOwnerRole() == ROLE_Authority)
    {
        DetectionSphere = NewObject<USphereComponent>(Owner, TEXT("ProximityHackDetectionSphere"));
        if (DetectionSphere)
        {
            DetectionSphere->InitSphereRadius(ProximityRadius);
            DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            DetectionSphere->SetCollisionObjectType(ECC_WorldDynamic);

            // Adjust these channels to match your project (here we overlap pawns only).
            DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
            DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

            DetectionSphere->SetupAttachment(Owner->GetRootComponent());
            DetectionSphere->RegisterComponent();

            DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &UProximityHackComponent::OnDetectionBeginOverlap);
            DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &UProximityHackComponent::OnDetectionEndOverlap);
        }
    }

    // --- World-space widget (cosmetic, exists on all machines but is visible only to hacker) ---
    if (ProximityHackWidgetClass)
    {
        ProximityWidgetComponent = NewObject<UWidgetComponent>(Owner);
        if (ProximityWidgetComponent)
        {
            ProximityWidgetComponent->SetupAttachment(Owner->GetRootComponent());
            ProximityWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
            ProximityWidgetComponent->SetDrawAtDesiredSize(true);
            ProximityWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 150.f));

            // Set class BEFORE register, then init so widget instance is created.
            ProximityWidgetComponent->SetWidgetClass(ProximityHackWidgetClass);
            ProximityWidgetComponent->RegisterComponent();
            ProximityWidgetComponent->InitWidget();

            // Start hidden; TickComponent will decide visibility per-machine.
            ProximityWidgetComponent->SetVisibility(false, true);

            if (UUserWidget* Widget = ProximityWidgetComponent->GetUserWidgetObject())
            {
                ProximityHackWidgetInstance = Cast<UProximityHackWidget>(Widget);
            }
        }
    }
}

void UProximityHackComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UProximityHackComponent, ActiveHacker);
}

bool UProximityHackComponent::IsLocallyViewedByActiveHacker() const
{
    if (!ActiveHacker || !GetWorld())
    {
        return false;
    }

    // This assumes one local player per process (typical listen server / client).
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        APawn* LocalPawn = PC->GetPawn();
        return (LocalPawn == ActiveHacker);
    }

    return false;
}




void UProximityHackComponent::HandleHackComplete()
{
    // SERVER: give reward to the hacker if configured
    if (GetOwnerRole() == ROLE_Authority)
    {
        if (RewardItem && ActiveHacker)
        {
            // Only characters are expected to hack; adjust if you support other pawns.
            ACharacter* HackerCharacter = ActiveHacker;
            if (UInventoryComponent* Inventory = HackerCharacter->FindComponentByClass<UInventoryComponent>())
            {
                // New unique instance ID for this reward
                const FGuid NewInstanceID = FGuid::NewGuid();
                Inventory->AddItem(RewardItem, NewInstanceID);
            }
        }
    }

    // UI / cosmetic cleanup runs on all machines
    if (ProximityWidgetComponent)
    {
        ProximityWidgetComponent->SetVisibility(false, true);
    }

    if (ProximityHackWidgetInstance)
    {
        ProximityHackWidgetInstance->SetHackProgress(1.0f); // optional: show full once
    }

    // Clear tag from hacker
    RemoveProximityTagFrom(ActiveHacker);
}

void UProximityHackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Ensure we have a widget instance (in case it was created slightly later).
    if (!ProximityHackWidgetInstance && ProximityWidgetComponent)
    {
        if (UUserWidget* Widget = ProximityWidgetComponent->GetUserWidgetObject())
        {
            ProximityHackWidgetInstance = Cast<UProximityHackWidget>(Widget);
        }
    }

    // Server: maintain ActiveHacker from PlayersInRange.
    if (GetOwnerRole() == ROLE_Authority && !bHacked)
    {
        Server_UpdateProximity();
    }

    // Only show the widget if this machine's local pawn is the active hacker AND hacking is in progress.
    const bool bShouldShowWidget = (bHackingInProgress && IsLocallyViewedByActiveHacker());

    if (ProximityWidgetComponent)
    {
        ProximityWidgetComponent->SetVisibility(bShouldShowWidget, true);
    }

    // Only the hacker's local machine updates the widget.
    if (bShouldShowWidget && ProximityHackWidgetInstance)
    {
        ProximityHackWidgetInstance->SetHackProgress(GetHackProgress());
    }
}

// ------------------------------------------------------------------
// Overlap events (server only)
// ------------------------------------------------------------------

void UProximityHackComponent::OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (GetOwnerRole() != ROLE_Authority || !OtherActor)
    {
        return;
    }

    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (!Character || !Character->IsPlayerControlled())
    {
        return;
    }

    // Add to PlayersInRange if not already there.
    PlayersInRange.AddUnique(Character);

    // If nobody hacking yet and guard not already hacked, start with this one.
    if (!ActiveHacker && !bHacked)
    {
        StartHackingForPlayer(Character);
    }
}

void UProximityHackComponent::OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (GetOwnerRole() != ROLE_Authority || !OtherActor)
    {
        return;
    }

    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (!Character)
    {
        return;
    }

    PlayersInRange.RemoveSwap(Character);

    // If the leaving character was the active hacker, cancel and choose a new one if possible.
    if (Character == ActiveHacker)
    {
        CancelHackingForPlayer();

        if (!bHacked)
        {
            ACharacter* NewHacker = FindFirstPlayerInRange();
            if (NewHacker)
            {
                StartHackingForPlayer(NewHacker);
            }
        }
    }
}

// ------------------------------------------------------------------
// Proximity logic (server)
// ------------------------------------------------------------------

void UProximityHackComponent::Server_UpdateProximity()
{
    // Clean up invalid players (e.g., destroyed).
    PlayersInRange.RemoveAll([](const TWeakObjectPtr<ACharacter>& Ptr)
    {
        return !Ptr.IsValid();
    });

    // If there is an active hacker, ensure they are still in PlayersInRange.
    if (ActiveHacker)
    {
        const bool bStillInRange = PlayersInRange.Contains(ActiveHacker);
        if (!bStillInRange)
        {
            CancelHackingForPlayer();

            if (!bHacked)
            {
                ACharacter* NewHacker = FindFirstPlayerInRange();
                if (NewHacker)
                {
                    StartHackingForPlayer(NewHacker);
                }
            }
        }
        // else: still in range; base UHackComponent is progressing the hack.
        return;
    }

    // No active hacker: see if any players are still in range.
    if (!bHacked)
    {
        ACharacter* First = FindFirstPlayerInRange();
        if (First)
        {
            StartHackingForPlayer(First);
        }
    }
}

ACharacter* UProximityHackComponent::FindFirstPlayerInRange() const
{
    for (const TWeakObjectPtr<ACharacter>& WeakChar : PlayersInRange)
    {
        if (ACharacter* Char = WeakChar.Get())
        {
            if (Char->IsPlayerControlled())
            {
                return Char;
            }
        }
    }
    return nullptr;
}

void UProximityHackComponent::StartHackingForPlayer(ACharacter* NewHacker)
{
    if (!NewHacker || bHacked) return;

    // Only one hacker at a time. If someone is already hacking, ignore new player.
    if (ActiveHacker && ActiveHacker != NewHacker)
    {
        return;
    }

    ActiveHacker = NewHacker;

    // Apply ProximityHacking tag if desired.
    ApplyProximityTagTo(ActiveHacker);

    // Reuse parent logic: clears HackElapsed via multicast + sets bHackingInProgress and ticking.
    StartHacking();
}

void UProximityHackComponent::CancelHackingForPlayer()
{
    // Remove tag from the active hacker.
    RemoveProximityTagFrom(ActiveHacker);
    ActiveHacker = nullptr;

    // Use parent CancelHacking to stop ticking and reset elapsed.
    CancelHacking();

    // Explicitly reset the widget so the bar shows 0 after cancel.
    if (ProximityHackWidgetInstance)
    {
        ProximityHackWidgetInstance->SetHackProgress(0.0f);
    }

    // Hide widget.
    if (ProximityWidgetComponent)
    {
        ProximityWidgetComponent->SetVisibility(false, true);
    }
}

// ------------------------------------------------------------------
// GAS Tag helpers
// ------------------------------------------------------------------

void UProximityHackComponent::ApplyProximityTagTo(ACharacter* Player)
{
    if (!Player || !ProximityHackingTag.IsValid())
        return;

    if (IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Player))
    {
        if (UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent())
        {
            FGameplayTagContainer TagsToAdd;
            TagsToAdd.AddTag(ProximityHackingTag);
            ASC->AddLooseGameplayTags(TagsToAdd);
        }
    }
}

void UProximityHackComponent::RemoveProximityTagFrom(ACharacter* Player)
{
    if (!Player || !ProximityHackingTag.IsValid())
        return;

    if (IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(Player))
    {
        if (UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent())
        {
            FGameplayTagContainer TagsToRemove;
            TagsToRemove.AddTag(ProximityHackingTag);
            ASC->RemoveLooseGameplayTags(TagsToRemove);
        }
    }
}