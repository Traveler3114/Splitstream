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

    // Detection sphere (server only)
    if (GetOwnerRole() == ROLE_Authority)
    {
        DetectionSphere = NewObject<USphereComponent>(Owner, TEXT("ProximityHackDetectionSphere"));
        if (DetectionSphere)
        {
            DetectionSphere->InitSphereRadius(ProximityRadius);
            DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            DetectionSphere->SetCollisionObjectType(ECC_WorldDynamic);
            DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
            DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

            DetectionSphere->SetupAttachment(Owner->GetRootComponent());
            DetectionSphere->RegisterComponent();

            DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &UProximityHackComponent::OnDetectionBeginOverlap);
            DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &UProximityHackComponent::OnDetectionEndOverlap);
        }
    }

    // World-space widget on all machines
    if (ProximityHackWidgetClass)
    {
        ProximityWidgetComponent = NewObject<UWidgetComponent>(Owner);
        if (ProximityWidgetComponent)
        {
            ProximityWidgetComponent->SetupAttachment(Owner->GetRootComponent());
            ProximityWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
            ProximityWidgetComponent->SetDrawAtDesiredSize(true);
            ProximityWidgetComponent->SetRelativeLocation(FVector::ZeroVector);
            ProximityWidgetComponent->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));

            ProximityWidgetComponent->SetWidgetClass(ProximityHackWidgetClass);
            ProximityWidgetComponent->RegisterComponent();
            ProximityWidgetComponent->InitWidget();

            ProximityWidgetComponent->SetVisibility(false, true);

            ProximityWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
            float WidgetBaseSize = 100.0f; // match your widget's design size
            float Scale = (ProximityRadius * 2.0f) / WidgetBaseSize;
            ProximityWidgetComponent->SetWorldScale3D(FVector(1.f, Scale, Scale));

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
    DOREPLIFETIME(UProximityHackComponent, bDrainingHack);
}

bool UProximityHackComponent::IsLocallyViewedByActiveHacker() const
{
    if (!ActiveHacker || !GetWorld())
    {
        return false;
    }

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        APawn* LocalPawn = PC->GetPawn();
        return (LocalPawn == ActiveHacker);
    }

    return false;
}

void UProximityHackComponent::HandleHackComplete()
{
    // Server: reward hacker
    if (GetOwnerRole() == ROLE_Authority)
    {
        if (RewardItem && ActiveHacker)
        {
            if (UInventoryComponent* Inventory = ActiveHacker->FindComponentByClass<UInventoryComponent>())
            {
                const FGuid NewInstanceID = FGuid::NewGuid();
                Inventory->AddItem(RewardItem, NewInstanceID);
            }
        }
    }

    // Hide widget everywhere
    if (ProximityWidgetComponent)
    {
        ProximityWidgetComponent->SetVisibility(false, true);
    }

    if (ProximityHackWidgetInstance)
    {
        ProximityHackWidgetInstance->SetHackProgress(1.0f);
    }

    RemoveProximityTagFrom(ActiveHacker);
}

void UProximityHackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Ensure widget instance
    if (!ProximityHackWidgetInstance && ProximityWidgetComponent)
    {
        if (UUserWidget* Widget = ProximityWidgetComponent->GetUserWidgetObject())
        {
            ProximityHackWidgetInstance = Cast<UProximityHackWidget>(Widget);
        }
    }

    const bool bIsServer = (GetOwnerRole() == ROLE_Authority);

    // Server updates ActiveHacker selection (who is allowed to fill/drain)
    if (bIsServer && !bHacked)
    {
        Server_UpdateProximity();
    }

    // --- DRAINING LOGIC: opposite of filling, runs on any machine with this component ---
    if (bDrainingHack && HackDuration > 0.f)
    {
        // Filling is HackElapsed += DeltaTime (in UHackComponent::TickComponent when bHackingInProgress).
        // Here we do the opposite.
        HackElapsed = FMath::Max(0.f, HackElapsed - DeltaTime);

        if (HackElapsed <= 0.f)
        {
            HackElapsed = 0.f;
            bDrainingHack = false;
            bHackingInProgress = false;
            bHacked = false; // make sure it's not marked completed

            RemoveProximityTagFrom(ActiveHacker);
            ActiveHacker = nullptr;

            if (ProximityHackWidgetInstance)
            {
                ProximityHackWidgetInstance->SetHackProgress(0.0f);
            }
            if (ProximityWidgetComponent)
            {
                ProximityWidgetComponent->SetVisibility(false, true);
            }
        }
    }

    // Compute local progress from HackElapsed instead of GetHackProgress()
    float Progress = 0.f;
    if (HackDuration > 0.f)
    {
        Progress = FMath::Clamp(HackElapsed / HackDuration, 0.f, 1.f);
    }

    const bool bShouldShowWidget =
        ActiveHacker &&
        IsLocallyViewedByActiveHacker() &&
        (bHackingInProgress || bDrainingHack) &&
        (Progress > 0.f);

    if (ProximityWidgetComponent)
    {
        ProximityWidgetComponent->SetVisibility(bShouldShowWidget, true);
    }

    if (bShouldShowWidget && ProximityHackWidgetInstance)
    {
        ProximityHackWidgetInstance->SetHackProgress(Progress);
    }
}

