// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GEII_PortalProjectCharacter.h"
#include "GEII_PortalProjectHUD.h"
#include "GameFramework/PlayerController.h"
#include "GEII_PortalPlayerController.generated.h"

/**
 * 
 */
class APortalManager;

UCLASS()

class GEII_PORTALPROJECT_API AGEII_PortalPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	
	APortalManager* GetPortalManager();
	
	//TSubclassOf<AGEII_PortalProjectCharacter> GetPlayerCharacter();
	
	class AGEII_PortalProjectCharacter* GetPortalCharacter();
	
	FMatrix GetCameraProjectionMatrix();
	
	//Portal class to spawn 
	UPROPERTY(EditDefaultsOnly, Category = Portal)
	TSubclassOf<class APortal> PortalClass;
	
	//Portal manager class to spawn 
	UPROPERTY(EditDefaultsOnly, Category = Portal)
	TSubclassOf<class APortalManager> PortalManagerClass;
	
	UPROPERTY(EditDefaultsOnly, Category = Player)
	TSubclassOf<class AGEII_PortalProjectCharacter> PlayerCharacterClass;

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	
	virtual void OnPossess(APawn* InPawn) override;

private:
	class AGEII_PortalProjectHUD* PortalProjectHUD;


protected:
	
	virtual void BeginPlay() override;
	
	APortalManager* PortalManager;
	
	AGEII_PortalProjectCharacter* PortalCharacter;
};
