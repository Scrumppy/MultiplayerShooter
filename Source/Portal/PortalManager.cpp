// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalManager.h"

#include "EngineUtils.h"
#include "GEII_PortalPlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "PortalFunctionLibrary.h"
#include "Portal.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
APortalManager::APortalManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(FName("CaptureComponent"));
	CaptureComponent->SetupAttachment(RootComponent);

	//Post Process Settings, for optimization, disabling motion blur, etc
	FPostProcessSettings CaptureSettings;
	CaptureSettings.bOverride_AmbientOcclusionQuality = true;
	CaptureSettings.bOverride_MotionBlurAmount = true;
	CaptureSettings.bOverride_SceneFringeIntensity = true;
	CaptureSettings.bOverride_GrainIntensity = true;
	CaptureSettings.bOverride_ScreenSpaceReflectionQuality = true;
	CaptureSettings.bOverride_ScreenPercentage = true;
	CaptureSettings.AmbientOcclusionQuality = 0.0f;
	CaptureSettings.MotionBlurAmount = 0.0f;
	CaptureSettings.SceneFringeIntensity = 0.0f;
	CaptureSettings.GrainIntensity = 0.0f;
	CaptureSettings.ScreenSpaceReflectionQuality = 0.0f;
	CaptureSettings.ScreenPercentage = 100.0f;

	//Capture Component Settings
	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->bCaptureOnMovement = false;
	CaptureComponent->LODDistanceFactor = 3;
	CaptureComponent->TextureTarget = nullptr;
	CaptureComponent->bEnableClipPlane = true;
	CaptureComponent->bUseCustomProjectionMatrix = true;
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneColorSceneDepth;
	CaptureComponent->PostProcessSettings = CaptureSettings;
}

APortal* APortalManager::SpawnPortal(APortal* Target, const FHitResult& HitComponent)
{
	//Get portal
	APortal* Portal = nullptr;

	//Get world
	UWorld* const World = GetWorld();
	
	if (World != nullptr && PortalClass != nullptr)
	{
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		//Portal origin
		FVector Origin = HitComponent.Location;
		//Portal rotation
		FRotator Rotation = HitComponent.ImpactNormal.Rotation();

		//Spawn portal
		Portal = World->SpawnActor<APortal>(PortalClass, Origin, Rotation, ActorSpawnParams);
		//Set portal surface on spawn
		Portal->SetPortalSurface(HitComponent.GetActor());
		UE_LOG(LogTemp, Display, TEXT("Spawned Portal"));
	
		//Set portal targets
		if (Target != nullptr)
		{
			Portal->SetTarget(Target);
			Target->SetTarget(Portal);
		}

		//Check if portal can be spawned
		if(!Portal->CheckPortalCanSpawn())
		{
			UE_LOG(LogTemp, Error, TEXT("Portal doesnt fit on surface"));
			Target->SetTarget(nullptr);
			Portal->Destroy();
		}
		if (VerifyPortalPlacement(Portal, Origin))
		{
			Portal->SetActorLocation(Origin);
		}
		else
		{
			Target->SetTarget(nullptr);
			Portal->Destroy();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to spawn portal"));
	}
	return Portal;
}

void APortalManager::SpawnRedPortal(const FHitResult& Hit)
{
	//Check if red portal is already placed
	if (RedPortal != nullptr)
	{
		BluePortal->SetTarget(nullptr);
		RedPortal->Destroy();
	}

	RedPortal = SpawnPortal(BluePortal, Hit);
}

void APortalManager::SpawnBluePortal(const FHitResult& Hit)
{
	//Check if blue portal is already placed
	if (BluePortal != nullptr)
	{
		RedPortal->SetTarget(nullptr);
		BluePortal->Destroy();
	}

	BluePortal = SpawnPortal(RedPortal, Hit);
}

void APortalManager::SetControllerOwner(AGEII_PortalPlayerController* NewOwner)
{
	OwningController = NewOwner;
}

void APortalManager::SetPortalClass(TSubclassOf<APortal> Class)
{
	PortalClass = Class;
}

void APortalManager::UpdatePortalView()
{
	if (OwningController != nullptr)
	{
		//Get player camera manager
		APlayerCameraManager* CameraManager = OwningController->PlayerCameraManager;
		
		if (CameraManager != nullptr)
		{
			//For each portal
			for (TActorIterator<APortal> It(GetWorld()); It; ++It)
			{
				APortal* Portal = *It;
				//Get target of each portal
				APortal* Target = Portal->GetTarget();
				
				if (Portal != nullptr && Target != nullptr)
				{
					//Update the portal view position
					FVector Position = UPortalFunctionLibrary::ConvertLocation(CameraManager->GetCameraLocation(), Portal, Target);
					//Update the portal view rotation
					FRotator Rotation = UPortalFunctionLibrary::ConvertRotation(CameraManager->GetCameraRotation(), Portal, Target);
					//Set position and rotation of the camera to respective vectors
					CaptureComponent->SetWorldLocationAndRotation(Position, Rotation);
					
					//Clip the scene capture plane
					CaptureComponent->ClipPlaneNormal = Target->GetActorForwardVector();
					CaptureComponent->ClipPlaneBase = Target->GetActorLocation() + (CaptureComponent->ClipPlaneNormal * -1.5f);
					CaptureComponent->CustomProjectionMatrix = OwningController->GetCameraProjectionMatrix();
					CaptureComponent->TextureTarget = Portal->GetRenderTarget();
					CaptureComponent->CaptureScene();
				}
			}
		}
	}
}

bool APortalManager::VerifyPortalPlacement(const APortal* Portal, FVector& Origin)
{
	//Get origin of the portal
	FVector OriginalOrigin = Origin;

	//Get vectors of the portal
	FVector Forward = Portal->GetActorForwardVector();
	FVector Right = Portal->GetActorRightVector();
	FVector Up = Portal->GetActorUpVector();

	//Check if portal is overlapping linked portal
	const APortal* Target = Portal->GetTarget();
	if (Target != nullptr)
	{
		FitPortalAroundTargetPortal(Portal, Target, Forward, Right, Up, Origin);
	}

	return true;
}

void APortalManager::FitPortalAroundTargetPortal(const APortal* Portal, const APortal* Target, const FVector& Forward, const FVector& Right, const FVector& Up, FVector& Origin)
{
	//Get Target Forward Vector
	FVector TargetForward = Target->GetActorForwardVector();

	//Reposition if portals are on the same face
	if (FVector::DotProduct(Forward, TargetForward) > 1.f - KINDA_SMALL_NUMBER)
	{
		//Get distance of the portal and its projections
		FVector Distance = Origin - Target->GetActorLocation();
		FVector RightProjection = FVector::DotProduct(Distance, Right) * Right;
		FVector UpProjection = FVector::DotProduct(Distance, Up) * Up;

		//Get length of the projections
		float RightProjectionLength = RightProjection.Size();
		float UpProjectionLength = UpProjection.Size();

		//If the length of the right projection vector is less than 1, equal to right vector
		if (RightProjectionLength < 1.0f)
		{
			RightProjection = Right;
		}

		//Caculate size of the portal, using bounding box
		FVector Size = Portal->CalculateComponentsBoundingBoxInLocalSpace().GetSize();
		
			//Normalize the projection
			RightProjection.Normalize();
			//Apply the projection vector to the origin
			Origin += RightProjection * (Size.Y - RightProjectionLength + 1.0f);
		
	}
}

// Called when the game starts or when spawned
void APortalManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APortalManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdatePortalView();

}

