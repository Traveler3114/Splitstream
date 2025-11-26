#include "CivilianCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "AbilitySystem/AttributeSets/PlayerAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "GameplayEffectTypes.h"
#include "Controllers/DefaultPlayerController.h"
#include "Characters/DefaultCharacter.h"
#include "Components/TimelineComponent.h"
#include "Components/StateTreeComponent.h"


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

    CivilianTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("CivilianTimeline"));

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

    if (CivilianCurve)
    {
        FOnTimelineFloat ProgressFunction;
        ProgressFunction.BindUFunction(this, FName("OnTimelineFloatUpdate"));
        CivilianTimeline->AddInterpFloat(CivilianCurve, ProgressFunction);

        FOnTimelineEvent FinishedFunction;
        FinishedFunction.BindUFunction(this, FName("OnTimelineFinished"));
        CivilianTimeline->SetTimelineFinishedFunc(FinishedFunction);

        CivilianTimeline->SetLooping(false);
    }
}

void ACivilianCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    if (Data.NewValue <= 0.f)
    {
        Destroy();
    }
}

void ACivilianCharacter::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (ADefaultCharacter* Player = Cast<ADefaultCharacter>(Actor))
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

            if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
            {
                FGameplayTag IllegalTag = TAG_Character_Status_Illegal;

                ASC->RegisterGameplayTagEvent(IllegalTag, EGameplayTagEventType::NewOrRemoved)
                    .AddLambda([this, Player](const FGameplayTag Tag, int32 NewCount)
                        {
                            FActorPerceptionBlueprintInfo Info;
                            if (AIPerceptionComponent)
                                AIPerceptionComponent->GetActorsPerception(Player, Info);

                            bool bSensedNow = false;
                            for (const auto& Stimulus : Info.LastSensedStimuli)
                            {
                                if (Stimulus.WasSuccessfullySensed())
                                {
                                    bSensedNow = true;
                                    break;
                                }
                            }

                            if (bSensedNow)
                            {
                                if (NewCount > 0)
                                {
                                    DetectedActor = Player;
                                    if (CivilianTimeline && TargetActor == nullptr)
                                    {
                                        CivilianTimeline->Play();
                                    }
                                }
                                else
                                {
                                    if (CivilianTimeline && TargetActor == nullptr)
                                    {
                                        CivilianTimeline->Reverse();
                                    }
                                }
                            }
                            else
                            {
                                if (CivilianTimeline && TargetActor == nullptr)
                                {
                                    CivilianTimeline->Reverse();
                                }
                            }
                        });

                if (ASC->HasMatchingGameplayTag(IllegalTag) && bSensed)
                {
                    DetectedActor = Player;
                    if (CivilianTimeline && TargetActor == nullptr)
                    {
                        CivilianTimeline->Play();
                    }
                }
                else if (ASC->HasMatchingGameplayTag(IllegalTag) && !bSensed)
                {
                    if (CivilianTimeline && TargetActor == nullptr)
                    {
                        CivilianTimeline->Reverse();
                    }
                }
            }
        }
    }
}

void ACivilianCharacter::OnTimelineFloatUpdate(float Value)
{
    if (DetectedActor)
    {
        ADefaultCharacter* DetectedPlayer = Cast<ADefaultCharacter>(DetectedActor);
        if (DetectedPlayer)
        {
            ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(DetectedPlayer->GetController());
            if (PC)
            {
                FVector PlayerLoc = DetectedPlayer->GetActorLocation();
                FVector CivLoc = GetActorLocation();

                FRotator CameraRot = PC->PlayerCameraManager->GetCameraRotation();
                FVector CameraForward = CameraRot.Vector();
                FVector CameraRight = FRotationMatrix(CameraRot).GetUnitAxis(EAxis::Y);

                FVector ToCivilian = CivLoc - PlayerLoc;

                FVector FlatForward = CameraForward; FlatForward.Z = 0; FlatForward.Normalize();
                FVector FlatToCivilian = ToCivilian; FlatToCivilian.Z = 0; FlatToCivilian.Normalize();

                float AngleRad = FMath::Acos(FVector::DotProduct(FlatForward, FlatToCivilian));
                float AngleDeg = FMath::RadiansToDegrees(AngleRad);
                float Sign = FVector::DotProduct(CameraRight, FlatToCivilian) > 0 ? 1.0f : -1.0f;
                AngleDeg *= Sign;

                PC->ClientUpdateDetectionWidget(this, Value, false, AngleDeg);
            }
        }
    }
}

void ACivilianCharacter::OnTimelineFinished()
{
    // At this point detection is finished (fully detected or reversed)
    CivilianTimeline->Stop();

    // Hide the widget for player:
    if (DetectedActor)
    {
        ADefaultCharacter* DetectedPlayer = Cast<ADefaultCharacter>(DetectedActor);
        if (DetectedPlayer)
        {
            ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(DetectedPlayer->GetController());
            if (PC)
            {
                PC->ClientUpdateDetectionWidget(this, 0.0f, true, 0.0f); // bIsLocked=true means hide
            }
        }
    }
}