// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponPickupSpawnPoint.h"

// Sets default values
AWeaponPickupSpawnPoint::AWeaponPickupSpawnPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//bReplicates = true;

}

// Called when the game starts or when spawned
void AWeaponPickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	SpawnWeapon();
}

void AWeaponPickupSpawnPoint::SpawnWeapon()
{
	SpawnedWeapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass, GetActorTransform());
	if (HasAuthority() && SpawnedWeapon)
	{
		//Start spawn timer when pickup is destroyed
		SpawnedWeapon->OnDestroyed.AddDynamic(this, &AWeaponPickupSpawnPoint::StartSpawnWeaponTimer);
	}
}

void AWeaponPickupSpawnPoint::SpawnWeaponTimerFinished()
{
	if (HasAuthority())
	{
		SpawnWeapon();
	}
}

void AWeaponPickupSpawnPoint::StartSpawnWeaponTimer(AActor* DestroyedActor)
{
	GetWorldTimerManager().SetTimer(SpawnWeaponTimer, this, &AWeaponPickupSpawnPoint::SpawnWeaponTimerFinished, SpawnDelay);
}

// Called every frame
void AWeaponPickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (SpawnedWeapon && !SpawnedWeapon->bPickedUp)
	{
		SpawnedWeapon->GetWeaponMesh()->AddLocalRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f));
	}

}

