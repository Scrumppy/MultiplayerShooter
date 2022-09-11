#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_PortalGun UMETA(DisplayName = "Portal Gun"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};