#include "Bullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "AbilitySystemComponent.h"

ABullet::ABullet()
{
    bReplicates = true;
    SetReplicateMovement(true);

    // Create a scene root
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    // Collision
    CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
    CollisionComp->InitCapsuleSize(5.0f, 15.0f);
    CollisionComp->SetCollisionProfileName("Projectile");
    CollisionComp->SetupAttachment(DefaultSceneRoot);

    // Mesh
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(CollisionComp);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Movement
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp; // Moves collision, mesh follows via root
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
        IAbilitySystemInterface* ASCActor = Cast<IAbilitySystemInterface>(OtherActor);
        if (ASCActor && DamageEffectClass)
        {
            UAbilitySystemComponent* ASC = ASCActor->GetAbilitySystemComponent();
            if (ASC)
            {
                FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
                EffectContext.AddSourceObject(this);

                FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DamageEffectClass, 1, EffectContext);
                if (SpecHandle.IsValid())
                {
                    ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
                }
            }
        }

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