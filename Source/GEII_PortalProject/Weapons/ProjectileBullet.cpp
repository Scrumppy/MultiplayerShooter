// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "GEII_PortalProject/GEII_PortalProjectCharacter.h"
#include "Kismet/GameplayStatics.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	AGEII_PortalProjectCharacter* OwnerCharacter = Cast<AGEII_PortalProjectCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		AController* OwnerController = OwnerCharacter->Controller;
		if (OwnerController)
		{
			if (OwnerCharacter->HasAuthority())
			{
				const float DamageToCause = Hit.BoneName.ToString() == FString("head") ? HeadshotDamage : Damage;
				
				UGameplayStatics::ApplyDamage(OtherActor, DamageToCause, OwnerController, this, UDamageType::StaticClass());
			}
		}
		
	}
	
	//Called last because the super function destroys the bullet
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
