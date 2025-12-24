// GuardCharacter.cpp
#include "GuardCharacter.h"
#include "Actors/TimeObjects/GhostCharacterActor.h"
#include "Engine/Engine.h"
#include "Components/TextRenderComponent.h"
#include "Actors/PointActors/RefPointActor.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "AbilitySystem/AttributeSets/PlayerAttributeSet.h"
#include "GameStates/DefaultGameState.h"
#include "GameplayEffectTypes.h"
#include "Interfaces/IDetectable.h"
#include "Components/StateTreeComponent.h"
#include "ActorComponents/ProximityHackComponent.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "ActorComponents/DetectionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AGuardCharacter::AGuardCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

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


    NameText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NameText"));
    NameText->SetupAttachment(GetMesh());

    DetectionComponent = CreateDefaultSubobject<UDetectionComponent>(TEXT("DetectionComponent"));
    DetectionComponent->SetIsReplicated(true);
}



void AGuardCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    // If guard has died, fully disable ALL logic and leave only the ragdoll
    if (Data.NewValue <= 0.f)
    {
        bIsDead = true;
        if (HasAuthority())
        {
            if (ADefaultGameState* GS = Cast<ADefaultGameState>(GetWorld()->GetGameState()))
            {
                GS->CancelPreAlarm(nullptr);
            }
        }

        // Detach/Destroy controller (blocks AI/possession/BT/StateTree logic)
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

        // Enable ragdoll physics on mesh
        USkeletalMeshComponent* SkelMesh = GetMesh();
        if (SkelMesh)
        {
            SkelMesh->SetCollisionProfileName(TEXT("Ragdoll"));
            SkelMesh->SetSimulatePhysics(true);
        }

        // Hide text/name or other visible extras
        if (NameText)
        {
            NameText->SetVisibility(false);
        }

        // Destroy spawned ghost
        if (SpawnedGhost)
        {
            SpawnedGhost->Destroy();
            SpawnedGhost = nullptr;
        }

        // Disable AI Perception
        if (AIPerceptionComponent)
        {
            AIPerceptionComponent->Deactivate();            // Hides/disables sensing, perception updates, callbacks
            AIPerceptionComponent->OnPerceptionUpdated.Clear(); // Remove all bound events
            // AIPerceptionComponent->DestroyComponent();    // alternatively, fully remove component
        }

        // Remove attribute change delegates
        if (AbilitySystemComponent && AttributeSet)
        {
            AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
                .RemoveAll(this);
        }
    }
}

void AGuardCharacter::OnRep_GuardName()
{
    if (NameText)
        NameText->SetText(FText::FromString(GuardName));
}

void AGuardCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (NameText)
    {
        NameText->SetText(FText::FromString(GuardName));
    }

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
        .AddUObject(this, &AGuardCharacter::OnHealthChanged);

    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AGuardCharacter::OnPerceptionUpdated);
    }
    if (HasAuthority())
    {
        FActorSpawnParameters Params;
        Params.Owner = this;
        Params.Instigator = GetInstigator();
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        SpawnedGhost = GetWorld()->SpawnActor<AGhostCharacterActor>(
            GhostClass,
            GetActorLocation(),
            GetActorRotation(),
            Params);

        if (SpawnedGhost)
        {
            FVector Offset = ARefPointActor::GetOffsetBetweenFirstTwoRefPoints(GetWorld());
            Offset.Z -= 90.0f;
            SpawnedGhost->GhostOffset = Offset;
        }
    }
}

void AGuardCharacter::OnDetected_Implementation(AActor* Detector)
{
    if (!bIsInCameraView && !bIsDead)
    {
        bIsInCameraView = true;
        if (SpawnedGhost)
        {
            SpawnedGhost->UpdateGhostVisibility();
        }
    }
    if (DetectionComponent && !(DetectionComponent->bDetectionInProgress) && !(DetectionComponent->bFullyDetected) && bIsDead) DetectionComponent->StartDetection(Detector);
}

void AGuardCharacter::OnLost_Implementation(AActor* Detector)
{
    if (bIsInCameraView && !bIsDead)
    {
        bIsInCameraView = false;
        if (SpawnedGhost)
        {
            SpawnedGhost->UpdateGhostVisibility();
        }
    }
    if (DetectionComponent && !(DetectionComponent->bDetectionInProgress) && !(DetectionComponent->bFullyDetected) && bIsDead) DetectionComponent->StopDetection(Detector);
}

void AGuardCharacter::OnFullyDetected_Implementation(AActor* ActorDetected)
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



    if (HasAuthority())
    {
        if (ADefaultGameState* GS = Cast<ADefaultGameState>(GetWorld()->GetGameState()))
        {
            GS->StartPreAlarm(this, PreAlarmDuration);
        }
    }
}

void AGuardCharacter::OnRep_IsInCameraView()
{
    if (SpawnedGhost)
    {
        SpawnedGhost->UpdateGhostVisibility();
    }
}

bool AGuardCharacter::ShouldGhostBeVisible_Implementation() const
{
    return bIsInCameraView;
}

USkeletalMeshComponent* AGuardCharacter::GetMirrorMesh_Implementation() const
{
    return GetMesh();
}

void AGuardCharacter::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
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



void AGuardCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGuardCharacter, GuardName);
    DOREPLIFETIME(AGuardCharacter, bIsInCameraView);
}

bool AGuardCharacter::IsActorAlreadyDetected_Implementation(AActor* DetectingActor) const
{
    if (bIsDead)
        return false;
    if (!AIPerceptionComponent)
        return false;
    TArray<AActor*> PerceivedActors;
    AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);
    return PerceivedActors.Contains(DetectingActor);
}