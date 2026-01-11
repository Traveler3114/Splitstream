#include "IllegalArea.h"
#include "Components/BoxComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"

#define ILLEGALAREA_LOG(Text, ...) \
    UE_LOG(LogTemp, Warning, TEXT("[IllegalArea] " Text), ##__VA_ARGS__)

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
    ILLEGALAREA_LOG("BeginPlay: Setting up overlap delegates for %s", *GetName());

    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AIllegalArea::OnOverlapBegin);
    TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AIllegalArea::OnOverlapEnd);
}

void AIllegalArea::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ILLEGALAREA_LOG("OnOverlapBegin: Area=%s ThisComp=%s (%p) OtherActor=%s (%p) OtherComp=%s (%p)",
        *GetName(),
        OverlappedComp ? *OverlappedComp->GetName() : TEXT("NULL"), OverlappedComp,
        OtherActor ? *OtherActor->GetName() : TEXT("NULL"), OtherActor,
        OtherComp ? *OtherComp->GetName() : TEXT("NULL"), OtherComp);

    if (!OtherActor || !GameplayEffectClass)
    {
        ILLEGALAREA_LOG("OnOverlapBegin: Early exit, OtherActor or GameplayEffectClass missing");
        return;
    }

    // Check for Ability System
    IAbilitySystemInterface* AbilitySystemActor = Cast<IAbilitySystemInterface>(OtherActor);
    if (!AbilitySystemActor)
    {
        ILLEGALAREA_LOG("OnOverlapBegin: Actor %s does not implement IAbilitySystemInterface", *OtherActor->GetName());
        return;
    }

    UAbilitySystemComponent* ASC = AbilitySystemActor->GetAbilitySystemComponent();
    if (!ASC)
    {
        ILLEGALAREA_LOG("OnOverlapBegin: Could not get ASC from %s", *OtherActor->GetName());
        return;
    }

    ILLEGALAREA_LOG("OnOverlapBegin: Got ASC %p for Actor %s", ASC, *OtherActor->GetName());

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
        ILLEGALAREA_LOG("OnOverlapBegin: SpecHandle INVALID for %s", *OtherActor->GetName());
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
        ILLEGALAREA_LOG("OnOverlapBegin: Applied Effect %s to %s (ASC=%p), Handle=%s",
            *GameplayEffectClass->GetName(), *OtherActor->GetName(), ASC, *EffectHandle.ToString());
    }
    else
    {
        ILLEGALAREA_LOG("OnOverlapBegin: EffectHandle INVALID for %s", *OtherActor->GetName());
    }

    // Print all EffectHandles contents
    TArray<FString> MapEntries;
    for (auto& Entry : EffectHandles)
    {
        FString HandlesStr;
        for (const FActiveGameplayEffectHandle& H : Entry.Value)
        {
            HandlesStr += H.ToString() + TEXT(", ");
        }
        FString EntryStr = FString::Printf(TEXT("[Key:%s@%p] Handles:%s"),
            Entry.Key.IsValid() ? *Entry.Key->GetName() : TEXT("NULL"),
            Entry.Key.Get(),
            *HandlesStr);
        MapEntries.Add(EntryStr);
    }
    ILLEGALAREA_LOG("EffectHandles map after add: %s", *FString::Join(MapEntries, TEXT(" | ")));
}

void AIllegalArea::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ILLEGALAREA_LOG("OnOverlapEnd: Area=%s ThisComp=%s (%p) OtherActor=%s (%p) OtherComp=%s (%p)",
        *GetName(),
        OverlappedComp ? *OverlappedComp->GetName() : TEXT("NULL"), OverlappedComp,
        OtherActor ? *OtherActor->GetName() : TEXT("NULL"), OtherActor,
        OtherComp ? *OtherComp->GetName() : TEXT("NULL"), OtherComp);

    if (!OtherActor)
    {
        ILLEGALAREA_LOG("OnOverlapEnd: Early exit, OtherActor missing");
        return;
    }

    IAbilitySystemInterface* AbilitySystemActor = Cast<IAbilitySystemInterface>(OtherActor);
    if (!AbilitySystemActor)
    {
        ILLEGALAREA_LOG("OnOverlapEnd: Actor %s does not implement IAbilitySystemInterface", *OtherActor->GetName());
        return;
    }

    UAbilitySystemComponent* ASC = AbilitySystemActor->GetAbilitySystemComponent();
    if (!ASC)
    {
        ILLEGALAREA_LOG("OnOverlapEnd: Could not get ASC from %s", *OtherActor->GetName());
        return;
    }

    ILLEGALAREA_LOG("OnOverlapEnd: Got ASC %p for Actor %s", ASC, *OtherActor->GetName());

    TArray<FActiveGameplayEffectHandle>* HandlesPtr = EffectHandles.Find(OtherActor);
    if (!HandlesPtr)
    {
        ILLEGALAREA_LOG("OnOverlapEnd: NO EffectHandles found for OtherActor %s (ptr %p) -- current keys:", *OtherActor->GetName(), OtherActor);
        for (auto& Entry : EffectHandles)
        {
            ILLEGALAREA_LOG("-- Map has Actor: %s Addr=%p", Entry.Key.IsValid() ? *Entry.Key->GetName() : TEXT("NULL"), Entry.Key.Get());
        }
        return;
    }

    TArray<FActiveGameplayEffectHandle>& Handles = *HandlesPtr;
    for (const FActiveGameplayEffectHandle& Handle : Handles)
    {
        if (Handle.IsValid())
        {
            bool bRemoved = ASC->RemoveActiveGameplayEffect(Handle);
            ILLEGALAREA_LOG("OnOverlapEnd: Removed EffectHandle %s from %s (Result=%d)", *Handle.ToString(), *OtherActor->GetName(), bRemoved);
        }
        else
        {
            ILLEGALAREA_LOG("OnOverlapEnd: EffectHandle for %s is INVALID!", *OtherActor->GetName());
        }
    }
    EffectHandles.Remove(OtherActor);

    // Print all EffectHandles contents after removal
    TArray<FString> MapEntries;
    for (auto& Entry : EffectHandles)
    {
        FString HandlesStr;
        for (const FActiveGameplayEffectHandle& H : Entry.Value)
        {
            HandlesStr += H.ToString() + TEXT(", ");
        }
        FString EntryStr = FString::Printf(TEXT("[Key:%s@%p] Handles:%s"),
            Entry.Key.IsValid() ? *Entry.Key->GetName() : TEXT("NULL"),
            Entry.Key.Get(),
            *HandlesStr);
        MapEntries.Add(EntryStr);
    }
    ILLEGALAREA_LOG("EffectHandles map after remove: %s", *FString::Join(MapEntries, TEXT(" | ")));
}