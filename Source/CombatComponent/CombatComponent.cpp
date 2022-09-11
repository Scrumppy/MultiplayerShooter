// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GEII_PortalProject/GEII_PortalPlayerController.h"
#include "GEII_PortalProject/GEII_PortalProjectCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, PrimaryWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PortalCharacter)
	{
		if (PortalCharacter->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;

	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	PortalController = PortalController == nullptr ? Cast<AGEII_PortalPlayerController>(PortalCharacter->Controller) : PortalController;
	if (PortalController)
	{
		PortalController->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
}

void UCombatComponent::UpdateCarriedAmmo(AWeapon* WeaponToUpdate)
{
	if (WeaponToUpdate == nullptr) return;
	if (CarriedAmmoMap.Contains(WeaponToUpdate->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[WeaponToUpdate->GetWeaponType()];
	}

	PortalController = PortalController == nullptr ? Cast<AGEII_PortalPlayerController>(PortalCharacter->Controller) : PortalController;
	if (PortalController)
	{
		PortalController->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::ResetCarriedAmmo()
{
	CarriedAmmoMap.Reset();
	CarriedAmmo = 0;
	UE_LOG(LogTemp, Warning, TEXT("reset carried ammo"))

	PortalController = PortalController == nullptr ? Cast<AGEII_PortalPlayerController>(PortalCharacter->Controller) : PortalController;
	if (PortalController)
	{
		PortalController->SetHUDCarriedAmmo(CarriedAmmo);
	}
}


void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] += FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);
		UpdateCarriedAmmo(EquippedWeapon);
	}
	//If weapon is empty, automatically reload
	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		ServerFire(HitResult.ImpactPoint);
		StartFireTimer();
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize); 
	}

	//Crosshair vectors
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y/ 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	//Deproject crosshair vectors to world
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || PortalCharacter == nullptr) return;

	if (EquippedWeapon && PortalCharacter)
	{
		PortalCharacter->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay);	
	}
}

