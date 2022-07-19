// Copyright Epic Games, Inc. All Rights Reserved.

#include "GEII_PortalProjectCharacter.h"
#include "Runtime/Engine/Public/EngineGlobals.h"
#include "GEII_PortalProjectProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "Portal.h"
#include "GEII_PortalProjectHUD.h"
#include "DrawDebugHelpers.h"
#include "GEII_PortalPlayerController.h"
#include "GEII_PortalProject.h"
#include "GEII_PortalProjectGameMode.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AGEII_PortalProjectCharacter

AGEII_PortalProjectCharacter::AGEII_PortalProjectCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	//Ignore Projectile
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh3P"));
	Mesh3P->SetOnlyOwnerSee(true);
	Mesh3P->SetupAttachment(FirstPersonCameraComponent);
	//Mesh3P->SetCollisionObjectType(ECC_SkeletalMesh);
	

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(Mesh3P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	TP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TP_Gun"));
	TP_Gun->SetupAttachment(Mesh3P);



	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	bCanShootWall = false;

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);
}

void AGEII_PortalProjectCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AGEII_PortalProjectCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AGEII_PortalProjectCharacter, Health);
	DOREPLIFETIME(AGEII_PortalProjectCharacter, PlayerColours);
}

void AGEII_PortalProjectCharacter::PollInit()
{
	if (PortalPlayerState == nullptr)
	{
		PortalPlayerState = GetPlayerState<AGEII_PortalPlayerState>();
		if (PortalPlayerState)
		{
			PortalPlayerState->AddToScore(0.f);
			PortalPlayerState->AddToDeaths(0);
		}
	}
}

void AGEII_PortalProjectCharacter::RagdollPlayer()
{
	MulticastRagdollPlayer_Implementation();
}

void AGEII_PortalProjectCharacter::MulticastRagdollPlayer_Implementation()
{
	GetMesh3P()->SetSimulatePhysics(true);
	GetMesh1P()->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
	GetMesh3P()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
	GetMesh3P()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh1P()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	
}

void AGEII_PortalProjectCharacter::PlayFireMontage()
{
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

// ------------------- Weapons

void AGEII_PortalProjectCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	OverlappingWeapon = Weapon;
}

bool AGEII_PortalProjectCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}

bool AGEII_PortalProjectCharacter::SetHasPortalGun()
{
	return bHasPortalGun = true;
}

bool AGEII_PortalProjectCharacter::IsAiming()
{
	return (CombatComponent && CombatComponent->bAiming);
}

void AGEII_PortalProjectCharacter::AimButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(true);
	}
}

void AGEII_PortalProjectCharacter::AimButtonReleased()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(false);
	}
}

void AGEII_PortalProjectCharacter::PrimaryFireButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(true);
	}
}

void AGEII_PortalProjectCharacter::PrimaryFireButtonReleased()
{
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(false);
	}
}

void AGEII_PortalProjectCharacter::ReloadButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->Reload();
	}
}

// -------------------

// ------------------- Health

void AGEII_PortalProjectCharacter::OnRep_Health()
{
	UpdateHUDHealth();
}

void AGEII_PortalProjectCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();

	if (Health == 0.f)
	{
		AGEII_PortalProjectGameMode* PortalGameMode =  GetWorld()->GetAuthGameMode<AGEII_PortalProjectGameMode>();
		if (PortalGameMode)
		{
			PortalPlayerController = PortalPlayerController == nullptr ? Cast<AGEII_PortalPlayerController>(Controller) : PortalPlayerController;
			AGEII_PortalPlayerController* AttackerController = Cast<AGEII_PortalPlayerController>(InstigatorController);
			PortalGameMode->PlayerEliminated(this, PortalPlayerController, AttackerController);
		}
	}
}

void AGEII_PortalProjectCharacter::UpdateHUDHealth()
{
	//If null, cast controller to portalplayercontroller and set it to portalplayercontroller, but if not null, set portalplayercontroller equal to itself
	PortalPlayerController = PortalPlayerController	== nullptr ?  Cast<AGEII_PortalPlayerController>(Controller) : PortalPlayerController;
	if (PortalPlayerController)
	{
		PortalPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AGEII_PortalProjectCharacter::Elimination()
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->EquippedWeapon->Dropped();
	}
	MulticastElimination();
	GetWorldTimerManager().SetTimer(EliminationTimer, this, &AGEII_PortalProjectCharacter::EliminationTimerFinished, EliminationDelay);
}

