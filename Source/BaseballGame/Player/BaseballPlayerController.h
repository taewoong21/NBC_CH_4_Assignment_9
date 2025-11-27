// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BaseballPlayerController.generated.h"

class UChatInput;
class UUserWidget;

UCLASS()
class BASEBALLGAME_API ABaseballPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ABaseballPlayerController();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	void SetChatMessageString(const FString& InChatMessageString);

	void PrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Client, Reliable)
	void ClientRPCPrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Server, Reliable)
	void ServerRPCPrintChatMessageString(const FString& InChatMessageString);

protected:
	/* 입력 위젯 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UChatInput> ChatInputWidgetClass;

	UPROPERTY()
	TObjectPtr<UChatInput> ChatInputWidgetInstance;

	FString ChatMessageString;

	/* 공지 위젯 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> NotificationTextWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> NotificationTextWidgetInstance;

	/* 턴 타이머 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> TurnTimerWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> TurnTimerWidgetInstance;

public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	FText NotificationText; // 공지 내용
};
