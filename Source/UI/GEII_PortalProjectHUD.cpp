// Copyright Epic Games, Inc. All Rights Reserved.

#include "GEII_PortalProjectHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "GEII_PortalPlayerController.h"
#include "GEII_PortalProjectCharacter.h"
#include "DrawDebugHelpers.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AGEII_PortalProjectHUD::AGEII_PortalProjectHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/FirstPerson/Textures/FirstPersonCrosshair"));
	CrosshairTex = CrosshairTexObj.Object;

	//Find Character Overlay Class
	ConstructorHelpers::FClassFinder<UUserWidget> CharacterOverlayBPClass(TEXT("/Game/FirstPersonCPP/Blueprints/WBP_CharacterOverlay"));
	if (!ensure(CharacterOverlayBPClass.Class != nullptr)) return;

	CharacterOverlayClass = CharacterOverlayBPClass.Class;
}

void AGEII_PortalProjectHUD::BeginPlay()
{
	Super::BeginPlay();

	if (CharacterOverlayClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found Class %s"), *CharacterOverlayClass->GetName());
	}
	
	AddCharacterOverlay();
}

void AGEII_PortalProjectHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void AGEII_PortalProjectHUD::DrawHUD()
{
	Super::DrawHUD();
	// Draw very simple crosshair

	FVector2D ViewportSize;
	
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize); 
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y/ 2.f);
	
	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition( (Center.X), (Center.Y + 0.0f));

	// draw the crosshair
	FCanvasTileItem TileItem( CrosshairLocation, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TileItem);

}


