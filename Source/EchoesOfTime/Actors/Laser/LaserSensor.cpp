#include "LaserSensor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "GameStates/DefaultGameState.h"
#include "Characters/DefaultCharacter.h"

ALaserSensor::ALaserSensor()
{
    bReplicates = true;
    NetUpdateFrequency = 10.f; // Lasers toggle infrequently

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    StartMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StartMesh"));
    StartMesh->SetupAttachment(DefaultSceneRoot);

    EndMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EndMesh"));
    EndMesh->SetupAttachment(DefaultSceneRoot);

    LaserBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LaserBox"));
    LaserBox->SetupAttachment(DefaultSceneRoot);
    LaserBox->SetCollisionProfileName(TEXT("Trigger"));

    LaserBeamNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LaserBeamNiagara"));
    LaserBeamNiagara->SetupAttachment(DefaultSceneRoot);

    bIsActive = true;
}

void ALaserSensor::BeginPlay()
{
    Super::BeginPlay();

    LaserBox->OnComponentBeginOverlap.AddDynamic(this, &ALaserSensor::OnLaserOverlap);

    // Ensure initial visuals reflect replicated bIsActive
    ApplyActiveState();
}

void ALaserSensor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // Get Start and End mesh locations
    const FVector StartLoc = StartMesh->GetComponentLocation();
    const FVector EndLoc = EndMesh->GetComponentLocation();

    // Calculate midpoint and direction
    const FVector MidLoc = (StartLoc + EndLoc) * 0.5f;
    const FVector Delta = EndLoc - StartLoc;
    const float Length = Delta.Size();
    const FVector Direction = Delta.GetSafeNormal();

    // Set Box at midpoint
    LaserBox->SetWorldLocation(MidLoc);

    // Set Box rotation to align X axis with beam direction
    FRotator BoxRot = UKismetMathLibrary::FindLookAtRotation(MidLoc, EndLoc);
    LaserBox->SetWorldRotation(BoxRot);

    // Set Box size: X = Length, Y and Z = desired thickness
    FVector BoxExtent = FVector(Length * 0.5f, 2.5f, 2.5f); // adjust thickness as needed
    LaserBox->SetBoxExtent(BoxExtent);

    // Update Niagara beam
    if (LaserBeamNiagara)
    {
        LaserBeamNiagara->SetVectorParameter(TEXT("Beam Start"), StartLoc);
        LaserBeamNiagara->SetVectorParameter(TEXT("Beam End"), EndLoc);
        LaserBeamNiagara->Activate(true);
    }
}

void ALaserSensor::OnLaserOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority())
    {
        return;
    }

    if (OtherActor && OtherActor->IsA(ADefaultCharacter::StaticClass()))
    {
        if (ADefaultGameState* GS = Cast<ADefaultGameState>(GetWorld()->GetGameState()))
        {
            // start alarm instead of immediate restart
            GS->StartAlarm();
        }
    }
}

void ALaserSensor::SetActive(bool bNewActive)
{
    if (HasAuthority())
    {
        // We are server: set directly
        bIsActive = bNewActive;
        ApplyActiveState();
    }
    else
    {
        // Ask server to set it
        ServerSetActive(bNewActive);
    }
}

void ALaserSensor::ServerSetActive_Implementation(bool bNewActive)
{
    bIsActive = bNewActive;
    ApplyActiveState();
}

void ALaserSensor::OnRep_IsActive()
{
    ApplyActiveState();
}

void ALaserSensor::ApplyActiveState()
{
    // Visuals
    if (LaserBeamNiagara)
    {
        LaserBeamNiagara->SetActive(bIsActive, true);
        LaserBeamNiagara->SetVisibility(bIsActive, true);
    }


    // Collision / overlap
    if (LaserBox)
    {
        LaserBox->SetGenerateOverlapEvents(bIsActive);
        LaserBox->SetCollisionEnabled(bIsActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    }
}

void ALaserSensor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ALaserSensor, bIsActive);
}