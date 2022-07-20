// Fill out your copyright notice in the Description page of Project Settings.


#include "GEII_PortalPlayerController.h"

#include "PortalManager.h"

APortalManager* AGEII_PortalPlayerController::GetPortalManager()
{
	return PortalManager;
}

AGEII_PortalProjectCharacter* AGEII_PortalPlayerController::GetPortalCharacter()
{
	return PortalCharacter;
}


FMatrix AGEII_PortalPlayerController::GetCameraProjectionMatrix()
{
	//Get camera projection matrix to apply it on the portal manager update portal view function
	FMatrix ProjectionMatrix;

	if (GetLocalPlayer() != nullptr)
	{
		FSceneViewProjectionData PlayerProjectionData;
		GetLocalPlayer()->GetProjectionData(GetLocalPlayer()->ViewportClient->Viewport,EStereoscopicPass::eSSP_FULL,PlayerProjectionData);
		ProjectionMatrix = PlayerProjectionData.ProjectionMatrix;
	}
	return ProjectionMatrix;
}

void AGEII_PortalPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//Spawn portal manager into the scene and attach it to the player controller
	PortalManager = GetWorld()->SpawnActor<APortalManager>(PortalManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
	PortalManager->AttachToActor(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
	PortalManager->SetControllerOwner(this);
	PortalManager->SetControllerOwner(this);
	PortalManager->SetPortalClass(PortalClass);
	PortalCharacter->SetPlayerClass(PlayerCharacterClass);

	PortalProjectHUD = Cast<AGEII_PortalProjectHUD>(GetHUD());
}


void AGEII_PortalPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (PortalCharacter)
	{
		SetHUDHealth(PortalCharacter->GetHealth(), PortalCharacter->GetMaxHealth());
	}
}


void AGEII_PortalPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	//CHeck if hud is valid, if not, then cast to the project hud
	PortalProjectHUD = PortalProjectHUD == nullptr ? Cast<AGEII_PortalProjectHUD>(GetHUD()) : PortalProjectHUD;

	if (PortalProjectHUD && PortalProjectHUD->CharacterOverlay && PortalProjectHUD->CharacterOverlay->HealthBar && PortalProjectHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health / MaxHealth;
		PortalProjectHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);

		FString HealthText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Health));
		PortalProjectHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void AGEII_PortalPlayerController::SetHUDScore(float Score)
{
	PortalProjectHUD = PortalProjectHUD == nullptr ? Cast<AGEII_PortalProjectHUD>(GetHUD()) : PortalProjectHUD;

	if (PortalProjectHUD && PortalProjectHUD->CharacterOverlay && PortalProjectHUD->CharacterOverlay->ScoreAmount)
	{
		FString ScoreAmountText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		PortalProjectHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreAmountText));
	}
}

void AGEII_PortalPlayerController::SetHUDDeaths(int32 Deaths)
{
	PortalProjectHUD = PortalProjectHUD == nullptr ? Cast<AGEII_PortalProjectHUD>(GetHUD()) : PortalProjectHUD;

	if (PortalProjectHUD && PortalProjectHUD->CharacterOverlay && PortalProjectHUD->CharacterOverlay->DeathsAmount)
	{
		FString DeathsAmountText = FString::Printf(TEXT("%d"), Deaths);
		PortalProjectHUD->CharacterOverlay->DeathsAmount->SetText(FText::FromString(DeathsAmountText));
	}
}

void AGEII_PortalPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	PortalProjectHUD = PortalProjectHUD == nullptr ? Cast<AGEII_PortalProjectHUD>(GetHUD()) : PortalProjectHUD;

	if (PortalProjectHUD && PortalProjectHUD->CharacterOverlay && PortalProjectHUD->CharacterOverlay->AmmoAmount)
	{
		FString AmmoAmountText = FString::Printf(TEXT("%d"), Ammo);
		PortalProjectHUD->CharacterOverlay->AmmoAmount->SetText(FText::FromString(AmmoAmountText));
	}
}

void AGEII_PortalPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	PortalProjectHUD = PortalProjectHUD == nullptr ? Cast<AGEII_PortalProjectHUD>(GetHUD()) : PortalProjectHUD;

	if (PortalProjectHUD && PortalProjectHUD->CharacterOverlay && PortalProjectHUD->CharacterOverlay->CarriedAmmoAmount)
	{
		FString CarriedAmmoAmmountText = FString::Printf(TEXT("%d"), Ammo);
		PortalProjectHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoAmmountText));
	}
}
