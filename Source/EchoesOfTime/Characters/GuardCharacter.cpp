// GuardCharacter.cpp
#include "GuardCharacter.h"
#include "Actors/TimeObjects/GhostCharacterActor.h"
#include "Actors/PointActors/NavNode.h"
#include "Engine/Engine.h"
#include "Characters/DefaultCharacter.h"
#include "Components/TextRenderComponent.h"
#include "Actors/PointActors/RefPointActor.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "AbilitySystem/AttributeSets/PlayerAttributeSet.h"
#include "Controllers/DefaultPlayerController.h"
#include "GameStates/DefaultGameState.h"
#include "GameplayEffectTypes.h"
#include "Interfaces/IDetectable.h"
#include "TimerManager.h"
#include "Actors/LockerActor.h"

AGuardCharacter::AGuardCharacter()
{
    PrimaryActorTick.bCanEverTick = false; // No Tick function, disable for performance

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
}

void AGuardCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    if (GEngine)
    {
        FString DebugMsg = FString::Printf(TEXT("Health changed: %.1f"), Data.NewValue);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, DebugMsg);
    }

    if (Data.NewValue <= 0.f)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Health is ZERO!"));
        }

        if (HasAuthority())
        {
            if (ADefaultGameState* GS = Cast<ADefaultGameState>(GetWorld()->GetGameState()))
            {
                GS->CancelPreAlarm(this);
                if (!GS->bAlarmActive)
                {
                    GS->CancelAlarm(this);
                }
            }
        }

        if (SpawnedGhost)
        {
            SpawnedGhost->Destroy();
            SpawnedGhost = nullptr;
        }

        Destroy();
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
    if (!bIsInCameraView)
    {
        bIsInCameraView = true;
        if (SpawnedGhost)
        {
            SpawnedGhost->UpdateGhostVisibility();
        }
    }
}

void AGuardCharacter::OnLost_Implementation(AActor* Detector)
{
    if (bIsInCameraView)
    {
        bIsInCameraView = false;
        if (SpawnedGhost)
        {
            SpawnedGhost->UpdateGhostVisibility();
        }
    }
}

// Called by detected actors when their own detection timeline is finished
void AGuardCharacter::OnFullyDetected_Implementation(AActor* DetectingActor)
{
    TargetActor = DetectingActor; // The actor that is now fully detected
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
    // Example for player character - returns true if this guard is currently perceiving DetectingActor
    if (!AIPerceptionComponent)
        return false;

    TArray<AActor*> PerceivedActors;
    AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);
    return PerceivedActors.Contains(DetectingActor);
}