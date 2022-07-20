// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalFunctionLibrary.h"

FVector UPortalFunctionLibrary::ConvertLocation(FVector const& Location, AActor* Portal, AActor* Target)
{
	//Get direction of the portal
	FVector Direction = Location - Portal->GetActorLocation();
	//Get target location
	FVector TargetLocation = Target->GetActorLocation();

	//Using dot product to check if the direction of the first parameter is facing the second parameter direction, if equal to 1 it is essentialy facing
	FVector Dots;
	Dots.X = FVector::DotProduct(Direction, Portal->GetActorForwardVector());
	Dots.Y = FVector::DotProduct(Direction, Portal->GetActorRightVector());
	Dots.Z = FVector::DotProduct(Direction, Portal->GetActorUpVector());

	//Change the direction of the player when he teleports
	FVector NewDirection = Dots.X * -Target->GetActorForwardVector()
		+ Dots.Y * -Target->GetActorRightVector()
		+ Dots.Z * Target->GetActorUpVector();

	//Convert the location and apply direction
	return TargetLocation + NewDirection;
}

FRotator UPortalFunctionLibrary::ConvertRotation(FRotator const& Rotation, AActor* Portal, AActor* Target)
{
	//Direction of which the player is going to face
	FVector RotationAdjustment(0.f, 0.f, -180.f);
	FVector LocalAdjustment = FVector::ZeroVector;

	//Check if the dot product value is approximetly close to each other
	if (FVector::DotProduct(Portal->GetActorForwardVector(), FVector::UpVector) > KINDA_SMALL_NUMBER)
	{
		//Make sure the angle is between + or - 180 degrees
		LocalAdjustment.X = FMath::UnwindDegrees(Portal->GetTransform().GetRotation().Euler().X);
		LocalAdjustment.Y = 180.f;
		RotationAdjustment.Z += LocalAdjustment.X;
	}
	//If the dot product value is less than the kinda small number
	else if (FVector::DotProduct(Portal->GetActorForwardVector(), FVector::UpVector) < -KINDA_SMALL_NUMBER)
	{
		LocalAdjustment.X = FMath::UnwindDegrees(Portal->GetTransform().GetRotation().Euler().X);
		LocalAdjustment.Y = -180.f;
		RotationAdjustment.Z -= LocalAdjustment.X;
	}
	
	//Represents a rotation around an axis, to rotate the character around an axis
	FQuat QuatRotation = FQuat::MakeFromEuler(RotationAdjustment) * FQuat(Rotation);
	//Transform our portal rotation from world space to local space
	FQuat LocalQuat = FQuat::MakeFromEuler(LocalAdjustment) * Portal->GetActorTransform().GetRotation().Inverse() * QuatRotation;
	//Convert the rotation
	FQuat NewWorldQuat = Target->GetActorTransform().GetRotation() * LocalQuat;
	return NewWorldQuat.Rotator();
	
}

bool UPortalFunctionLibrary::CheckIsInFront(FVector const& Point, FVector const& PortalLocation,
	FVector const& PortalNormal)
{
	//Create imaginary plane at the direction and location of the portal
	FPlane PortalPlane = FPlane(PortalLocation, PortalNormal);
	//Calculate the distance between the plane and a point, if that distance is greater or equal to 0, it will return true, otherwise will return false
	return PortalPlane.PlaneDot(Point) >= 0;
}

bool UPortalFunctionLibrary::CheckIsCrossing(FVector const& Point, FVector const& PortalLocation,
	FVector const& PortalNormal, bool& out_LastInFront, FVector& out_LastPosition)
{
	//Create a plane
	FPlane PortalPlane = FPlane(PortalLocation, PortalNormal);
	FVector IntersectionPoint;
	//Will return true if there is an intersection between the segment plane and the plane where the portal is	
	bool IsIntersecting = FMath::SegmentPlaneIntersection(out_LastPosition,Point,PortalPlane,IntersectionPoint);

	bool IsInFront = CheckIsInFront(Point, PortalLocation, PortalNormal);
	bool IsCrossing = false;

	//if intersecting and we are not in front
	if (IsIntersecting && !IsInFront && out_LastInFront)
	{
		IsCrossing = true;
	}

	out_LastInFront = IsInFront;
	out_LastPosition = Point;

	return IsCrossing;
}
