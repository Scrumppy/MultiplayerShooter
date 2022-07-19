// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuInterface.h"
#include "Components/EditableTextBox.h"
#include "Components/WidgetSwitcher.h"
#include "MainMenu.generated.h"

USTRUCT()
struct FServerData
{
	GENERATED_BODY()

	FString Name;
	FString HostUserName;
	uint16 CurrentPlayers;
	uint16 MaxPlayers;
};

/**
 * 
 */
UCLASS()
class GEII_PORTALPROJECT_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenu(const FObjectInitializer & ObjectInitializer);

	void SetServerList(TArray<FServerData> ServerNames);
	
	void SetMenuInterface(IMenuInterface* MenuInterfaceToSet);

	void SelectIndex(uint32 Index);

	void Setup();

	void Teardown();
	
protected:
	virtual bool Initialize();
	
private:
	TSubclassOf<class UUserWidget> ServerRowClass;

	//Widget Binds
	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* ConfirmJoinMenuButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* CancelJoinMenuButton;

	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;

	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* MenuSwitcher;

	UPROPERTY(meta = (BindWidget))
	UWidget* MainMenu;
	
	UPROPERTY(meta = (BindWidget))
	UWidget* JoinMenu;

	UPROPERTY(meta = (BindWidget))
	UWidget* HostMenu;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ServerHostName;

	UPROPERTY(meta = (BindWidget))
	UButton* ConfirmHostMenuButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CancelHostMenuButton;

	UPROPERTY(meta = (BindWidget))
	UPanelWidget* ServerList;

	//Menu Functions
	UFUNCTION()
	void  HostServer();
	UFUNCTION()
	void OpenHostMenu();
	UFUNCTION()
	void OpenJoinMenu();
	UFUNCTION()
	void  OpenMainMenu();
	UFUNCTION()
	void JoinServer();
	UFUNCTION()
	void QuitPressed();

	TOptional<uint32> SelectedIndex;
	
	//Interface Pointer
	IMenuInterface* MenuInterface;

	void UpdateChildren();

};
