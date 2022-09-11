// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class GEII_PORTALPROJECT_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	void SpawnPickup();
	void SpawnPickupTimerFinished();

	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<APickup> PickupClass;

	UPROPERTY()
	APickup* SpawnedPickup;

	FTimerHandle SpawnPickupTimer;

	UPROPERTY(EditAnywhere)
	float SpawnDelay;

};
