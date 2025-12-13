// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Actors/Wire/WireActor.h"
#include "SecurityDocumentWidget.generated.h"

/**
 * 
 */
UCLASS()
class ECHOESOFTIME_API USecurityDocumentWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	void AddColorWireLine(TArray<EWireColor> WireColors);

protected:
	/** Vertical box in the widget (bind this in UMG). */
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* ColorWireBox;
};
