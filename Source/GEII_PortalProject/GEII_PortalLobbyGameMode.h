// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GEII_PortalProjectGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GEII_PortalLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class GEII_PORTALPROJECT_API AGEII_PortalLobbyGameMode : public AGEII_PortalProjectGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

private:
	void StartGame();
	
	uint32 NumberOfPlayers = 0;
	//GameState->PlayerArray.Num()

	FTimerHandle GameStartTimer;
};