void AGEII_PortalProjectCharacter::MulticastElimination_Implementation()
{
	if (PortalPlayerController)
	{
		PortalPlayerController->SetHUDWeaponAmmo(0);
	}
	bEliminated = true;

	//Disable character moment
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	
	RagdollPlayer();

	//Disable input
	if (PortalPlayerController)
	{
		DisableInput(PortalPlayerController);
	}

	// //Disable Collision
	// GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGEII_PortalProjectCharacter::EliminationTimerFinished()
{
	AGEII_PortalProjectGameMode* PortalGameMode =  GetWorld()->GetAuthGameMode<AGEII_PortalProjectGameMode>();
	if (PortalGameMode)
	{
		PortalGameMode->RequestRespawn(this, Controller);
		if (CombatComponent)
		{
			CombatComponent->ResetCarriedAmmo();
		}
	}
}

// -------------------
void AGEII_PortalProjectCharacter::SetPlayerClass(TSubclassOf<AGEII_PortalProjectCharacter> Class)
{
	Class = PlayerClass;
}

void AGEII_PortalProjectCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	UpdateHUDHealth();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AGEII_PortalProjectCharacter::ReceiveDamage);
	}
}

void AGEII_PortalProjectCharacter::Tick(float DeltaTime) 
{
	Super::Tick(DeltaTime);

	PollInit();
	// FHitResult Hit;
	// FVector Start = FirstPersonCameraComponent->GetComponentLocation();
	// FVector FowardVector = FirstPersonCameraComponent->GetForwardVector();	
	// FVector End = Start + (FowardVector * 10000);
	// FCollisionObjectQueryParams TraceParams;
	// bCanShootWall = false;
	//
	// AGEII_PortalProjectHUD* myHud = Cast<AGEII_PortalProjectHUD>(UGameplayStatics::GetPlayerController(this, 0)->GetHUD());
	// myHud->bShowHUD = false;
	// if (GetWorld() != nullptr)
	// {
	// 	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility);
	// 	if (Hit.GetComponent() != nullptr)
	// 	{
	// 		if (Hit.Component->GetCollisionObjectType() == ECC_GameTraceChannel2)
	// 		{
	// 			myHud->bShowHUD = true;
	// 			bCanShootWall = true;
	// 		}
	//
	// 	}
	// }
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGEII_PortalProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// // Bind fire event
	// PlayerInputComponent->BindAction("FirePortalA", IE_Pressed, this, &AGEII_PortalProjectCharacter::OnFirePortalA);
	// PlayerInputComponent->BindAction("FirePortalB", IE_Pressed, this, &AGEII_PortalProjectCharacter::OnFirePortalB);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AGEII_PortalProjectCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AGEII_PortalProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGEII_PortalProjectCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGEII_PortalProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGEII_PortalProjectCharacter::LookUpAtRate);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AGEII_PortalProjectCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AGEII_PortalProjectCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("PrimaryFire", IE_Pressed, this, &AGEII_PortalProjectCharacter::PrimaryFireButtonPressed);
	PlayerInputComponent->BindAction("PrimaryFire", IE_Released, this, &AGEII_PortalProjectCharacter::PrimaryFireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AGEII_PortalProjectCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &AGEII_PortalProjectCharacter::NextWeapon);
	PlayerInputComponent->BindAction("LastWeapon", IE_Pressed, this, &AGEII_PortalProjectCharacter::LastWeapon);
}

void AGEII_PortalProjectCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent)
	{
		CombatComponent->PortalCharacter = this;
	}
}

void AGEII_PortalProjectCharacter::EquipOverlappingWeapon()
{
	if (CombatComponent)
	{
		if (OverlappingWeapon)
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipOverlappingWeapon();
		}
	}
}

void AGEII_PortalProjectCharacter::ServerEquipOverlappingWeapon_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);

	}
}

void AGEII_PortalProjectCharacter::NextWeapon()
{
	if (CombatComponent && CombatComponent->WeaponArray.Num() == 0)
	{
		return;
	}
	else if (CombatComponent->WeaponArray.Num() > 1)
	{
		CombatComponent->WeaponArray[WeaponCounter]->SetActorHiddenInGame(true);
		CombatComponent->WeaponArray[WeaponCounter]->SetWeaponState(EWeaponState::EWS_Stored);
		WeaponCounter += 1;
		if (WeaponCounter > 1)
		{
			WeaponCounter = 0;;
		}

		CombatComponent->SwitchWeapons(CombatComponent->WeaponArray[WeaponCounter]);
	}
}

