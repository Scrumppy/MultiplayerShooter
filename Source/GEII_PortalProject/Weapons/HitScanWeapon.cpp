// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GEII_PortalProject/GEII_PortalProjectCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	
	AController* InstigatorController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("Muzzle");

	if (MuzzleFlashSocket && InstigatorController)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);
		if (FireHit.bBlockingHit)
		{
			AGEII_PortalProjectCharacter* PortalCharacter = Cast<AGEII_PortalProjectCharacter>(FireHit.GetActor());
			if (PortalCharacter)
			{
				if (HasAuthority())
				{
					UGameplayStatics::ApplyDamage(PortalCharacter, Damage, InstigatorController,this, UDamageType::StaticClass());
				}
			}
		}
	}
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	FVector RandomVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0, SphereRadius);
	FVector EndLocation = SphereCenter + RandomVector;
	FVector ToEndLocation = EndLocation - TraceStart;
	
	return FVector(TraceStart + ToEndLocation * 1.5f);
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = TraceEndWithScatter(TraceStart, HitTarget);
		FCollisionQueryParams CollisionParameters;
		CollisionParameters.AddIgnoredActor(Cast<AActor>(OutHit.GetActor()));

		World->LineTraceSingleByChannel(OutHit, TraceStart, End, ECollisionChannel::ECC_GameTraceChannel5, CollisionParameters);
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticles, OutHit.ImpactPoint, OutHit.ImpactNormal.Rotation());
		}
	}
}

