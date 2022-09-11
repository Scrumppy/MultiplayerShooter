// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class GEII_PORTALPROJECT_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void ApplyDamage();
	
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;


private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	float DamageInnerRadius = 200.f;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	float DamageOuterRadius = 500.f;
	
};
