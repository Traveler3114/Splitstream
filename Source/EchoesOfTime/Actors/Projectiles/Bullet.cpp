#include "Bullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "AbilitySystemComponent.h"

ABullet::ABullet()
{
    bReplicates = true;
    SetReplicateMovement(true);

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
    CollisionComp->InitCapsuleSize(5.0f, 15.0f);
    CollisionComp->SetCollisionProfileName("Projectile");
    CollisionComp->SetupAttachment(DefaultSceneRoot);

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(CollisionComp);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

    if (ProjectileMovement)
    {
        ProjectileMovement->Velocity = GetActorForwardVector() * ProjectileMovement->InitialSpeed;
    }
    if (CollisionComp)
    {
        CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ABullet::OnBeginOverlap);
    }
}

void ABullet::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor)
    {
        return;
    }

    if (OtherActor == this)
    {
        return;
    }

    IAbilitySystemInterface* ASCActor = Cast<IAbilitySystemInterface>(OtherActor);
    if (!ASCActor)
    {
        Destroy();
        return;
    }
    if (!DamageEffectClass)
    {
        Destroy();
        return;
    }

    UAbilitySystemComponent* ASC = ASCActor->GetAbilitySystemComponent();
    if (!ASC)
    {
        Destroy();
        return;
    }

    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
    EffectContext.AddSourceObject(this);

    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DamageEffectClass, 1, EffectContext);
    if (SpecHandle.IsValid())
    {
        ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }

    Destroy();
}

void ABullet::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    if (CollisionComp)
    {
        CollisionComp->OnComponentBeginOverlap.RemoveDynamic(this, &ABullet::OnBeginOverlap);
    }
}

void ABullet::SetIgnoreActorsAndComponents(AActor* IgnoreActor, UPrimitiveComponent* IgnoreComponent)
{
    if (CollisionComp)
    {
        if (IgnoreActor)
        {
            // Optional: implement if you want to call IgnoreActorWhenMoving
        }
        if (IgnoreComponent)
        {
            CollisionComp->IgnoreComponentWhenMoving(IgnoreComponent, true);
        }
    }
}