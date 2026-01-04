#include "DronePawn.h"
#include "DefaultCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystem/AttributeSets/PlayerAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Components/SpotLightComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameStates/DefaultGameState.h"
#include "Actors/PointActors/NavNode.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/EOTGameplayTags.h"

ADronePawn::ADronePawn()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    USceneComponent* DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    DroneMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DroneMesh"));
    DroneMesh->SetupAttachment(RootComponent);

    DroneSpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("DroneSpotLight"));
    DroneSpotLight->SetupAttachment(RootComponent);
    DroneSpotLight->SetRelativeLocation(FVector(0, 0, -10));
    DroneSpotLight->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
    DroneSpotLight->SetIntensity(5000.f);
    DroneSpotLight->SetLightColor(FLinearColor::Green);
    DroneSpotLight->AttenuationRadius = DetectionDistance;
    DroneSpotLight->OuterConeAngle = ViewConeAngle * 0.5f;
    DroneSpotLight->InnerConeAngle = ViewConeAngle * 0.5f;
    DroneSpotLight->CastShadows = false;
    DroneSpotLight->SetVisibility(true);

    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("AttributeSet"));
}

void ADronePawn::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (DroneSpotLight)
    {
        DroneSpotLight->OuterConeAngle = ViewConeAngle * 0.5f;
        DroneSpotLight->InnerConeAngle = ViewConeAngle * 0.5f;
        DroneSpotLight->AttenuationRadius = DetectionDistance;
    }
}

void ADronePawn::RequestRepair_Implementation(AActor* RepairInstigator)
{
    bIsDead = false;
    // Optionally reset visuals, status.
}

void ADronePawn::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    if (Data.NewValue <= 0.f)
    {
        bIsDead = true;
        DetachFromControllerPendingDestroy();

        // Perform a downward trace to "land" the drone on the floor
        FVector Start = GetActorLocation();
        FVector End = Start - FVector(0, 0, 2000); // Trace down 2000 units

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);

        bool bHit = GetWorld()->LineTraceSingleByChannel(
            Hit,
            Start,
            End,
            ECC_Visibility,
            Params
        );

        if (bHit)
        {
            SetActorLocation(Hit.Location);

            // Optionally align vertical rotation:
            FRotator MyRot = GetActorRotation();
            SetActorRotation(FRotator(0.f, MyRot.Yaw, 0.f));
        }

        // Optionally, only show where the trace landed in debug:
        // DrawDebugSphere(GetWorld(), Hit.Location, 16, 8, FColor::Red, false, 5);

        // Leave these lines out so you don't ragdoll:
        // if (DroneMesh)
        // {
        //     DroneMesh->SetCollisionProfileName(TEXT("Ragdoll"));
        //     DroneMesh->SetSimulatePhysics(true);
        //     DroneMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        //     DroneMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
        // }

        if (AbilitySystemComponent && AttributeSet)
        {
            AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
                .RemoveAll(this);
        }
        OnRepairRequested.Broadcast(this);
    }
}

void ADronePawn::BeginPlay()
{
    Super::BeginPlay();
    GetWorldTimerManager().SetTimer(DetectionTimerHandle, this, &ADronePawn::DetectionUpdate, DetectionInterval, true);
    OnRep_DetectedActor();
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
        .AddUObject(this, &ADronePawn::OnHealthChanged);
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
}

void ADronePawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(DetectionTimerHandle);
    Super::EndPlay(EndPlayReason);
}

static bool IsBoundsPointInCone(
    FVector BoundsPoint, FVector ConeOrigin, FVector ConeForward,
    float ConeLength, float ConeHalfAngleDeg)
{
    FVector ToPoint = BoundsPoint - ConeOrigin;
    float Dist = ToPoint.Size();
    if (Dist > ConeLength)
        return false;
    ToPoint.Normalize();
    float Dot = FVector::DotProduct(ConeForward, ToPoint);
    float CosHalfFOV = FMath::Cos(FMath::DegreesToRadians(ConeHalfAngleDeg));
    return Dot >= CosHalfFOV;
}

