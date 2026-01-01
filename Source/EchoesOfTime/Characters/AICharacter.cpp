// AICharacter.cpp

#include "AICharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "AbilitySystem/AttributeSets/PlayerAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Interfaces/IDetectable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ActorComponents/InventoryComponent.h"
#include "Components/CapsuleComponent.h"
#include "ActorComponents/DetectionComponent.h"
#include "ActorComponents/SearchComponent.h"
#include "ActorComponents/ProximityHackComponent.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "Components/StateTreeComponent.h"

AAICharacter::AAICharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // AI Perception setup
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1600.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("AttributeSet"));

    DetectionComponent = CreateDefaultSubobject<UDetectionComponent>(TEXT("DetectionComponent"));
    DetectionComponent->SetIsReplicated(true);

    SearchComponent = CreateDefaultSubobject<USearchComponent>(TEXT("SearchComponent"));
    SearchComponent->SetIsReplicated(true);
}

void AAICharacter::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority() && AttributeInitGE)
    {
        if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
        {
            FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
            FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(AttributeInitGE, 1, EffectContext);
            if (SpecHandle.IsValid())
            {
                ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            }
        }
    }
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
        .AddUObject(this, &AAICharacter::OnHealthChanged);

    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AAICharacter::OnPerceptionUpdated);
    }
    if (SearchComponent)
    {
        SearchComponent->OnSearchComplete.AddDynamic(this, &AAICharacter::OnSearchComplete);
    }
}

void AAICharacter::Interact_Implementation(AActor* Interactor)
{
    if (SearchComponent && bIsDead)
    {
        SearchComponent->Interact(Interactor);
    }
}
void AAICharacter::CancelInteract_Implementation(AActor* Interactor)
{
    if (SearchComponent && bIsDead)
    {
        SearchComponent->CancelInteract(Interactor);
    }
}
void AAICharacter::SetHighlighted_Implementation(bool bHighlight)
{
    USkeletalMeshComponent* SkelMesh = GetMesh();
    if (SkelMesh && bIsDead)
    {
        SkelMesh->SetRenderCustomDepth(bHighlight);
        SkelMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}
void AAICharacter::OnSearchComplete()
{
    AActor* Interactor = SearchComponent ? SearchComponent->LastInteractor.Get() : nullptr;
    TryPickup(Interactor);
}

void AAICharacter::TryPickup(AActor* Interactor)
{
    if (!HasAuthority()) return;
    if (!ItemData || !Interactor) return;

    UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();
    if (Inventory && Inventory->AddItem(ItemData, ItemInstanceID))
    {
        OnItemPickedUp.Broadcast(Interactor, ItemData);
        Destroy();
    }
}

void AAICharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    if (Data.NewValue <= 0.f)
    {
        bIsDead = true;

        if (HasAuthority() && AIPerceptionComponent)
        {
            TArray<AActor*> PerceivedActors;
            AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);
            for (AActor* Actor : PerceivedActors)
            {
                if (Actor && Actor->GetClass()->ImplementsInterface(UDetectable::StaticClass()))
                {
                    IDetectable::Execute_OnForceDetectionEnd(Actor, this);
                }
            }
        }

        DetachFromControllerPendingDestroy();

        // Stop movement
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->DisableMovement();
        }
        // Disable capsule collision
        if (GetCapsuleComponent())
        {
            GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        if (HasAuthority())
        {
            if (UProximityHackComponent* ProxComp = FindComponentByClass<UProximityHackComponent>())
            {
                ProxComp->DestroyComponent();
            }
        }
        // Enable ragdoll on mesh
        USkeletalMeshComponent* SkelMesh = GetMesh();
        if (SkelMesh)
        {
            SkelMesh->SetCollisionProfileName(TEXT("Ragdoll"));
            SkelMesh->SetSimulatePhysics(true);
            SkelMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // Traces + physics
            SkelMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
        }
        // Disable perception
        if (AIPerceptionComponent)
        {
            AIPerceptionComponent->Deactivate();
            AIPerceptionComponent->OnPerceptionUpdated.Clear();
        }
        // Remove attribute change delegates
        if (AbilitySystemComponent && AttributeSet)
        {
            AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
                .RemoveAll(this);
        }

    }
}

void AAICharacter::OnDetected_Implementation(AActor* Detector)
{
    if (DetectionComponent && !DetectionComponent->IsDetectionInProgress(Detector) && !DetectionComponent->IsFullyDetected(Detector) && bIsDead)
        DetectionComponent->StartDetection(Detector);
}
void AAICharacter::OnLost_Implementation(AActor* Detector)
{
    if (DetectionComponent && !DetectionComponent->IsDetectionInProgress(Detector) && !DetectionComponent->IsFullyDetected(Detector) && bIsDead)
        DetectionComponent->StopDetection(Detector);
}
void AAICharacter::OnForceDetectionEnd_Implementation(AActor* Detector)
{
    if (DetectionComponent) DetectionComponent->ForceImmediateDetectionEnd(Detector);
}
void AAICharacter::OnFullyDetected_Implementation(AActor* ActorDetected)
{
    if (bIsDead) return;
    TargetActor = ActorDetected;
    if (ActorDetected->IsA(APawn::StaticClass()))
    {
        AController* GuardController = GetController();
        if (GuardController)
        {
            UStateTreeComponent* StateTreeComp = GuardController->FindComponentByClass<UStateTreeComponent>();
            if (StateTreeComp)
            {
                FStateTreeEvent MyEvent(TAG_StateTree_Event_FullyDetected_Pawn);
                StateTreeComp->SendStateTreeEvent(MyEvent);
            }
        }
    }
    else
    {
        AController* GuardController = GetController();
        if (GuardController)
        {
            UStateTreeComponent* StateTreeComp = GuardController->FindComponentByClass<UStateTreeComponent>();
            if (StateTreeComp)
            {
                FStateTreeEvent MyEvent(TAG_StateTree_Event_FullyDetected_Actor);
                StateTreeComp->SendStateTreeEvent(MyEvent);
            }
        }
    }
}

bool AAICharacter::IsActorAlreadyDetected_Implementation(AActor* DetectingActor) const
{
    if (bIsDead)
        return false;
    if (!AIPerceptionComponent)
        return false;
    TArray<AActor*> PerceivedActors;
    AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);
    return PerceivedActors.Contains(DetectingActor);
}

void AAICharacter::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->GetClass()->ImplementsInterface(UDetectable::StaticClass()))
        {
            FActorPerceptionBlueprintInfo Info;
            AIPerceptionComponent->GetActorsPerception(Actor, Info);

            bool bSensed = false;
            for (const auto& Stimulus : Info.LastSensedStimuli)
            {
                if (Stimulus.WasSuccessfullySensed())
                {
                    bSensed = true;
                    break;
                }
            }

            if (bSensed)
            {
                DetectedActor = Actor;
                IDetectable::Execute_OnDetected(Actor, this); // Notify target it is being detected
            }
            else
            {
                IDetectable::Execute_OnLost(Actor, this); // Notify target it is not being detected
                if (DetectedActor == Actor)
                {
                    DetectedActor = nullptr;
                }
            }
        }
    }
}