#include "CivilianCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "AbilitySystem/AttributeSets/PlayerAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Interfaces/IDetectable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StateTreeComponent.h"
#include "AbilitySystem/EOTGameplayTags.h"

ACivilianCharacter::ACivilianCharacter()
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
}

void ACivilianCharacter::BeginPlay()
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
        .AddUObject(this, &ACivilianCharacter::OnHealthChanged);

    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACivilianCharacter::OnPerceptionUpdated);
    }
}

void ACivilianCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    if (Data.NewValue <= 0.f)
    {
        // Detach/Destroy controller removes AI or Player control and all brain logic
        DetachFromControllerPendingDestroy();

        // Stop movement
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->DisableMovement();
        }

        // Disable capsule collision so body doesn't "pop"
        if (GetCapsuleComponent())
        {
            GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        // Enable physics, i.e., ragdoll, on the mesh
        USkeletalMeshComponent* SkelMesh = GetMesh();
        if (SkelMesh)
        {
            SkelMesh->SetCollisionProfileName(TEXT("Ragdoll"));
            SkelMesh->SetSimulatePhysics(true);
        }

        // Deactivate perception (stops further sensing, disables related events)
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

void ACivilianCharacter::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
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
                IDetectable::Execute_OnLost(Actor, this);
                if (DetectedActor == Actor)
                {
                    DetectedActor = nullptr;
                }
            }
        }
    }
}

// Called by detected actors when their detection timeline is finished
void ACivilianCharacter::OnFullyDetected_Implementation(AActor* ActorDetected)
{
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

bool ACivilianCharacter::IsActorAlreadyDetected_Implementation(AActor* DetectingActor) const
{
    if (!AIPerceptionComponent || !AIPerceptionComponent->IsActive())
        return false;
    TArray<AActor*> PerceivedActors;
    AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);
    return PerceivedActors.Contains(DetectingActor);
}