#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IServerActionInterface.generated.h"

// 1. Define the generic payload struct.
USTRUCT(BlueprintType)
struct FServerActionPayload
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    bool   BoolValue = false;

    UPROPERTY(BlueprintReadWrite)
    float   FloatValue = 0.f;

    UPROPERTY(BlueprintReadWrite)
    int32   IntValue = 0;

    UPROPERTY(BlueprintReadWrite)
    UObject* ObjectValue = nullptr;

    UPROPERTY(BlueprintReadWrite)
    FString StringValue;

    // Add more fields as you need for future extensibility!
};

// 2. Declare the Unreal interface type.
UINTERFACE(MinimalAPI)
class UServerActionInterface : public UInterface
{
    GENERATED_BODY()
};

// 3. Implement your pure C++ interface. 
class ECHOESOFTIME_API IServerActionInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void ExecuteServerAction(const FServerActionPayload& Payload);
};