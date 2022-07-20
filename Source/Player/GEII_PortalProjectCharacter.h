// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GEII_PortalPlayerState.h"
#include "Components/CombatComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Containers/Array.h"
#include "Weapons/Weapon.h"
#include "GEII_PortalProjectCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UAnimMontage;
class USoundBase;

UCLASS(config=Game)
class AGEII_PortalProjectCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta=(AllowPrivateAccess = true), Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** Pawn mesh: 3st person view */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta=(AllowPrivateAccess = true), Category = Mesh)
	USkeletalMeshComponent* Mesh3P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = true), Category = Mesh)
	USkeletalMeshComponent* FP_Gun;

	/** Gun mesh: 3st person view */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = true), Category = Mesh)
	USkeletalMeshComponent* TP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USceneComponent* FP_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	// USpringArmComponent* CameraBoom;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess = true), Category = Mesh)
	UMaterialInterface* PlayerMaterial;

public:
	AGEII_PortalProjectCharacter();

	virtual void Tick(float DeltaTime) override;

	void SetPlayerClass(TSubclassOf<class AGEII_PortalProjectCharacter> Class);

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AGEII_PortalProjectProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint8 bUsingMotionControllers : 1;

	// Is portal A?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	bool bIsPortalA;

	// Is looking at a wall?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	bool bCanShootWall;

	//Player Colours Array
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Player Colours")
	TArray<FLinearColor> PlayerColours;

	UPROPERTY(VisibleAnywhere)
	UCombatComponent* CombatComponent;

	UPROPERTY(Replicated)
	AWeapon* OverlappingWeapon;

	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns Mesh3P subobject **/
	USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UPROPERTY()
	AGEII_PortalPlayerState* PortalPlayerState;

	//Return bIsPortal boolean
	bool GetPortalBool();

	UCameraComponent* GetCamera();

	virtual void PostInitializeComponents() override;

	void EquipOverlappingWeapon();

	void Elimination();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElimination();

	void RagdollPlayer();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRagdollPlayer();

	// Replicates

	//Replicate variables
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//Equip Weapon RPC
	UFUNCTION(Server, Reliable)
	void ServerEquipOverlappingWeapon();

	void SetOverlappingWeapon(AWeapon* Weapon);
	void PlayFireMontage();
	bool IsWeaponEquipped();
	bool SetHasPortalGun();
	bool IsAiming();
	ECombatState GetCombatState() const;
	FORCEINLINE bool IsEliminated() const { return bEliminated; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; }

	//Change Player Colour

	UFUNCTION(BlueprintCallable)
	void ChangePlayerColour(int32 Index);

	UFUNCTION(Server, Reliable)
	void ServerChangePlayerColour(int32 Index);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastChangePlayerColour(int32 Index);

	UPROPERTY(VisibleAnywhere)
	int32 WeaponCounter = 0;
protected:
	
	virtual void BeginPlay();

	UPROPERTY(EditAnywhere, Category = Player, meta = (AllowPrivateAccess = true))
	TSubclassOf<class AGEII_PortalProjectCharacter> PlayerClass;

	/** Fires a projectile. */
	void OnFirePortalA();
	void OnFirePortalB();
	
	AGEII_PortalProjectProjectile* OnFire();
	
	void OnResetVR();
	void MoveForward(float Val);
	void MoveRight(float Val);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void AimButtonPressed();
	void AimButtonReleased();
	void PrimaryFireButtonPressed();
	void PrimaryFireButtonReleased();
	void ReloadButtonPressed();
	void NextWeapon();
	void LastWeapon();
	
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	
	void UpdateHUDHealth();

	// Poll for any relevant classes and initialize the hud
	void PollInit();

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;
	
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);
	
private:

	class AGEII_PortalPlayerController* PortalPlayerController;

	bool bHasPortalGun = false;

	bool bEliminated = false;

	bool bIsRagdoll = false;

	FTimerHandle EliminationTimer;

	UPROPERTY(EditDefaultsOnly)
	float EliminationDelay = 2.f;

	void EliminationTimerFinished();

	//Player Health

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();
};