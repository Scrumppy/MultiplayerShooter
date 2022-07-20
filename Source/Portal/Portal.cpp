// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal.h"

#include "GEII_PortalProjectCharacter.h"
#include <Kismet/GameplayStatics.h>

#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "GEII_PortalPlayerController.h"
#include "PortalFunctionLibrary.h"
#include "PortalManager.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
APortal::APortal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(RootComponent);

	//Portal Border Mesh
	PortalBorderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalBorderMesh"));
	PortalBorderMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PortalBorderMesh->SetupAttachment(RootComponent);

	//Portal Mesh
	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	PortalBorderMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PortalMesh->SetupAttachment(RootComponent);

	//Box collision
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(FName("BoxComponent"));
	BoxComponent->SetBoxExtent(FVector(25, 80, 75));
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnOverlapBegin);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &APortal::OnOverlapEnd);
	BoxComponent->SetupAttachment(RootComponent);
	
	//Materials
	Material_A = CreateDefaultSubobject<UMaterialInterface>("Material_A");
	Material_B = CreateDefaultSubobject<UMaterialInterface>("Material_B");

	//Points
	TopPoint = CreateDefaultSubobject<USceneComponent>(FName("TopPoint"));
	TopPoint->SetupAttachment(RootComponent);
	RightPoint = CreateDefaultSubobject<USceneComponent>(FName("RightPoint"));
	RightPoint->SetupAttachment(RootComponent);
	BottomPoint = CreateDefaultSubobject<USceneComponent>(FName("BottomPoint"));
	BottomPoint->SetupAttachment(RootComponent);
	LeftPoint = CreateDefaultSubobject<USceneComponent>(FName("LeftPoint"));
	LeftPoint->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APortal::BeginPlay()
{
	Super::BeginPlay();
	
	//Change portal border or ring material
	ChangeBorderMeshMaterial();

	//CreateRender Target and Settings
	RenderTarget = NewObject<UTextureRenderTarget2D>(this, UTextureRenderTarget2D::StaticClass(), FName("RenderTarget"));
	RenderTarget->InitAutoFormat(1080, 720);
	RenderTarget->AddressX = TextureAddress::TA_Wrap;
	RenderTarget->AddressY = TextureAddress::TA_Wrap;
	RenderTarget->bAutoGenerateMips = false;
	RenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;
	RenderTarget->Filter = TextureFilter::TF_Bilinear;

	//Create material instance to take in the render target
	MaterialInstance = UMaterialInstanceDynamic::Create(Material, this);
	MaterialInstance->SetTextureParameterValue(FName("RenderTargetTexture"), RenderTarget);
	PortalMesh->SetMaterial(0, MaterialInstance);

	// //Check portal spawn
	// if(CheckPortalPlacement() == true)
	// {
	// 	bIsAbleToSpawn = true;
	// 	UE_LOG(LogTemp, Error, TEXT("Can Spawn Portal"));
	// }
	// else
	// {
	// 	bIsAbleToSpawn = false;
	// 	UE_LOG(LogTemp, Error, TEXT("Cant Spawn Portal"));
	// }
}

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//If player is overlapping, teleport
	if(bOverlapping)
	{
		TeleportPlayer();
	}
}

void APortal::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//Get portal character
	AGEII_PortalProjectCharacter* MyCharacter = Cast<AGEII_PortalProjectCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(),0));

	//If overlapping actor is portal character
	if (OtherActor == MyCharacter)
	{
		//Portal character is overlapping
		bOverlapping = true;
		if (PortalSurface != nullptr)
		{
			//Disable collision of the portal surface
			PortalSurface->SetActorEnableCollision(false);
		}
	}
}

void APortal::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	//Get portal character
	AGEII_PortalProjectCharacter* MyCharacter = Cast<AGEII_PortalProjectCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(),0));

	//If overlapping actor is portal character
	if (OtherActor == MyCharacter)
	{
		//Portal character is not overlapping
		bOverlapping = false;
		if (PortalSurface != nullptr)
		{
			//Enable collision of the portal surface
			PortalSurface->SetActorEnableCollision(true);
		}
	}
}

