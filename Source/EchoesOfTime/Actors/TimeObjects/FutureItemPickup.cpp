#include "FutureItemPickup.h"
#include "PastItemPickup.h"
#include "DrawDebugHelpers.h"

AFutureItemPickup::AFutureItemPickup()
{
    PrimaryActorTick.bCanEverTick = true; // Enable ticking!
}

void AFutureItemPickup::BeginPlay()
{
    Super::BeginPlay();
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("FutureItemPickup"));
    }
}

void AFutureItemPickup::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        60.f,
        24,
        FColor::Blue,
        false,
        -1.f,
        0,
        2.f
    );

    if (!MeshComponent->GetStaticMesh()) 
    {
        //if (GEngine)
        //{
        //    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Staticmesh Invalid"));
        //}
    }
}

void AFutureItemPickup::OnPastItemPickedUp()
{
    Destroy();
}