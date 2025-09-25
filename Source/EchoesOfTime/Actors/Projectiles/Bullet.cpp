#include "Bullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

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
    CollisionComp->OnComponentHit.AddDynamic(this, &ABullet::OnHit);
    CollisionComp->SetupAttachment(RootComponent);

    // Mesh
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(CollisionComp);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Optionally rotate mesh if needed:
    // MeshComp->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));

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
}

void ABullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor && OtherActor != this)
    {
        //UGameplayStatics::ApplyPointDamage(OtherActor, 20.f, GetVelocity().GetSafeNormal(), Hit, GetInstigatorController(), this, nullptr);
    }
    Destroy();
}