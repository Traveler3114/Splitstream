// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SecurityDocumentActor.generated.h"

class UArrowComponent;
class UWidgetComponent;
class USceneComponent;

UCLASS()
class ECHOESOFTIME_API ASecurityDocumentActor : public AActor
{
    GENERATED_BODY()

public:
    ASecurityDocumentActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UWidgetComponent* WidgetComp;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UArrowComponent* ArrowComp;
};