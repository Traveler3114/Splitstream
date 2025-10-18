// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "DefaultGameState.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnRestartRequested);

UCLASS()
class ECHOESOFTIME_API ADefaultGameState : public AGameState
{
	GENERATED_BODY()

public:
    FOnRestartRequested OnRestartRequested;

    UFUNCTION(BlueprintCallable)
    void RequestRestart()
    {
        OnRestartRequested.Broadcast();
    }
};
