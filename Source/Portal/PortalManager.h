// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalManager.generated.h"

class APortal;
class AGEII_PortalPlayerController;

UCLASS()
class GEII_PORTALPROJECT_API APortalManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortalManager();

	APortal* SpawnPortal(APortal* Target, const FHitResult& HitComponent);
	void SpawnRedPortal(const FHitResult& Hit);
	void SpawnBluePortal(const FHitResult& Hit);

	void SetControllerOwner(AGEII_PortalPlayerController* NewOwner);

	void SetPortalClass(TSubclassOf<class APortal> Class);

	void UpdatePortalView();
	
	bool CheckCanSpawnInWall();
	
	bool VerifyPortalPlacement(const APortal* Portal, FVector& Origin);

	void FitPortalAroundTargetPortal(const APortal* Portal, const APortal* Target, const FVector& Forward, const FVector& Right, const FVector& Up, FVector& Origin);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//Avoids creating one scene component per portal, thus recycling it each time we switch to a specific portal actor in the level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capture)
	USceneCaptureComponent2D* CaptureComponent;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	
	//Blue portal
	APortal* BluePortal;

	//Orange portal
	APortal* RedPortal;

	//Owning player controller
	UPROPERTY()
	AGEII_PortalPlayerController* OwningController;
	
	//Portal class to spawn 
	UPROPERTY(EditAnywhere, Category = Portal, meta = (AllowPrivateAccess = true))
	TSubclassOf<class APortal> PortalClass;

	//Can spawn portal?
	bool bCanSpawnPortal;
};
