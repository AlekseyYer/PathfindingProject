// Fill out your copyright notice in the Description page of Project Settings.


#include "TestSpawnActor.h"

// Sets default values
ATestSpawnActor::ATestSpawnActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
}

// Called when the game starts or when spawned
void ATestSpawnActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATestSpawnActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

