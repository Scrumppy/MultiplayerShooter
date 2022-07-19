// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Engine/GameInstance.h"
#include "MenuSystem/MainMenu.h"
#include "MenuSystem/MenuInterface.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "GEII_PortalGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class GEII_PORTALPROJECT_API UGEII_PortalGameInstance : public UGameInstance, public IMenuInterface
{
	GENERATED_BODY()

public:
	UGEII_PortalGameInstance(const FObjectInitializer & ObjectInitializer);

	virtual void Init();


    //UI and Connection Functions
	UFUNCTION(BlueprintCallable)
	void LoadMenu();
	
	UFUNCTION(Exec)
	virtual void Host(FString ServerName) override;

	UFUNCTION(Exec)
	virtual void Join(uint32 Index) override;

	virtual void LoadMainMenu() override;

	virtual void RefreshServerList() override;

	void StartSession();
	
private:
    //Class References and Pointers
	TSubclassOf<class UUserWidget> MenuClass;
	
	UMainMenu* Menu;

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

    //Delegates
	void OnCreateSessionComplete(FName SessionName, bool Success);
	void OnDestroySessionComplete(FName SessionName, bool Success);
	void OnFindSessionsComplete(bool Success);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	
	FString DesiredServerName;
	void CreateSession();
};