void AGEII_PortalProjectCharacter::LastWeapon()
{
	if (CombatComponent && CombatComponent->WeaponArray.Num() == 0)
	{
		return;
	}
	else if (CombatComponent->WeaponArray.Num() > 1)
	{
		CombatComponent->WeaponArray[WeaponCounter]->SetActorHiddenInGame(true);
		CombatComponent->WeaponArray[WeaponCounter]->SetWeaponState(EWeaponState::EWS_Stored);
		WeaponCounter -= 1;
		if (WeaponCounter < 0)
		{
			WeaponCounter = 1;;
		}
		CombatComponent->SwitchWeapons(CombatComponent->WeaponArray[WeaponCounter]);
	}
}

AGEII_PortalProjectProjectile* AGEII_PortalProjectCharacter::OnFire()
{
	// //Try and fire a projectile
	// if (ProjectileClass != nullptr)
	// {
	// 	UWorld* const World = GetWorld();
	// 	if (World != nullptr)
	// 	{
	// 		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
	// 		AnimInstance->Montage_Play(FireAnimation, 1.f);
	// 			
	// 		const FRotator SpawnRotation = GetControlRotation();
	// 		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
	// 		const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);
	//
	// 		//Set Spawn Collision Handling Override
	// 		FActorSpawnParameters ActorSpawnParams;
	// 		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	// 		//Get bIsPortalA bool
	// 		GetPortalBool();
	// 		// Spawn the projectile and return the reference
	// 		return World->SpawnActor<AGEII_PortalProjectProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
	// 	}
	// 	
	// }

	// try and play the sound if specified
	if (FireSound != nullptr)
	{
		//UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
	return nullptr;
}

// //Fire Portal A
// void AGEII_PortalProjectCharacter::OnFirePortalA()
// {
// 	if (bCanShootWall)
// 	{
// 		AGEII_PortalProjectProjectile* PortalProjectile = OnFire();
//
// 		if (PortalProjectile == nullptr)
// 		{
// 			return;
// 		}
// 		PortalProjectile->ChangeProjectileMaterial(true);
// 		bIsPortalA = true;
// 	}
// }
//
// //Fire Portal B
// void AGEII_PortalProjectCharacter::OnFirePortalB()
// {
// 	if (bCanShootWall)
// 	{
// 		AGEII_PortalProjectProjectile* PortalProjectile = OnFire();
//
// 		if (PortalProjectile == nullptr)
// 		{
// 			return;
// 		}
// 		PortalProjectile->ChangeProjectileMaterial(false);
// 		bIsPortalA = false;
// 	}
// }

void AGEII_PortalProjectCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AGEII_PortalProjectCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		//OnFirePortalA();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AGEII_PortalProjectCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}


void AGEII_PortalProjectCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AGEII_PortalProjectCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AGEII_PortalProjectCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AGEII_PortalProjectCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AGEII_PortalProjectCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AGEII_PortalProjectCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AGEII_PortalProjectCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AGEII_PortalProjectCharacter::TouchUpdate);
		return true;
	}
	
	return false;
}

bool AGEII_PortalProjectCharacter::GetPortalBool()
{
	return bIsPortalA;
}

UCameraComponent* AGEII_PortalProjectCharacter::GetCamera()
{
	return FirstPersonCameraComponent;
}


ECombatState AGEII_PortalProjectCharacter::GetCombatState() const
{
	if (CombatComponent == nullptr) ECombatState::ECS_MAX;

	return CombatComponent->CombatState;
}

void AGEII_PortalProjectCharacter::ChangePlayerColour(int32 Index)
{
	ServerChangePlayerColour(Index);
}

void AGEII_PortalProjectCharacter::ServerChangePlayerColour_Implementation(int32 Index)
{
	MulticastChangePlayerColour(Index);
}

void AGEII_PortalProjectCharacter::MulticastChangePlayerColour_Implementation(int32 Index)
{
	UMaterialInstanceDynamic* DynamicPlayerMaterial = UMaterialInstanceDynamic::Create(PlayerMaterial, this);
	DynamicPlayerMaterial->SetVectorParameterValue("BodyColor", PlayerColours[Index]);
	Mesh1P->SetMaterial(0, DynamicPlayerMaterial);
	Mesh3P->SetMaterial(0, DynamicPlayerMaterial);
}