void APortal::TeleportPlayer()
{
	//If there is a target
	if (Target != nullptr)
	{
		//Get player controller  
		AGEII_PortalPlayerController* PlayerController = Cast<AGEII_PortalPlayerController>(GetWorld()->GetFirstPlayerController());

		//If there is a player controller
		if(PlayerController != nullptr)
		{
			//Get portal character
			AGEII_PortalProjectCharacter* MyCharacter = Cast<AGEII_PortalProjectCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(),0));

			//If there is a portal character
			if(MyCharacter != nullptr)
			{
				//Check if player is crossing the portals
				if(UPortalFunctionLibrary::CheckIsCrossing(MyCharacter->GetActorLocation(), GetActorLocation(), GetActorForwardVector(), bLastInFront, LastPosition))
				{
					UE_LOG(LogTemp, Warning, TEXT("Teleported to %s"), *Target->GetName());
					//Save player velocity
					FVector SavedVelocity = MyCharacter->GetCharacterMovement()->Velocity;

					FHitResult HitResult;
					//Get location
					FVector Location = UPortalFunctionLibrary::ConvertLocation(MyCharacter->GetActorLocation(), this, Target);
					//Get rotation
					FRotator Rotation = UPortalFunctionLibrary::ConvertRotation(MyCharacter->GetActorRotation(), this, Target);
					//Set new location and rotation for the player
					MyCharacter->SetActorLocationAndRotation(Location, Rotation, false, &HitResult, ETeleportType::TeleportPhysics);
					//Set player control rotation
					PlayerController->SetControlRotation(UPortalFunctionLibrary::ConvertRotation(PlayerController->GetControlRotation(), this, Target));

					//Get the velocity and direction of the player, and multiply it by the respective vector of the portal
					FVector NewVelocity = 
						FVector::DotProduct(SavedVelocity, GetActorForwardVector()) * Target->GetActorForwardVector() +
						FVector::DotProduct(SavedVelocity, GetActorRightVector()) * Target->GetActorRightVector() +
						FVector::DotProduct(SavedVelocity, GetActorUpVector()) * Target->GetActorUpVector();

					//Apply new velocity, NewVelocity is negative so we face the other way
					MyCharacter->GetCharacterMovement()->Velocity = -NewVelocity;

					//Last position is the last location
					LastPosition = Location;
					//After teleportation, player is no longer overlapping
					bOverlapping = false;
				}
			}
		}
	}
}

void APortal::SetTarget(APortal* NewTarget)
{
	//Set target of the portal
	if(NewTarget != nullptr)
	{
		Target = NewTarget;
	}
}

void APortal::SetPortalSurface(AActor* Surface)
{
	//Set portal surface
	PortalSurface = Surface;
}

void APortal::ChangeBorderMeshMaterial()
{
	//Get portal character
	AGEII_PortalProjectCharacter* MyCharacter = Cast<AGEII_PortalProjectCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(),0));

	//Get is portal A
	bIsPortalA = MyCharacter->GetPortalBool();
	
	if(bIsPortalA)
	{
		PortalBorderMesh->SetMaterial(0,Material_A);
	}
	else
	{
		PortalBorderMesh->SetMaterial(0,Material_B);
	}
}

bool APortal::CheckPortalCanSpawn()
{
	//Create array of points
	TArray<USceneComponent*> PointArray;
	PointArray.Add(RightPoint);
	PointArray.Add(LeftPoint);
	PointArray.Add(TopPoint);
	PointArray.Add(BottomPoint);
	
	for (USceneComponent* Points : PointArray)
	{
		FHitResult HitResult;
		FVector Start = Points->GetComponentLocation();
		FVector End = (Points->GetForwardVector() * 50.f) + Start;
		
		if(GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility))
		{
			//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f);
			if(HitResult.GetComponent() != nullptr)
			{
				//Create array of failed casts, and add those failed casts to the array
				SuccessPointsArray.Add(Points);
				//UE_LOG(LogTemp, Error, TEXT("Sucess Point Array has: %d elements"), SuccessPointsArray.Num());
			}
		}
	}
	
	if(SuccessPointsArray.Num() == 4)
	{
		return true;
	}
	
	return  false;
}

AActor* APortal::GetPortalSurface() const
{
	return PortalSurface;
}

APortal* APortal::GetTarget() const
{
	return Target;
}

FVector APortal::GetBoxComponentLocation()
{
	return BoxComponent->GetComponentLocation();
	
}

FVector APortal::GetBoxComponentForwardVector()
{
	return BoxComponent->GetForwardVector();
}

UTextureRenderTarget2D* APortal::GetRenderTarget()
{
	return RenderTarget;
}


