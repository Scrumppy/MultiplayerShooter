// Copyright Epic Games, Inc. All Rights Reserved.

#include "GEII_PortalProjectProjectile.h"

#include "Portal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GEII_PortalPlayerController.h"
#include "PortalManager.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

AGEII_PortalProjectProjectile::AGEII_PortalProjectProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AGEII_PortalProjectProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	//Projectile Mesh
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);


	//Change Materials
	Material_A = CreateDefaultSubobject<UMaterialInterface>("Material_A");
	Material_B = CreateDefaultSubobject<UMaterialInterface>("Material_B");

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 8000.f;
	ProjectileMovement->MaxSpeed = 8000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void AGEII_PortalProjectProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//Get player controller
	AGEII_PortalPlayerController* PlayerController = Cast<AGEII_PortalPlayerController>(GetWorld()->GetFirstPlayerController());
	
	if(PlayerController != nullptr)
	{
		//Get portal manager
		APortalManager* PortalManager = PlayerController->GetPortalManager();
		//Get portal character
		AGEII_PortalProjectCharacter* MyCharacter = Cast<AGEII_PortalProjectCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(),0));

		//Get is portal A
		bool bIsPortalA = MyCharacter->GetPortalBool();

		//Try to spawn the portal, if collided component is a portal wall
		if(PortalManager != nullptr && OtherComp->GetCollisionObjectType() == ECC_GameTraceChannel2 && MyCharacter != nullptr && bIsPortalA) 
		{
			//Spawn Portal A (Orange Portal)
			//PortalManager->SpawnRedPortal(Hit);
			UE_LOG(LogTemp, Warning, TEXT("Spawned Orange Portal"));
		}
		else if (PortalManager != nullptr && OtherComp->GetCollisionObjectType() == ECC_GameTraceChannel2 && MyCharacter != nullptr && !bIsPortalA)
		{
			//Spawn Portal B (Blue Portal)
			//PortalManager->SpawnBluePortal(Hit);
			UE_LOG(LogTemp, Warning, TEXT("Spawned Blue Portal"));
		}
	}
	else
	{
		//No player controller detected
		UE_LOG(LogTemp, Warning, TEXT("Null player controller"));
	}
	Destroy();
}

void AGEII_PortalProjectProjectile::ChangeProjectileMaterial(bool bIsPortalA)
{
	if(bIsPortalA)
	{
		ProjectileMesh->SetMaterial(0,Material_A);
	}
	else
	{
		ProjectileMesh->SetMaterial(0,Material_B);
	}
}



