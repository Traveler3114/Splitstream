// ProximityHackComponent.cpp
#include "ProximityHackComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Widgets/ProximityHackWidget.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"
#include "AbilitySystem/GE_Timer.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "ActorComponents/InventoryComponent.h"
#include "DataAssets/ItemBase.h"

UProximityHackComponent::UProximityHackComponent()
{
    SetIsReplicatedByDefault(true);
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f;
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
            float WidgetBaseSize = 259.0f; // match your widget's design size
            float Scale = (ProximityRadius * 2.0f) / WidgetBaseSize;
            ProximityWidgetComponent->SetWorldScale3D(FVector(1.f, Scale, Scale));

            if (UUserWidget* Widget = ProximityWidgetComponent->GetUserWidgetObject())
            {
                ProximityHackWidgetInstance = Cast<UProximityHackWidget>(Widget);
            }
        }
    }

    CachedFirstPC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
}

void UProximityHackComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UProximityHackComponent, ActiveHacker);
    DOREPLIFETIME(UProximityHackComponent, bDrainingHack);
}

bool UProximityHackComponent::IsLocallyViewedByActiveHacker() const
{
    if (!ActiveHacker)
    {
        return false;
    }

    APlayerController* PC = CachedFirstPC.IsValid() ? CachedFirstPC.Get() : nullptr;
    if (!PC)
    {
        return false;
    }

    APawn* LocalPawn = PC->GetPawn();
    return (LocalPawn == ActiveHacker);
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

void UProximityHackComponent::OnTimerRemoved(const FGameplayEffectRemovalInfo& RemovalInfo)
{
    if (!RemovalInfo.bPrematureRemoval && GetOwnerRole() == ROLE_Authority)
    {
        SetHacked();
    }
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

    // --- FILLING LOGIC (server only for source of truth) ---
    if (bIsServer && bHackingInProgress && !bDrainingHack && !bHacked && HackDuration > 0.f)
    {
        HackElapsed = FMath::Min(HackDuration, HackElapsed + DeltaTime);
    }

    // --- DRAINING LOGIC: opposite of filling ---
    if (bDrainingHack && HackDuration > 0.f)
    {
        HackElapsed = FMath::Max(0.f, HackElapsed - DeltaTime);

        if (HackElapsed <= 0.f)
        {
            HackElapsed = 0.f;
            bDrainingHack = false;
            bHackingInProgress = false;

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

    // Compute local progress from HackElapsed
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

    if (bHacked)
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

    if (bHacked)
        return;

    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (!Character)
        return;

    PlayersInRange.RemoveSwap(Character);

    if (Character == ActiveHacker)
    {
        BeginDrainForActiveHacker();
    }
}

// Proximity logic (server)

void UProximityHackComponent::Server_UpdateProximity()
{

    if (bHacked)
        return;


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
                ApplyTimerEffect();
                SetComponentTickEnabled(true);
            }
        }
        else
        {
            // ActiveHacker is out of range. See if we have someone else who should take over.
            ACharacter* NewHacker = FindFirstPlayerInRange();
            if (NewHacker && NewHacker != ActiveHacker)
            {
                FullyCancelAndReset();
                StartHackingForPlayer(NewHacker);
            }
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
    ApplyTimerEffect();
    SetComponentTickEnabled(true);
}

void UProximityHackComponent::ApplyTimerEffect()
{
    if (!ActiveHacker) return;

    if (IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(ActiveHacker))
    {
        if (UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent())
        {
            // Unbind and remove existing timer if any
            if (TimerEffectHandle.IsValid())
            {
                if (FOnActiveGameplayEffectRemoved_Info* RemovedDel = ASC->OnGameplayEffectRemoved_InfoDelegate(TimerEffectHandle))
                {
                    RemovedDel->RemoveAll(this);
                }
                ASC->RemoveActiveGameplayEffect(TimerEffectHandle);
                TimerEffectHandle.Invalidate();
            }

            float RemainingDuration = FMath::Max(0.001f, HackDuration - HackElapsed);

            FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
            FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(UGE_Timer::StaticClass(), 1.f, Context);
            if (Spec.IsValid())
            {
                Spec.Data->SetSetByCallerMagnitude(TAG_Effect_Timer, RemainingDuration);
                TimerEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
                if (FOnActiveGameplayEffectRemoved_Info* RemovedDel = ASC->OnGameplayEffectRemoved_InfoDelegate(TimerEffectHandle))
                {
                    RemovedDel->AddUObject(this, &UProximityHackComponent::OnTimerRemoved);
                }
            }
        }
    }
}

void UProximityHackComponent::BeginDrainForActiveHacker()
{
    // Remove GE timer (premature removal, completion handler will ignore it)
    if (TimerEffectHandle.IsValid() && ActiveHacker)
    {
        if (IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(ActiveHacker))
        {
            if (UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent())
            {
                if (FOnActiveGameplayEffectRemoved_Info* RemovedDel = ASC->OnGameplayEffectRemoved_InfoDelegate(TimerEffectHandle))
                {
                    RemovedDel->RemoveAll(this);
                }
                ASC->RemoveActiveGameplayEffect(TimerEffectHandle);
            }
        }
        TimerEffectHandle.Invalidate();
    }

    bHackingInProgress = false;
    bDrainingHack = true;
    SetComponentTickEnabled(true);
}

void UProximityHackComponent::FullyCancelAndReset()
{
    bDrainingHack = false;
    CancelHacking();
    RemoveProximityTagFrom(ActiveHacker);
    ActiveHacker = nullptr;
}

void UProximityHackComponent::CancelHacking()
{
    // Remove GE timer
    if (TimerEffectHandle.IsValid())
    {
        if (ActiveHacker)
        {
            if (IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(ActiveHacker))
            {
                if (UAbilitySystemComponent* ASC = AbilityInterface->GetAbilitySystemComponent())
                    {
                        if (FOnActiveGameplayEffectRemoved_Info* RemovedDel = ASC->OnGameplayEffectRemoved_InfoDelegate(TimerEffectHandle))
                        {
                            RemovedDel->RemoveAll(this);
                        }
                        ASC->RemoveActiveGameplayEffect(TimerEffectHandle);
                    }
            }
        }
        TimerEffectHandle.Invalidate();
    }

    bHackingInProgress = false;
    MulticastResetHackElapsed();
}

void UProximityHackComponent::MulticastResetHackElapsed_Implementation()
{
    HackElapsed = 0.f;
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