void UCombatComponent::FireTimerFinished()
{
	bCanFire = true;
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (!ensure(EquippedWeapon != nullptr)) return;
	if (PortalCharacter && CombatState == ECombatState::ECS_Unoccupied)
	{
		PortalCharacter->PlayFireMontage();
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (PortalCharacter == nullptr || WeaponToEquip == nullptr) return;

	if (WeaponArray.Num() == 0)
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}
	else
	{
		for (int i = 0; i < WeaponArray.Num(); i++)
		{
			if (WeaponArray[i] != nullptr && WeaponToEquip->GetClass() == WeaponArray[i]->GetClass())
			{
				UE_LOG(LogTemp, Warning, TEXT("Same Weapon?"));
				return;
			}
			else if (WeaponArray[i] != nullptr)
			{
				EquipSecondaryWeapon(WeaponToEquip);
			}
		}
	}
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	PrimaryWeapon = WeaponToEquip;
	EquippedWeapon = PrimaryWeapon;
	WeaponArray.Add(PrimaryWeapon);
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	UE_LOG(LogTemp, Warning, TEXT("Item %s"), WeaponArray[0]);
	UE_LOG(LogTemp, Warning, TEXT("Array Size %i"), WeaponArray.Num());
	UE_LOG(LogTemp, Warning, TEXT("Class %i"), WeaponArray[0]->GetClass());

	//Put Weapon In HandSocket
	const USkeletalMeshSocket* HandSocket = PortalCharacter->GetMesh1P()->GetSocketByName(FName("hand_lSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, PortalCharacter->GetMesh1P());
	}

	EquippedWeapon->SetOwner(PortalCharacter);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo(EquippedWeapon);
	if (EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	SecondaryWeapon = WeaponToEquip;
	EquippedWeapon = SecondaryWeapon;
	PrimaryWeapon->SetActorHiddenInGame(true);
	PrimaryWeapon->SetWeaponState(EWeaponState::EWS_Stored);
	WeaponArray.Add(SecondaryWeapon);

	SecondaryWeapon->SetActorHiddenInGame(false);
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	UE_LOG(LogTemp, Warning, TEXT("Item %s"), WeaponArray[0]);
	UE_LOG(LogTemp, Warning, TEXT("Array Size %i"), WeaponArray.Num());
	UE_LOG(LogTemp, Warning, TEXT("Class %i"), WeaponArray[0]->GetClass());

	//Put Weapon In HandSocket
	const USkeletalMeshSocket* HandSocket = PortalCharacter->GetMesh1P()->GetSocketByName(FName("hand_lSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, PortalCharacter->GetMesh1P());
	}

	EquippedWeapon->SetOwner(PortalCharacter);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo(EquippedWeapon);
	if (EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
	PortalCharacter->WeaponCounter += 1;
	
}

void UCombatComponent::SwitchWeapons(AWeapon* WeaponToEquip)
{
	UE_LOG(LogTemp, Warning, TEXT("Item %p"), WeaponToEquip);
	WeaponToEquip->SetActorHiddenInGame(false);
	if (WeaponToEquip == WeaponArray[0])
	{
		EquippedWeapon = PrimaryWeapon;
		PrimaryWeapon->SetActorHiddenInGame(false);
		SecondaryWeapon->SetActorHiddenInGame(true);

		UE_LOG(LogTemp, Warning, TEXT("Primary"));

	}
	else if (WeaponToEquip == WeaponArray[1])
	{
		EquippedWeapon = SecondaryWeapon;
		SecondaryWeapon->SetActorHiddenInGame(false);
		PrimaryWeapon->SetActorHiddenInGame(true);
		UE_LOG(LogTemp, Warning, TEXT("Secondary"));

	}
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo(EquippedWeapon);
	
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && PortalCharacter)
	{	
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* HandSocket = PortalCharacter->GetMesh1P()->GetSocketByName(FName("hand_lSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, PortalCharacter->GetMesh1P());
		}
	}
}

void UCombatComponent::OnRep_PrimaryWeapon()
{
	if (PrimaryWeapon && PortalCharacter)
	{
		PrimaryWeapon->SetWeaponState(EWeaponState::EWS_Stored);

	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && PortalCharacter)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_Stored);

	}
}
bool UCombatComponent::ShouldSwapWeapons()
{
	return (PrimaryWeapon != nullptr && SecondaryWeapon != nullptr);
}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading)
	{
		ServerReload(); 
	}
}

int32 UCombatComponent::AmountToReload()
{
	//Calculate amount of ammo to reload, cannot reload more ammo than the weapon max ammo capacity
	if (EquippedWeapon == nullptr) return 0;
	
	int32 RoomInMag = EquippedWeapon->GetMaxAmmo() - EquippedWeapon->GetAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
	
}

void UCombatComponent::FinishReloading()
{
	if (PortalCharacter == nullptr || EquippedWeapon == nullptr) return;

	if (PortalCharacter->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		int32 ReloadAmount = AmountToReload();

		if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
		{
			// will never be reater than the amount of ammo we are carrying
			CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
			CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		}

		PortalController = PortalController == nullptr ? Cast<AGEII_PortalPlayerController>(PortalCharacter->Controller) : PortalController;
		if (PortalController)
		{
			PortalController->SetHUDCarriedAmmo(CarriedAmmo);
		}

		EquippedWeapon->AddAmmo(-ReloadAmount);
		EquippedWeapon->PlayReloadSound();
	}
	
	if (bFireButtonPressed)
	{
		Fire(); 
	}
}


void UCombatComponent::ServerReload_Implementation()
{
	if (PortalCharacter == nullptr || EquippedWeapon == nullptr) return;
	
	CombatState = ECombatState::ECS_Reloading;
	FinishReloading();
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		break;	
	
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;	
	}
}

