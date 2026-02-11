#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IServerActionInterface.generated.h"

/**
 * Generic payload for server-side actions dispatched through IServerActionInterface.
 * Contains common value types so a single RPC can carry various data without
 * needing a unique RPC per action.
 */
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
};

/**
 * Interface for actors that can execute a generic server-authoritative action.
 *
 * Allows any actor to receive a server action with a flexible payload,
 * typically forwarded via ADefaultPlayerController::ServerExecuteAction().
 */
UINTERFACE(MinimalAPI)
class UServerActionInterface : public UInterface
{
    GENERATED_BODY()
};

class SPLITSTREAM_API IServerActionInterface
{
    GENERATED_BODY()

public:
    /**
     * Executes a server-authoritative action with the given payload.
     * @param Payload  A generic data payload containing the action parameters.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void ExecuteServerAction(const FServerActionPayload& Payload);
};