#pragma once

#include "UObject/Interface.h"
#include "IDetectable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UDetectable : public UInterface
{
    GENERATED_BODY()
};

class SPLITSTREAM_API IDetectable
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detection")
    void OnDetected(AActor* Detector);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detection")
    void OnLost(AActor* Detector);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detection")
    void OnFullyDetected(AActor* DetectingActor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detection")
    void OnForceDetectionEnd(AActor* Detector);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detection")
    bool IsActorAlreadyDetected(AActor* DetectingActor) const;

};