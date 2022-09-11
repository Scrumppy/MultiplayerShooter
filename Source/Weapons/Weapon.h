// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName =  "Initial State"),
	EWS_Equipped UMETA(DisplayName =  "Equipped"),
	EWS_Stored UMETA(DisplayName = "Stored"),
	EWS_Dropped UMETA(DisplayName =  "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class GEII_PORTALPROJECT_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_Owner() override;

	void SetHUDAmmo();
	
	virtual void Fire(const FVector& HitTarget);

	void Dropped();

	void AddAmmo(int32 AmmoToAdd);

	void PlayReloadSound();

	//Weapon Cooldown
	UPROPERTY(EditAnywhere, Category = "Combat")
	float FireDelay = 0.2f;

	UPROPERTY()
	bool bPickedUp;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere,	BlueprintReadWrite, meta = (AllowPrivateAccess = true), Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USceneComponent* MuzzleLocation;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	UParticleSystem* MuzzleParticle;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Weapon Properties", meta = (AllowPrivateAccess = true))
	USoundBase* FireSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Weapon Properties", meta = (AllowPrivateAccess = true))
	USoundBase* PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Weapon Properties", meta = (AllowPrivateAccess = true))
	USoundBase* ReloadSound;

	const USkeletalMeshSocket* MuzzleSocket;

	//Replicate Weapon State
	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	UFUNCTION()
	void OnRep_Ammo();

	void SpendRound();
	
	UPROPERTY(EditAnywhere)
	int32 MaxAmmo;

	UPROPERTY()
	class AGEII_PortalProjectCharacter* PortalOwnerCharacter;

	UPROPERTY()
	class AGEII_PortalPlayerController* PortalOwnerController;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	

public:	
	void SetWeaponState(EWeaponState State);

	bool IsEmpty();
	
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }

	FORCEINLINE USceneComponent* GetMuzzleLocation() const { return MuzzleLocation; }

	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

	FORCEINLINE int32 GetAmmo() const { return Ammo; }

	FORCEINLINE int32 GetMaxAmmo() const { return MaxAmmo; }
};
