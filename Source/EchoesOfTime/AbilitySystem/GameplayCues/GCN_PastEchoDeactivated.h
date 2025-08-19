#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GCN_PastEchoDeactivated.generated.h"

UCLASS()
class ECHOESOFTIME_API UGCN_PastEchoDeactivated : public UGameplayCueNotify_Static
{
    GENERATED_BODY()

public:
    virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};