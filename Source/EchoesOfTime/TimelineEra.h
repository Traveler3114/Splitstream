#pragma once

#include "CoreMinimal.h"
#include "TimelineEra.generated.h"

UENUM(BlueprintType)
enum class ETimelineEra : uint8
{
    Past    UMETA(DisplayName = "Past"),
    Future  UMETA(DisplayName = "Future")
};