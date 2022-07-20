// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GEII_PortalPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class GEII_PORTALPROJECT_API AGEII_PortalPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Deaths();
	void AddToScore(float ScoreAmount);
	void AddToDeaths(int32 DeathsAmount);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY()
	class AGEII_PortalProjectCharacter* PortalCharacter;
	UPROPERTY()
	class AGEII_PortalPlayerController* PortalController;

	UPROPERTY(ReplicatedUsing = OnRep_Deaths)
	int32 Deaths;
	//float Score = GetScore();
};
