#include "FutureCube.h"
#include "PresentCube.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AFutureCube::AFutureCube()
{
    PrimaryActorTick.bCanEverTick = true;

    ObjectMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectMesh"));
    RootComponent = ObjectMesh;

    ObjectMesh->SetSimulatePhysics(true);
}

void AFutureCube::BeginPlay()
{
    Super::BeginPlay();

    if (PresentObject)
    {
        // Get the initial relative position between the PresentObject and FutureCube (we're only interested in the X difference)
        FVector PresentLocation = PresentObject->GetActorLocation();
        FVector FutureLocation = GetActorLocation();


        // Store the initial offset on the X-axis (X difference only)
        InitialXOffset = PresentLocation.X - FutureLocation.X;
        PreviousPresentObjectTransform.SetLocation(PresentObject->GetActorLocation());
    }
}

void AFutureCube::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (PresentObject)
    {
        // Get the current location and velocity of the PresentObject
        FVector PresentLocation = PresentObject->GetActorLocation();
        FVector PresentVelocity = PresentObject->GetVelocity();  // Get the PresentObject's velocity

        // Get the previous location of the PresentObject (to check if it's moving)
        FVector PreviousLocation = PreviousPresentObjectTransform.GetLocation();

        // Check if the PresentObject has moved
        bool bPresentIsMoving = !PresentLocation.Equals(PreviousLocation);

        if (!bPresentIsMoving)
        {
            // Allow FutureCube to move independently (physics enabled)
            if (!ObjectMesh->IsSimulatingPhysics())
            {
                ObjectMesh->SetSimulatePhysics(true);
            }

            FVector CurrentVelocity = ObjectMesh->GetPhysicsLinearVelocity();
            CurrentVelocity.X += 10.f * DeltaTime;  // Your independent movement logic
            ObjectMesh->SetPhysicsLinearVelocity(CurrentVelocity);
        }
        else
        {
            // Strictly mirror PresentObject position and rotation
            if (ObjectMesh->IsSimulatingPhysics())
            {
                ObjectMesh->SetSimulatePhysics(false);  // Disable physics while mirroring position
            }

            FVector FutureLocation = PresentLocation;
            FutureLocation.X = PresentLocation.X - InitialXOffset;

            SetActorLocation(FutureLocation, false, nullptr, ETeleportType::TeleportPhysics);
            SetActorRotation(PresentObject->GetActorRotation());
        }

        // Store the current location as previous for the next tick comparison
        PreviousPresentObjectTransform.SetLocation(PresentLocation);
    }
}



