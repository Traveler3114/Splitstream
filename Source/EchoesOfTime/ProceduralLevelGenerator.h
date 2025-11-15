#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralLevelGenerator.generated.h"

enum class ETimelineEra : uint8;

USTRUCT(BlueprintType)
struct FRandomDate
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 Year;

    UPROPERTY(BlueprintReadOnly)
    int32 Month;

    UPROPERTY(BlueprintReadOnly)
    int32 Day;

    FRandomDate() : Year(2020), Month(1), Day(1) {}
    bool operator<(const FRandomDate& Other) const
    {
        if (Year != Other.Year) return Year < Other.Year;
        if (Month != Other.Month) return Month < Other.Month;
        return Day < Other.Day;
    }
};

UCLASS()
class ECHOESOFTIME_API AProceduralLevelGenerator : public AActor
{
    GENERATED_BODY()
public:
    AProceduralLevelGenerator();
    void HandlePastSpawns();
    void HandleFutureSpawns();

    UPROPERTY(Replicated, BlueprintReadOnly)
    FRandomDate PastDate;

    UPROPERTY(Replicated, BlueprintReadOnly)
    FRandomDate FutureDate;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Civilian")
    TSubclassOf<class ACivilianCharacter> CivilianBPClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Newspaper")
    TSubclassOf<class ANewspaperActor> NewspaperBPClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Items")
    TSubclassOf<class ASearchableActor> SearchableItemBPClass;

    UPROPERTY(Replicated, BlueprintReadOnly)
    FString PastLeverOrderString;
protected:
    virtual void BeginPlay() override;
    void SpawnCivilianDeskItems(const TArray<class ACivilianCharacter*>& Civilians, TSubclassOf<class ASearchableActor> ItemClass);
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    FString GenerateRandomCode(int Length = 4) const;

    FRandomDate GeneratePastDate() const;
    FRandomDate GenerateFutureDate(const FRandomDate& MinDate) const;
    FRandomDate GenerateRandomDate() const;

    // --- Helper for era-based spawning ---
    void HandleEraSpawns(
        ETimelineEra Era,
        TArray<ACivilianCharacter*>& OutSpawnedCivilians,
        TArray<class ADeskActor*>& OutDesks
    );
    FString GenerateUniqueName(const TArray<FString>& FirstNames, const TArray<FString>& Surnames, TSet<FString>& UsedNames) const;
};