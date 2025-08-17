#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GCN_PastEchoDeactivated.generated.h"

// Hides all actors with tag "Ghost" locally on the owning client.
UCLASS()
class ECHOESOFTIME_API UGCN_PastEchoDeactivated : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

private:
	static void SetActorLocalVisibility(AActor* Actor, bool bVisible);
};