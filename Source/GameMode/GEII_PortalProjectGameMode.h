// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameModeBase.h"
#include "GEII_PortalProjectGameMode.generated.h"

UCLASS(minimalapi)
class AGEII_PortalProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGEII_PortalProjectGameMode();

	virtual void PlayerEliminated(class AGEII_PortalProjectCharacter* EliminatedCharacter, class AGEII_PortalPlayerController* VictimController, AGEII_PortalPlayerController* AttackerController);

	virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;


private:
	UPROPERTY()
	class AGEII_PortalProjectCharacter* PortalCharacter;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	TSubclassOf<AActor> ActorClass;

	int32 PlayerCount;

	bool bCanExecute = true;

	float Delay = 2;

	void TimerFinished();
};



