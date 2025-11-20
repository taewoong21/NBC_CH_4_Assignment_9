// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BaseballPlayerController.h"
#include "UI/ChatInput.h"
#include "Kismet/KismetSystemLibrary.h"
#include "BaseballGame.h"
#include "EngineUtils.h"

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
}

void ABaseballPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	ChatMessageString = InChatMessageString;

	// PrintChatMessageString(ChatMessageString);

	if (IsLocalController() == true)
	{
		ServerRPCPrintChatMessageString(InChatMessageString);
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

void ABaseballPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	// Server에 존재하는 Player Controller 중 유효한 것을 찾기
	for (TActorIterator<ABaseballPlayerController> It(GetWorld()); It; ++It)
	{
		ABaseballPlayerController* BBPlayerController = *It;
		if (IsValid(BBPlayerController) == true)
		{
			BBPlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
		}
	}
}

void ABaseballPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);
}
