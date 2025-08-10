#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DefaultCharacter.generated.h"

UCLASS()
class ECHOESOFTIME_API ADefaultCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADefaultCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UWidgetComponent* PlayerInfoWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UArrowComponent* ArrowComp;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerInfo)
	FString ReplicatedPlayerName;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerInfo)
	UTexture2D* ReplicatedAvatarTexture;

	UFUNCTION()
	void OnRep_PlayerInfo();

};