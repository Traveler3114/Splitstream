#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GCN_ShowGhostLocal.generated.h"

// Triggers only on the locally controlled client for the target actor.
// Makes all actors with tag "Ghost" locally visible. Add SFX/VFX here later.
UCLASS()
class ECHOESOFTIME_API UGCN_ShowGhostLocal : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

private:
	static void SetActorLocalVisibility(AActor* Actor, bool bVisible);
};