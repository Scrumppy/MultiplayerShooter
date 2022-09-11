// Fill out your copyright notice in the Description page of Project Settings.


#include "GEII_PortalGameInstance.h"

#include "Blueprint/UserWidget.h"
#include "OnlineSubsystem.h"
#include "UObject/ConstructorHelpers.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MenuSystem/MainMenu.h"

const static FName SESSION_NAME = TEXT("My Session");
const static FName SERVER_NAME_SETTINGS_KEY = TEXT("ServerName");

UGEII_PortalGameInstance::UGEII_PortalGameInstance(const FObjectInitializer& ObjectInitializer)
{
	//Class Finders
	ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/Game/Menu/WBP_MainMenu"));
	if (!ensure(MenuBPClass.Class != nullptr)) return;

	MenuClass = MenuBPClass.Class;
}

void UGEII_PortalGameInstance::Init()
{
	//Get Online Subsystem
	IOnlineSubsystem* Subsystem =  IOnlineSubsystem::Get();
	if (Subsystem != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found Subsystem %s"), *Subsystem->GetSubsystemName().ToString());
		//Get Session Interface
		SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Found session interface"))
			//Create Delegates
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UGEII_PortalGameInstance::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UGEII_PortalGameInstance::OnDestroySessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UGEII_PortalGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UGEII_PortalGameInstance::OnJoinSessionComplete);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Found No Subsystem"))
	}
	UE_LOG(LogTemp, Warning, TEXT("Found Class %s"), *MenuClass->GetName());

	if (GEngine != nullptr)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UGEII_PortalGameInstance::OnNetworkFailure);
	}
}

void UGEII_PortalGameInstance::LoadMenu()
{
	if (!ensure(MenuClass != nullptr)) return;

	//Create Menu Widget
	Menu = CreateWidget<UMainMenu>(this, MenuClass);
	if (!ensure(Menu != nullptr)) return;

	//Add Menu To Viewport
	Menu->Setup();
	Menu->SetMenuInterface(this);
}

void UGEII_PortalGameInstance::Host(FString ServerName)
{
	DesiredServerName = ServerName;
	if (SessionInterface.IsValid())
	{
		//Get Existing Session
		auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
		if (ExistingSession != nullptr)
		{
			//Destroy Existing Session
			SessionInterface->DestroySession(SESSION_NAME);
		}
		else
		{
			//Create New Session
			CreateSession();
		}
	}
}

void UGEII_PortalGameInstance::Join(uint32 Index)
{
	if (!SessionInterface.IsValid()) return;
	if (!SessionSearch.IsValid()) return;
	
	if (Menu != nullptr)
	{
		//Destroy Menu Widget Once Game Is Loaded
		Menu->Teardown();
	}

	SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);
}

void UGEII_PortalGameInstance::LoadMainMenu()
{
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) return;

	PlayerController->ClientTravel("/Game/Menu/MainMenu", ETravelType::TRAVEL_Absolute);
}

void UGEII_PortalGameInstance::OnCreateSessionComplete(FName SessionName, bool Success)
{
	if (!Success)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create session"));
		return;
	}

	if (Menu != nullptr)
	{
		Menu->Teardown();
	}

	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	//Travel Host To Map
	World->ServerTravel("/Game/FirstPersonCPP/Maps/FirstPersonExampleMap?listen");
}

void UGEII_PortalGameInstance::OnDestroySessionComplete(FName SessionName, bool Success)
{
	if (Success)
	{
		CreateSession();
	}
}

void UGEII_PortalGameInstance::RefreshServerList()
{
	//Session Search Results
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid())
	{
		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		UE_LOG(LogTemp, Warning, TEXT("Starting to find session..."))
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UGEII_PortalGameInstance::StartSession()
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->StartSession(SESSION_NAME);		
	}
}

void UGEII_PortalGameInstance::OnFindSessionsComplete(bool Success)
{
	if (Success && SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Finished finding sessions..."))
		
		TArray<FServerData> ServerNames;
		//Show Up Session Results
		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found session names: %s"), *	SearchResult.GetSessionIdStr())
			FServerData Data;
			Data.Name = SearchResult.GetSessionIdStr();
			Data.HostUserName = SearchResult.Session.OwningUserName;
			Data.CurrentPlayers = Data.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;
			Data.MaxPlayers = 4;

			FString ServerName;
			
			 if (SearchResult.Session.SessionSettings.Get(SERVER_NAME_SETTINGS_KEY, ServerName))
			 {
				 Data.Name = ServerName;
			 }
			 else
			 {
			 	 Data.Name = "Could not find name";
			 }
			
			ServerNames.Add(Data);
		}
		Menu->SetServerList(ServerNames);
	}
}

void UGEII_PortalGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{	
	if(!SessionInterface.IsValid()) return;

	FString Address;
	if(!SessionInterface->GetResolvedConnectString(SessionName, Address))
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not get connect string"))
		return;
	}
	
	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;
	Engine->AddOnScreenDebugMessage(0, 4.0f, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));
	
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) return;
	
	//Travel To Server
	PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void UGEII_PortalGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType,
	const FString& ErrorString)
{
	LoadMainMenu();
}

void UGEII_PortalGameInstance::CreateSession()
{
	if (SessionInterface.IsValid())
	{
		//Set Session Settings
		FOnlineSessionSettings SessionSettings;
		IOnlineSubsystem* Subsystem =  IOnlineSubsystem::Get();
		
		 if (Subsystem->GetSubsystemName() == "NULL")
		 {
			 SessionSettings.bIsLANMatch = true;
		 }
		 else
		 {
			 SessionSettings.bIsLANMatch = false;
		 }
		
		SessionSettings.NumPublicConnections = 4;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bUsesPresence = true;
		SessionSettings.Set(SERVER_NAME_SETTINGS_KEY, DesiredServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		//Create Session
		SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);

		UE_LOG(LogTemp, Warning, TEXT("Created Session %s"), *SESSION_NAME.ToString())
	}
}


