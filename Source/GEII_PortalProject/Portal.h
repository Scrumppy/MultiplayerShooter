// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

UCLASS()
class GEII_PORTALPROJECT_API APortal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortal();
	
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void TeleportPlayer();

	void SetTarget(APortal* NewTarget);

	void SetPortalSurface(AActor* Surface);

	void ChangeBorderMeshMaterial();

	bool CheckPortalCanSpawn();
	
	AActor* GetPortalSurface() const;

	APortal* GetTarget() const;

	FVector GetBoxComponentLocation();

	FVector GetBoxComponentForwardVector();

	UTextureRenderTarget2D* GetRenderTarget();

	TArray<USceneComponent*> SuccessPointsArray;

	bool bIsAbleToSpawn;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Portal Border Mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
	UStaticMeshComponent* PortalBorderMesh;

	//Portal Mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
	UStaticMeshComponent* PortalMesh;

	//Portal Collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
	UBoxComponent* BoxComponent;
	
	//Materials
	UPROPERTY(EditAnywhere, Category = Portal)
	UMaterialInterface* Material_A;
	UPROPERTY(EditAnywhere, Category = Portal)
	UMaterialInterface* Material_B;

	//Render target and other capture related components
	UPROPERTY(EditAnywhere, Category = Capture)
	UTextureRenderTarget2D* RenderTarget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capture)
	UMaterialInterface* Material;
	UPROPERTY(EditAnywhere, Category = Capture)
	UMaterialInstanceDynamic* MaterialInstance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capture)
	APortal* Target;
	//Points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
	USceneComponent* LeftPoint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
	USceneComponent* RightPoint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
	USceneComponent* BottomPoint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
	USceneComponent* TopPoint;

	
private:
	
	bool bOverlapping;

	bool bIsPortalA;

	bool bLastInFront;
	
	FVector LastPosition;

	TArray<AActor*> OverlappingActors;
	
	AActor* PortalSurface;

	APortal* LinkedPortal;
	
};
