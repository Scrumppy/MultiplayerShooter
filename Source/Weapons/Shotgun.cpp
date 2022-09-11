// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"

#include "Engine/SkeletalMeshSocket.h"
#include "GEII_PortalProject/GEII_PortalProjectCharacter.h"
#include "Kismet/GameplayStatics.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	
	AController* InstigatorController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("Muzzle");

	if (MuzzleFlashSocket && InstigatorController)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		
		TMap<AGEII_PortalProjectCharacter*, uint32> HitMap;
		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			//FVector End = TraceEndWithScatter(Start, HitTarget);
			 FHitResult FireHit;
			 WeaponTraceHit(Start, HitTarget, FireHit);

			AGEII_PortalProjectCharacter* PortalCharacter = Cast<AGEII_PortalProjectCharacter>(FireHit.GetActor());
			if (PortalCharacter && HasAuthority() && FireHit.GetActor() != OwnerPawn)
			{
				if (HitMap.Contains(PortalCharacter))
				{
					HitMap[PortalCharacter]++;
				}
				else
				{
					HitMap.Emplace(PortalCharacter, 1);
				}
			}
		}
		for (auto Map : HitMap)
		{
			if (Map.Key && HasAuthority())
			{
				UGameplayStatics::ApplyDamage(Map.Key, Damage * Map.Value, InstigatorController,this, UDamageType::StaticClass());
			}
			
		}
	}
	Super::Fire(HitTarget);
}
