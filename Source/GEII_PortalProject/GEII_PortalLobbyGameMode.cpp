// Fill out your copyright notice in the Description page of Project Settings.


#include "GEII_PortalLobbyGameMode.h"

#include "GEII_PortalGameInstance.h"

void AGEII_PortalLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	++NumberOfPlayers;

	if (NumberOfPlayers >= 2)
	{
		GetWorldTimerManager().SetTimer(GameStartTimer, this, &AGEII_PortalLobbyGameMode::StartGame, 2);
		UE_LOG(LogTemp, Warning, TEXT("Game has started!"))
	}
}

void AGEII_PortalLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	--NumberOfPlayers;
}

void AGEII_PortalLobbyGameMode::StartGame()
{
	UGEII_PortalGameInstance* GameInstance = Cast<UGEII_PortalGameInstance>(GetGameInstance());
	
	if(GameInstance == nullptr) return;
	
	GameInstance->StartSession();
	
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	if (World)
	{
		bUseSeamlessTravel = true;
		World->ServerTravel("/Game/FirstPersonCPP/Maps/FirstPersonExampleMap?listen"); 
	}

}
