// Fill out your copyright notice in the Description page of Project Settings.


#include "GEII_PortalPlayerState.h"

#include "GEII_PortalPlayerController.h"
#include "GEII_PortalProjectCharacter.h"
#include "Net/UnrealNetwork.h"

void AGEII_PortalPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGEII_PortalPlayerState, Deaths);
}

void AGEII_PortalPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	PortalCharacter = PortalCharacter == nullptr ? Cast<AGEII_PortalProjectCharacter>(GetPawn()) : PortalCharacter;
	if (PortalCharacter)
	{
		PortalController = PortalController == nullptr ? Cast<AGEII_PortalPlayerController>(PortalCharacter->Controller) : PortalController;
		if (PortalController)
		{
			PortalController->SetHUDScore(GetScore());
		}
	}
}

void AGEII_PortalPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	PortalCharacter = PortalCharacter == nullptr ? Cast<AGEII_PortalProjectCharacter>(GetPawn()) : PortalCharacter;
	if (PortalCharacter)
	{
		PortalController = PortalController == nullptr ? Cast<AGEII_PortalPlayerController>(PortalCharacter->Controller) : PortalController;
		if (PortalController)
		{
			PortalController->SetHUDScore(GetScore());
		}
	}
}

void AGEII_PortalPlayerState::AddToDeaths(int32 DeathsAmount)
{
	Deaths += DeathsAmount;
	PortalCharacter = PortalCharacter == nullptr ? Cast<AGEII_PortalProjectCharacter>(GetPawn()) : PortalCharacter;
	if (PortalCharacter)
	{
		PortalController = PortalController == nullptr ? Cast<AGEII_PortalPlayerController>(PortalCharacter->Controller) : PortalController;
		if (PortalController)
		{
			PortalController->SetHUDDeaths(Deaths);
		}
	}
}

void AGEII_PortalPlayerState::OnRep_Deaths()
{
	PortalCharacter = PortalCharacter == nullptr ? Cast<AGEII_PortalProjectCharacter>(GetPawn()) : PortalCharacter;
	if (PortalCharacter)
	{
		PortalController = PortalController == nullptr ? Cast<AGEII_PortalPlayerController>(PortalCharacter->Controller) : PortalController;
		if (PortalController)
		{
			PortalController->SetHUDDeaths(Deaths);
		}
	}
}
