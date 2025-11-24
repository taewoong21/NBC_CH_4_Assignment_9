// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BaseballPlayerController.h"
#include "Player/BaseballPlayerState.h"
#include "UI/ChatInput.h"
#include "Game/BaseballGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "BaseballGame.h"
#include "EngineUtils.h"

ABaseballPlayerController::ABaseballPlayerController()
{
	bReplicates = true;
}

void ABaseballPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false) // Stand Alone 상태일 때는 무시
	{
		return;
	}

	// Owning Client 일 때, 실행 (Owning Client = Local Client)
	// InputMode를 UI에 집중 : Chatting만 구현하기 때문
	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	if (IsValid(ChatInputWidgetClass) == true)
	{
		ChatInputWidgetInstance = CreateWidget<UChatInput>(this, ChatInputWidgetClass);
		if (IsValid(ChatInputWidgetInstance) == true)
		{
			ChatInputWidgetInstance->AddToViewport();
		}
	}

	if (IsValid(NotificationTextWidgetClass) == true)
	{
		NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
		if (IsValid(NotificationTextWidgetInstance) == true)
		{
			NotificationTextWidgetInstance->AddToViewport();
		}
	}
}

void ABaseballPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NotificationText);
}

void ABaseballPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	ChatMessageString = InChatMessageString;

	// PrintChatMessageString(ChatMessageString);

	if (IsLocalController() == true)
	{
		// ServerRPCPrintChatMessageString(InChatMessageString);

		ABaseballPlayerState* BBPS = GetPlayerState<ABaseballPlayerState>();
		if (IsValid(BBPS) == true)
		{
			FString CombinedMessageString = BBPS->GetPlayerInfoString() + TEXT(": ") + InChatMessageString;

			ServerRPCPrintChatMessageString(CombinedMessageString);
		}
	}
}

void ABaseballPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	// UKismetSystemLibrary::PrintString(this, ChatMessageString, true, true, FLinearColor::Red, 0.5f);

	/*FString NetModeString = BaseballGameFunctionLibrary::GetNetModeString(this);
	FString CombinedMessageString = FString::Printf(TEXT("%s: %s"), *NetModeString, *InChatMessageString);
	BaseballGameFunctionLibrary::MyPrintString(this, CombinedMessageString, 10.f);*/

	BaseballGameFunctionLibrary::MyPrintString(this, InChatMessageString, 10.f);
}

void ABaseballPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);
}

void ABaseballPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	// Server에 존재하는 Player Controller 중 유효한 것을 찾기
	/*for (TActorIterator<ABaseballPlayerController> It(GetWorld()); It; ++It)
	{
		ABaseballPlayerController* BBPlayerController = *It;
		if (IsValid(BBPlayerController) == true)
		{
			BBPlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
		}
	}*/

	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM) == true)
	{
		ABaseballGameModeBase* BBGM = Cast<ABaseballGameModeBase>(GM);
		if (IsValid(BBGM) == true)
		{
			BBGM->PrintChatMessageString(this, InChatMessageString);
		}
	}
}
