#include "LaserSensor.h"
#include "Characters/DefaultCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "GameStates/DefaultGameState.h"
#include "Kismet/KismetMathLibrary.h"

ALaserSensor::ALaserSensor()
{
    bReplicates = true;

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
}

void ALaserSensor::BeginPlay()
{
    Super::BeginPlay();
    LaserBox->OnComponentBeginOverlap.AddDynamic(this, &ALaserSensor::OnLaserOverlap);
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
    FVector BoxExtent = FVector(Length * 0.5f, 2.5f, 2.5f); // 30 is thickness, tweak as needed
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
    if (HasAuthority())
    {
        if (OtherActor && OtherActor->IsA(ADefaultCharacter::StaticClass()))
        {
            if (HasAuthority())
            {
                if (ADefaultGameState* GS = Cast<ADefaultGameState>(GetWorld()->GetGameState()))
                {
                    GS->RequestRestart();
                }
            }
        }
    }
}