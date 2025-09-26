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
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Bullet hit!"));
        }

        // Example: Apply damage, spawn FX, etc. here

        Destroy();
    }
}