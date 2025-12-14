// Fill out your copyright notice in the Description page of Project Settings.


#include "SearchableActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ActorComponents/SearchComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASearchableActor::ASearchableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;
}

// Called when the game starts or when spawned
void ASearchableActor::BeginPlay()
{
	Super::BeginPlay();
    SearchComponent = FindComponentByClass<USearchComponent>();
    if (SearchComponent)
    {
        SearchComponent->OnSearchComplete.AddDynamic(this, &ASearchableActor::OnSearchComplete);
    }
}

void ASearchableActor::Interact_Implementation(AActor* Interactor)
{
    if (SearchComponent)
        SearchComponent->Interact(Interactor);
}

void ASearchableActor::OnSearchComplete()
{
}


