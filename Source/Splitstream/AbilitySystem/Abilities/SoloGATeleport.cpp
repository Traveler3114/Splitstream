#include "SoloGATeleport.h"
#include "Actors/PointActors/RefPointActor.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

USoloGATeleport::USoloGATeleport()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    FGameplayTagContainer Tags;
    FGameplayTag MyTag = TAG_Character_Ability_Solo_Teleport;
    Tags.AddTag(MyTag);
    SetAssetTags(Tags);


    ActivationOwnedTags.AddTag(TAG_Character_Status_Teleporting);
}



void USoloGATeleport::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    ACharacter* PlayerCharacter = Cast<ACharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);
    if (!PlayerCharacter)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // Get both reference points directly
    TArray<AActor*> RefPoints;
    UGameplayStatics::GetAllActorsOfClass(PlayerCharacter->GetWorld(), ARefPointActor::StaticClass(), RefPoints);
    if (RefPoints.Num() < 2)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // Reference points
    const FVector RefA = RefPoints[0]->GetActorLocation();
    const FVector RefB = RefPoints[1]->GetActorLocation();

    // Compute player's relative position to the closest reference point
    const FVector PlayerLoc = PlayerCharacter->GetActorLocation();
    const float DistToA = FVector::Dist(PlayerLoc, RefA);
    const float DistToB = FVector::Dist(PlayerLoc, RefB);

    // If closer to RefA, teleport relative to RefB; if closer to RefB, teleport relative to RefA
    FVector NewLocation;
    if (DistToA < DistToB)
    {
        NewLocation = RefB + (PlayerLoc - RefA);
    }
    else
    {
        NewLocation = RefA + (PlayerLoc - RefB);
    }

    PlayerCharacter->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}