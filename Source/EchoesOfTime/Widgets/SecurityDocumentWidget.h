#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Actors/Wire/WireActor.h"
#include "SecurityDocumentWidget.generated.h"

UCLASS()
class ECHOESOFTIME_API USecurityDocumentWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void AddColorWireLine(TArray<EWireColor> WireColors);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* ColorWireBox;
};