void ADronePawn::DetectionUpdate()
{
    if (DetectedActor)
        return;

    FVector DroneLocation = DroneSpotLight ? DroneSpotLight->GetComponentLocation() : GetActorLocation();
    FVector Forward = DroneSpotLight ? DroneSpotLight->GetForwardVector() : GetActorForwardVector();

    TArray<AActor*> OverlappedActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        DroneLocation,
        DetectionDistance,
        ObjectTypes,
        nullptr,
        TArray<AActor*>{ this },
        OverlappedActors
    );

    float ClosestDist = DetectionDistance + 1.f;
    ADefaultCharacter* NewlyDetected = nullptr;
    for (AActor* Candidate : OverlappedActors)
    {
        if (!Candidate || Candidate == this)
            continue;

        ADefaultCharacter* DefaultChar = Cast<ADefaultCharacter>(Candidate);
        if (!DefaultChar)
            continue;
        UAbilitySystemComponent* ASC = DefaultChar->GetAbilitySystemComponent();
        if (!ASC || !ASC->HasMatchingGameplayTag(TAG_Character_Status_Illegal))
            continue;

        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(DefaultChar->GetRootComponent());
        FBox Bounds = PrimComp ? PrimComp->Bounds.GetBox() : FBox::BuildAABB(DefaultChar->GetActorLocation(), FVector(0,0,0));
        TArray<FVector> TestPoints;
        TestPoints.Add(Bounds.GetCenter());
        TestPoints.Add(Bounds.Min); TestPoints.Add(Bounds.Max);
        TestPoints.Add(FVector(Bounds.Min.X, Bounds.Min.Y, Bounds.Max.Z));
        TestPoints.Add(FVector(Bounds.Min.X, Bounds.Max.Y, Bounds.Min.Z));
        TestPoints.Add(FVector(Bounds.Max.X, Bounds.Min.Y, Bounds.Min.Z));
        TestPoints.Add(FVector(Bounds.Min.X, Bounds.Max.Y, Bounds.Max.Z));
        TestPoints.Add(FVector(Bounds.Max.X, Bounds.Min.Y, Bounds.Max.Z));
        TestPoints.Add(FVector(Bounds.Max.X, Bounds.Max.Y, Bounds.Min.Z));
        TestPoints.Add(FVector(Bounds.Max.X, Bounds.Max.Y, Bounds.Max.Z));

        bool bDetected = false;
        for (const FVector& Point : TestPoints)
        {
            if (IsBoundsPointInCone(Point, DroneLocation, Forward, DetectionDistance, ViewConeAngle * 0.5f))
            {
                FHitResult Hit;
                FCollisionQueryParams Params;
                Params.AddIgnoredActor(this);
                Params.AddIgnoredActor(DefaultChar);
                bool bHit = GetWorld()->LineTraceSingleByChannel(
                    Hit, DroneLocation, Point, ECC_Visibility, Params
                );
                if (bHit && Hit.GetActor() != DefaultChar)
                    continue;

                bDetected = true;
                float Dist = (Point - DroneLocation).Size();
                if (Dist < ClosestDist)
                {
                    ClosestDist = Dist;
                    NewlyDetected = DefaultChar;
                }
                break;
            }
        }
    }

    if (NewlyDetected)
    {
        DetectedActor = NewlyDetected;
        OnRep_DetectedActor();
        if (HasAuthority())
        {
            if (ADefaultGameState* GS = Cast<ADefaultGameState>(GetWorld()->GetGameState()))
            {
                GS->CancelPreAlarm(this);
                GS->StartAlarm(this);
            }
        }
    }
}

void ADronePawn::OnRep_DetectedActor()
{
    if (DroneSpotLight)
    {
        if (DetectedActor)
            DroneSpotLight->SetLightColor(FLinearColor::Red);
        else
            DroneSpotLight->SetLightColor(FLinearColor::Green);
    }
}

void ADronePawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADronePawn, DetectedActor);
}