#include "FutureGAShowGhosts.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Actors/TimeObjects/GhostCharacterActor.h"
#include "Components/PrimitiveComponent.h"

UFutureGAShowGhosts::UFutureGAShowGhosts()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	// Keep your existing tag setup as-is (assuming activation works now)
	FGameplayTagContainer Tags;
	FGameplayTag MyTag = FGameplayTag::RequestGameplayTag(FName("Character.Ability.ShowGhosts"));
	Tags.AddTag(MyTag);
	SetAssetTags(Tags);
}

void UFutureGAShowGhosts::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UWorld* World = GetWorld();
	APlayerController* PC = ActorInfo ? Cast<APlayerController>(ActorInfo->PlayerController.Get()) : nullptr;
	APawn* LocalPawn = ActorInfo ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!LocalPawn && PC) { LocalPawn = PC->GetPawn(); }

	if (!PC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility*/false, /*bWasCancelled*/true);
		return;
	}

	TArray<AActor*> Found;
	if (World)
	{
		UGameplayStatics::GetAllActorsOfClass(World, AGhostCharacterActor::StaticClass(), Found);

		for (AActor* Ghost : Found)
		{
			if (!Ghost) continue;

			Ghost->SetOwner(LocalPawn);
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility*/false, /*bWasCancelled*/false);
}