// Fill out your copyright notice in the Description page of Project Settings.

#include "IllegalArea.h"
#include "Components/BoxComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/EOTGameplayTags.h"

AIllegalArea::AIllegalArea()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Create the area volume
	AreaVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaVolume"));
	RootComponent = AreaVolume;
	
	AreaVolume->SetBoxExtent(FVector(200.0f, 200.0f, 200.0f));
	AreaVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AreaVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AreaVolume->SetGenerateOverlapEvents(true);
}

void AIllegalArea::BeginPlay()
{
	Super::BeginPlay();
	
	// Only the server should apply gameplay effects
	if (HasAuthority())
	{
		AreaVolume->OnComponentBeginOverlap.AddDynamic(this, &AIllegalArea::OnOverlapBegin);
		AreaVolume->OnComponentEndOverlap.AddDynamic(this, &AIllegalArea::OnOverlapEnd);
	}
}

void AIllegalArea::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || !HasAuthority())
	{
		return;
	}

	// Check if the actor already has an active effect (prevent duplicate applications)
	if (ActiveEffectHandles.Contains(OtherActor))
	{
		return;
	}

	// Get the ability system component
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OtherActor);
	if (!ASI)
	{
		return;
	}

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC || !IllegalAreaEffect)
	{
		return;
	}

	// Apply the gameplay effect
	// Note: The IllegalAreaEffect should be configured to grant the 
	// Character.Status.Illegal.Area tag (TAG_Character_Status_Illegal_Area).
	// This is typically configured in the Blueprint/asset for GE_Illegal.
	// The effect should have infinite duration and be manually removed on overlap end.
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(IllegalAreaEffect, 1.0f, EffectContext);
	if (SpecHandle.IsValid())
	{
		FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		if (ActiveHandle.IsValid())
		{
			// Store the handle so we can remove it later
			ActiveEffectHandles.Add(OtherActor, ActiveHandle);
		}
	}
}

void AIllegalArea::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || !HasAuthority())
	{
		return;
	}

	// Check if we have an active effect for this actor
	FActiveGameplayEffectHandle* HandlePtr = ActiveEffectHandles.Find(OtherActor);
	if (!HandlePtr || !HandlePtr->IsValid())
	{
		// No active effect to remove
		ActiveEffectHandles.Remove(OtherActor);
		return;
	}

	// Get the ability system component
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OtherActor);
	if (!ASI)
	{
		ActiveEffectHandles.Remove(OtherActor);
		return;
	}

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC)
	{
		ActiveEffectHandles.Remove(OtherActor);
		return;
	}

	// Remove the gameplay effect
	bool bRemoved = ASC->RemoveActiveGameplayEffect(*HandlePtr);
	
	// Clean up the map entry
	ActiveEffectHandles.Remove(OtherActor);
}
