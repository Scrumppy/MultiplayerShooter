// Copyright Epic Games, Inc. All Rights Reserved.

#include "GEII_PortalProjectGameMode.h"

#include "GEII_PortalPlayerController.h"
#include "GEII_PortalPlayerState.h"
#include "GEII_PortalProjectHUD.h"
#include "GEII_PortalProjectCharacter.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AGEII_PortalProjectGameMode::AGEII_PortalProjectGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
	if (DefaultPawnClass != nullptr)
	{
		DefaultPawnClass = PlayerPawnClassFinder.Class;
	}
	

	// use our custom HUD class
	HUDClass = AGEII_PortalProjectHUD::StaticClass();
}

void AGEII_PortalProjectGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	++PlayerCount;
}

void AGEII_PortalProjectGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (PortalCharacter)
	{
		AController* PortalController = PortalCharacter->Controller;
		
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		//Random Spawn
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(PortalController, PlayerStarts[Selection]);	
	}
}

void AGEII_PortalProjectGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AGEII_PortalProjectGameMode::TimerFinished()
{
	bCanExecute = false;
}

void AGEII_PortalProjectGameMode::PlayerEliminated(AGEII_PortalProjectCharacter* EliminatedCharacter,
                                                   AGEII_PortalPlayerController* VictimController, AGEII_PortalPlayerController* AttackerController)
{
	//If valid, cast to attacker controller player state, else equal nullpointer
	AGEII_PortalPlayerState* AttackerPlayerState = AttackerController ? Cast<AGEII_PortalPlayerState>(AttackerController->PlayerState) : nullptr;
	AGEII_PortalPlayerState* VictimPlayerState = VictimController ? Cast<AGEII_PortalPlayerState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);	
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDeaths(1);
	}
	
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Elimination();
	}
}

void AGEII_PortalProjectGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	if (EliminatedCharacter)
	{
		//Unpossess eliminated player
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}
	
	if (EliminatedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		//Random Respawn
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(EliminatedController, PlayerStarts[Selection]);	
	}
}