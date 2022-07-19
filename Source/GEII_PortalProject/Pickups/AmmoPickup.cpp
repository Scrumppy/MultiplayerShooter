// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"

#include "GEII_PortalProject/GEII_PortalProjectCharacter.h"

void AAmmoPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AGEII_PortalProjectCharacter* PortalCharacter = Cast<AGEII_PortalProjectCharacter>(OtherActor);
	if (PortalCharacter)
	{
		UCombatComponent* CombatComponent = PortalCharacter->GetCombatComponent();
		if (CombatComponent)
		{
			CombatComponent->PickupAmmo(WeaponType, AmmoAmountToAdd);
		}
	}
	Destroy();
}
