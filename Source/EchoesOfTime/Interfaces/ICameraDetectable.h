#pragma once

#include "UObject/Interface.h"
#include "ICameraDetectable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UCameraDetectable : public UInterface
{
    GENERATED_BODY()
};

class ICameraDetectable
{
    GENERATED_BODY()

public:
    // Called when detected by a camera
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera")
    void OnDetectedByCamera(class ASecurityCamera* Camera);

    // Called when lost by a camera
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera")
    void OnLostByCamera(class ASecurityCamera* Camera);
};