// Overlap events (server only)

void UProximityHackComponent::OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (GetOwnerRole() != ROLE_Authority || !OtherActor)
        return;

    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (!Character || !Character->IsPlayerControlled())
        return;

    PlayersInRange.AddUnique(Character);

    if (!ActiveHacker && !bHacked)
    {
        StartHackingForPlayer(Character);
    }
}

void UProximityHackComponent::OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (GetOwnerRole() != ROLE_Authority || !OtherActor)
        return;

    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (!Character)
        return;

    PlayersInRange.RemoveSwap(Character);

    if (Character == ActiveHacker)
    {
        // Start draining instead of hard reset
        BeginDrainForActiveHacker();
    }
}

// Proximity logic (server)

void UProximityHackComponent::Server_UpdateProximity()
{
    PlayersInRange.RemoveAll([](const TWeakObjectPtr<ACharacter>& Ptr)
        {
            return !Ptr.IsValid();
        });

    if (ActiveHacker)
    {
        const bool bStillInRange = PlayersInRange.Contains(ActiveHacker);

        if (bStillInRange)
        {
            // If they were draining but came back into range, resume forward fill
            if (bDrainingHack)
            {
                bDrainingHack = false;
                bHackingInProgress = true;
                SetComponentTickEnabled(true);
            }
        }
        else
        {
            // ActiveHacker is out of range. See if we have someone else who should take over.
            ACharacter* NewHacker = FindFirstPlayerInRange();
            if (NewHacker && NewHacker != ActiveHacker)
            {
                // New player in range while old one is draining -> throw away old progress and start fresh.
                FullyCancelAndReset();
                StartHackingForPlayer(NewHacker);
            }
            // If no one else, we just keep draining until HackElapsed hits 0.
        }

        return;
    }

    // No ActiveHacker: pick first available player and start from 0
    if (!bHacked)
    {
        ACharacter* First = FindFirstPlayerInRange();
        if (First)
        {
            FullyCancelAndReset();
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
    if (!NewHacker || bHacked)
        return;

    // Only one hacker at a time unless we explicitly stole it in Server_UpdateProximity
    if (ActiveHacker && ActiveHacker != NewHacker)
        return;

    ActiveHacker = NewHacker;

    ApplyProximityTagTo(ActiveHacker);

    // Stop any drain and start forward fill
    bDrainingHack = false;
    bHackingInProgress = true;
    SetComponentTickEnabled(true);

    // IMPORTANT: do NOT call StartHacking() here, because that would reset HackElapsed to 0 on all machines.
    // We either already reset via FullyCancelAndReset, or we continue from current HackElapsed.
}

void UProximityHackComponent::BeginDrainForActiveHacker()
{
    // Switch from forward fill to drain, keep current HackElapsed
    bHackingInProgress = false;
    bDrainingHack = true;
    SetComponentTickEnabled(true);
}

void UProximityHackComponent::FullyCancelAndReset()
{
    bDrainingHack = false;
    RemoveProximityTagFrom(ActiveHacker);
    ActiveHacker = nullptr;
    CancelHacking(); // this calls MulticastResetHackElapsed and stops UHackComponent tick
}

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