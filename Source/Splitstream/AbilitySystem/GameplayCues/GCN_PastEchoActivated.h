#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GCN_PastEchoActivated.generated.h"

UCLASS()
class SPLITSTREAM_API UGCN_PastEchoActivated : public UGameplayCueNotify_Static
{
    GENERATED_BODY()

public:
    virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};