// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"

void AProjectileRocket::ApplyDamage()
{
	APawn* FiringPawn =  GetInstigator();
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(this, Damage, 10.f, GetActorLocation(), DamageInnerRadius, DamageOuterRadius, 1.f,
				UDamageType::StaticClass(), TArray<AActor*>(), this, FiringController);
		}
	}
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
			ApplyDamage();
	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
