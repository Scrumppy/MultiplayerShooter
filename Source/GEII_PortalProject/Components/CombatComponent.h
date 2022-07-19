// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatState.h"
#include "Components/ActorComponent.h"
#include "GEII_PortalProject/Weapons/Weapon.h"
#include "GEII_PortalProject/Weapons/WeaponTypes.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GEII_PORTALPROJECT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	friend class AGEII_PortalProjectCharacter;
	
	void EquipWeapon(AWeapon* WeaponToEquip);

	bool ShouldSwapWeapons();

	void Reload();

	void FinishReloading();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	UPROPERTY(VisibleAnywhere, Category = "Weapon Inventory")
	TArray<AWeapon*> WeaponArray;

	void SwitchWeapons(AWeapon* WeaponToEquip);

protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_PrimaryWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	void FireButtonPressed(bool bPressed);

	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	int32 AmountToReload();

	//Weapon Equips

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);

	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);

	void DropEquippedWeapon();

	//Crosshair
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void UpdateCarriedAmmo(AWeapon* WeaponToUpdate);

	void ResetCarriedAmmo();


private:
	UPROPERTY()
	AGEII_PortalProjectCharacter* PortalCharacter;

	UPROPERTY()
	class AGEII_PortalPlayerController* PortalController;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_PrimaryWeapon)
	AWeapon* PrimaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;	

	UPROPERTY(Replicated)
	bool bAiming;
	
	bool bFireButtonPressed;

	//Weapon Cooldown
	FTimerHandle FireTimer;

	bool bCanFire = true;

	void StartFireTimer();

	void FireTimerFinished();

	bool CanFire();

	//Carried ammo for the current equipped ammo
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo);
	int32 CarriedAmmo;

	UPROPERTY(EditAnywhere);
	int32 MaxCarriedAmmo = 25;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	//Map data structure for the carried ammo
	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 10;
	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 5;
	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 2;

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();
};
