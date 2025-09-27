#include "Bullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h" // For debug

ABullet::ABullet()
{
    bReplicates = true;
    SetReplicateMovement(true);

    // Create and set as Root
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    // Capsule collision
    CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
    CollisionComp->InitCapsuleSize(5.0f, 15.0f);
    CollisionComp->SetCollisionProfileName("Projectile");
    CollisionComp->SetupAttachment(RootComponent);

    // Mesh
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(CollisionComp);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Movement
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 3000.f;
    ProjectileMovement->MaxSpeed = 3000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;

    InitialLifeSpan = 3.0f;

    
}

void ABullet::BeginPlay()
{
    Super::BeginPlay();

    // Set velocity in the direction the projectile is facing
    if (ProjectileMovement)
    {
        ProjectileMovement->Velocity = GetActorForwardVector() * ProjectileMovement->InitialSpeed;
    }
    CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ABullet::OnBeginOverlap);
}

void ABullet::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this)
    {
        // Print debug message on screen
        FString DebugMsg = FString::Printf(
            TEXT("Bullet hit! Actor: %s, Component: %s"),
            *OtherActor->GetName(),
            OtherComp ? *OtherComp->GetName() : TEXT("None")
        );
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, DebugMsg);
        }

        // Log to UE output log
        UE_LOG(LogTemp, Warning, TEXT("%s"), *DebugMsg);

        // Example: Apply damage, spawn FX, etc. here

        Destroy();
    }
}

void ABullet::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
	CollisionComp->OnComponentBeginOverlap.RemoveDynamic(this, &ABullet::OnBeginOverlap);
}


void ABullet::SetIgnoreActorsAndComponents(AActor* IgnoreActor, UPrimitiveComponent* IgnoreComponent)
{
    if (CollisionComp)
    {
        if (IgnoreActor)
        {
            CollisionComp->IgnoreActorWhenMoving(IgnoreActor, true);
        }
        if (IgnoreComponent)
        {
            CollisionComp->IgnoreComponentWhenMoving(IgnoreComponent, true);
        }
    }
}