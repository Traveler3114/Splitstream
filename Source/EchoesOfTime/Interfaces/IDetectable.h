#pragma once

#include "UObject/Interface.h"
#include "IDetectable.generated.h"

// Detectable.h
UINTERFACE(MinimalAPI, Blueprintable)
class UDetectable : public UInterface
{
    GENERATED_BODY()
};

class IDetectable
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detection")
    void OnDetected(AActor* Detector);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detection")
    void OnLost(AActor* Detector);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detection")
    void OnFullyDetected(AActor* DetectingActor);

};