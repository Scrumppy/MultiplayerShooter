// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PortalFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class GEII_PORTALPROJECT_API UPortalFunctionLibrary : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static FVector ConvertLocation(FVector const& Location, AActor* Portal, AActor* Target);

	UFUNCTION(BlueprintCallable)
	static FRotator ConvertRotation(FRotator const& Rotation, AActor* Portal, AActor* Target);

	UFUNCTION(BlueprintCallable)
	static bool CheckIsInFront(FVector const& Point, FVector const& PortalLocation, FVector const& PortalNormal);
	
	UFUNCTION(BlueprintCallable)
	static bool CheckIsCrossing(FVector const& Point, FVector const& PortalLocation, FVector const& PortalNormal, bool& out_LastInFront, FVector& out_LastPosition);

};
