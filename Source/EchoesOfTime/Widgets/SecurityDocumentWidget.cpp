// Fill out your copyright notice in the Description page of Project Settings.


#include "SecurityDocumentWidget.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Actors/ProceduralLevelGenerator.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Components/VerticalBoxSlot.h"

void USecurityDocumentWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Find the generator and get the wire sequence
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	AProceduralLevelGenerator* Gen = Cast<AProceduralLevelGenerator>(
		UGameplayStatics::GetActorOfClass(World, AProceduralLevelGenerator::StaticClass())
	);

	if (!Gen)
	{
		return;
	}

	if (Gen->PastWireDeviceSequence.Num() == 0)
	{
		return;
	}

	// Build color array from sequence
	TArray<EWireColor> WireColors;
	WireColors.Reserve(Gen->PastWireDeviceSequence.Num());
	for (const FWireSequenceStep& Step : Gen->PastWireDeviceSequence)
	{
		WireColors.Add(Step.WireColor);
	}

	AddColorWireLine(WireColors);
}


void USecurityDocumentWidget::AddColorWireLine(TArray<EWireColor> WireColors) 
{
	if (!ColorWireBox) 
	{
		return;
	}
	// For each wire color, create a colored box and add it to the horizontal box
	ColorWireBox->ClearChildren();
	for (EWireColor WireColor : WireColors) 
	{
		UBorder* ColorBox = NewObject<UBorder>(this);
		if (ColorBox) 
		{
			FLinearColor LinearColor;
			switch (WireColor) 
			{
				case EWireColor::Red:
					LinearColor = FLinearColor::Red;
					break;
				case EWireColor::Green:
					LinearColor = FLinearColor::Green;
					break;
				case EWireColor::Blue:
					LinearColor = FLinearColor::Blue;
					break;
				case EWireColor::Yellow:
					LinearColor = FLinearColor::Yellow;
					break;
				case EWireColor::Orange:
					LinearColor = FLinearColor(1.0f, 0.5f, 0.0f);
					break;
				case EWireColor::Purple:
					LinearColor = FLinearColor(0.5f, 0.0f, 0.5f);
					break;
				default:
					LinearColor = FLinearColor::Black;
					break;
			}
			ColorBox->SetBrushColor(LinearColor);
			ColorBox->SetPadding(ColorBoxPadding);
			ColorBox->SetDesiredSizeScale(ColorBoxDesiredSizeScale);
			if (UVerticalBoxSlot* MySlot = Cast<UVerticalBoxSlot>(ColorWireBox->AddChild(ColorBox)))
			{
				MySlot->SetPadding(ColorBoxSlotPadding);
			}
		}
	}
}