#include "IllegalArea.h"
#include "Components/BoxComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"

AIllegalArea::AIllegalArea()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;
    TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
}

void AIllegalArea::BeginPlay()
{
    Super::BeginPlay();

    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AIllegalArea::OnOverlapBegin);
    TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AIllegalArea::OnOverlapEnd);
}

void AIllegalArea::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

    if (!OtherActor || !GameplayEffectClass)
    {
        return;
    }

    // Check for Ability System
    IAbilitySystemInterface* AbilitySystemActor = Cast<IAbilitySystemInterface>(OtherActor);
    if (!AbilitySystemActor)
    {
        return;
    }

    UAbilitySystemComponent* ASC = AbilitySystemActor->GetAbilitySystemComponent();
    if (!ASC)
    {
        return;
    }


    // Prevent duplicate application from same area (optional)
    // FGameplayTag IllegalTag = FGameplayTag::RequestGameplayTag(TEXT("Character.Status.Illegal"));
    // if (ASC->HasMatchingGameplayTag(IllegalTag)) {
    //     ILLEGALAREA_LOG("OnOverlapBegin: %s already has Illegal tag, skipping apply.", *OtherActor->GetName());
    //     return;
    // }

    // Apply the effect and store the handle in a TArray per actor
    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GameplayEffectClass, 1.0f, EffectContext);

    if (!SpecHandle.IsValid())
    {
        return;
    }

    FActiveGameplayEffectHandle EffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    if (EffectHandle.IsValid())
    {
        if (EffectHandles.Contains(OtherActor))
        {
            EffectHandles[OtherActor].Add(EffectHandle);
        }
        else
        {
            EffectHandles.Add(OtherActor, { EffectHandle });
        }
    }
}

void AIllegalArea::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

    if (!OtherActor)
    {
        return;
    }

    IAbilitySystemInterface* AbilitySystemActor = Cast<IAbilitySystemInterface>(OtherActor);
    if (!AbilitySystemActor)
    {
        return;
    }

    UAbilitySystemComponent* ASC = AbilitySystemActor->GetAbilitySystemComponent();
    if (!ASC)
    {
        return;
    }


    TArray<FActiveGameplayEffectHandle>* HandlesPtr = EffectHandles.Find(OtherActor);
    if (!HandlesPtr)
    {
        return;
    }

    TArray<FActiveGameplayEffectHandle>& Handles = *HandlesPtr;
    for (const FActiveGameplayEffectHandle& Handle : Handles)
    {
        if (Handle.IsValid())
        {
            bool bRemoved = ASC->RemoveActiveGameplayEffect(Handle);
        }
    }
    EffectHandles.Remove(OtherActor);

}