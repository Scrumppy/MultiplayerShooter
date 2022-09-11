// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupSpawnPoint.h"

// Sets default values
APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnPickup();
}

void APickupSpawnPoint::SpawnPickup()
{
	SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClass, GetActorTransform());
	if (HasAuthority() && SpawnedPickup)
	{
		//Start spawn timer when pickup is destroyed
		SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);
	}
}

void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}

void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	GetWorldTimerManager().SetTimer(SpawnPickupTimer, this, &APickupSpawnPoint::SpawnPickupTimerFinished, SpawnDelay);
}

// Called every frame
void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

