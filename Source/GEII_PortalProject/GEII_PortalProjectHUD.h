// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "CharacterOverlay.h"
#include "GameFramework/HUD.h"
#include "GEII_PortalProjectCharacter.h"
#include "GEII_PortalProjectHUD.generated.h"

UCLASS()
class AGEII_PortalProjectHUD : public AHUD
{
	GENERATED_BODY()

public:
	AGEII_PortalProjectHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

	//UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass; 
	
	UCharacterOverlay* CharacterOverlay;

protected:
	virtual void BeginPlay() override;
	void AddCharacterOverlay();
};

