// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MainMenu.h"
#include "Blueprint/UserWidget.h"
#include "ServerRow.generated.h"

/**
 * 
 */
UCLASS()
class GEII_PORTALPROJECT_API UServerRow : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta =(BindWidget))
	class UTextBlock* ServerName;
	
	UPROPERTY(meta =(BindWidget))
	 UTextBlock* HostUser;
	
	UPROPERTY(meta =(BindWidget))
	 UTextBlock* ConnectionFraction;

	UPROPERTY(BlueprintReadOnly)
	bool Selected = false;
	
	void Setup(UMainMenu* InParent, uint32 InIndex);

private:
	UPROPERTY(meta =(BindWidget))
	UButton* RowButton;

	UPROPERTY()
	UMainMenu* Parent;
	
	uint32 Index;

	UFUNCTION()
	void OnClicked();
};
