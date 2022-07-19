// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GEII_PortalProject/Weapons/Weapon.h"
#include "WeaponPickupSpawnPoint.generated.h"

UCLASS()
class GEII_PORTALPROJECT_API AWeaponPickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponPickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	void SpawnWeapon();
	void SpawnWeaponTimerFinished();

	UFUNCTION()
	void StartSpawnWeaponTimer(AActor* DestroyedActor);

	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;
	
private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> WeaponClass;

	UPROPERTY()
	AWeapon* SpawnedWeapon;

	FTimerHandle SpawnWeaponTimer;

	UPROPERTY(EditAnywhere)
	float SpawnDelay;

};
