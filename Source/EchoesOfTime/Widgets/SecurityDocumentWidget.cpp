// Fill out your copyright notice in the Description page of Project Settings.


#include "SecurityDocumentWidget.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void USecurityDocumentWidget::AddColorWireLine(TArray<EWireColor> WireColors) 
{
	if (!ColorWireBox) 
	{
		return;
	}
	// For each wire color, create a colored box and add it to the horizontal box
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
			ColorBox->SetPadding(FMargin(5.0f));
			ColorBox->SetDesiredSizeScale(FVector2D(1.0f, 1.0f));
			if (UVerticalBoxSlot* MySlot = Cast<UVerticalBoxSlot>(ColorWireBox->AddChild(ColorBox)))
			{
				MySlot->SetPadding(FMargin(5.0f));
			}
		}
	}
}