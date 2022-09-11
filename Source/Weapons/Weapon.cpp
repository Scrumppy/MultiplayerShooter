// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Engine/SkeletalMeshSocket.h"
#include "GEII_PortalProject/GEII_PortalPlayerController.h"
#include "GEII_PortalProject/GEII_PortalProjectCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	//Set Weapon to Replicate
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	MuzzleLocation->SetupAttachment(WeaponMesh);

	MuzzleParticle = CreateDefaultSubobject<UParticleSystem>(TEXT("MuzzleParticle"));

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetOnlyOwnerSee(false);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	//Enable collision on server
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnOverlapBegin);
	}
	
	MuzzleSocket = WeaponMesh->GetSocketByName(FName("Muzzle"));
	MuzzleLocation->SetWorldLocation(MuzzleSocket->GetSocketLocation(WeaponMesh));

	bPickedUp = false;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                             int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AGEII_PortalProjectCharacter* PortalCharacter = Cast<AGEII_PortalProjectCharacter>(OtherActor);
	
	if (PortalCharacter && PortalCharacter->CombatComponent->WeaponArray.Num() < 2)
	{
		//Set current overlapping weapon
		PortalCharacter->SetOverlappingWeapon(this);
		PortalCharacter->EquipOverlappingWeapon();

		if (PickupSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
		}
		bPickedUp = true;
	}
}

void AWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	 AGEII_PortalProjectCharacter* PortalCharacter = Cast<AGEII_PortalProjectCharacter>(OtherActor);
	
	if (PortalCharacter)
	{
		PortalCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	AGEII_PortalProjectCharacter* PortalCharacter = Cast<AGEII_PortalProjectCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(),0));;
	
	WeaponState = State;
	
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
		
	case EWeaponState::EWS_Stored:
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		WeaponMesh->SetSimulatePhysics(false);	
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
		
	case EWeaponState::EWS_Stored:
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;

	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
}

void AWeapon::OnRep_Ammo()
{
	PortalOwnerCharacter = PortalOwnerCharacter == nullptr ? Cast<AGEII_PortalProjectCharacter>(GetOwner()) : PortalOwnerCharacter;
	if (PortalOwnerCharacter)
	{
		PortalOwnerController = PortalOwnerController == nullptr ? Cast<AGEII_PortalPlayerController>(PortalOwnerCharacter->Controller) : PortalOwnerController;
		if (PortalOwnerController)
		{
			PortalOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	
	PortalOwnerCharacter = PortalOwnerCharacter == nullptr ? Cast<AGEII_PortalProjectCharacter>(GetOwner()) : PortalOwnerCharacter;
	if (PortalOwnerCharacter)
	{
		PortalOwnerController = PortalOwnerController == nullptr ? Cast<AGEII_PortalPlayerController>(PortalOwnerCharacter->Controller) : PortalOwnerController;
		if (PortalOwnerController)
		{
			PortalOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::SetHUDAmmo()
{
	PortalOwnerCharacter = PortalOwnerCharacter == nullptr ? Cast<AGEII_PortalProjectCharacter>(GetOwner()) : PortalOwnerCharacter;
	if (PortalOwnerCharacter)
	{
		PortalOwnerController = PortalOwnerController == nullptr ? Cast<AGEII_PortalPlayerController>(PortalOwnerCharacter->Controller) : PortalOwnerController;
		if (PortalOwnerController)
		{
			PortalOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::SpendRound()
{
	if (Ammo <= 0)
	{
		Ammo = 0;
	}
	else
	{
		--Ammo;
	}

	PortalOwnerCharacter = PortalOwnerCharacter == nullptr ? Cast<AGEII_PortalProjectCharacter>(GetOwner()) : PortalOwnerCharacter;
	if (PortalOwnerCharacter)
	{
		PortalOwnerController = PortalOwnerController == nullptr ? Cast<AGEII_PortalPlayerController>(PortalOwnerCharacter->Controller) : PortalOwnerController;
		if (PortalOwnerController)
		{
			PortalOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (MuzzleParticle)
	{
		FVector SpawnLocation = MuzzleLocation->GetComponentLocation();
		FRotator SpawnRotation = MuzzleLocation->GetComponentRotation();
		FName MuzzleFlash = WeaponMesh->GetSocketBoneName(FName("MuzzleFlash"));

		UGameplayStatics* SpawnEmitter;
		//SpawnEmitter->SpawnEmitterAtLocation(GetWorld(), MuzzleParticle, SpawnLocation, SpawnRotation, true);
		SpawnEmitter->SpawnEmitterAttached(MuzzleParticle, MuzzleLocation, MuzzleFlash, SpawnLocation, SpawnRotation, EAttachLocation::KeepWorldPosition, true);
	}

	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}
	SpendRound();
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MaxAmmo);
	SetHUDAmmo();
}

void AWeapon::PlayReloadSound()
{
	if (ReloadSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
	}
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}



