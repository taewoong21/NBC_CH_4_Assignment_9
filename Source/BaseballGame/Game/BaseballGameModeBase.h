// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BaseballGameModeBase.generated.h"

class ABaseballPlayerController;

UCLASS()
class BASEBALLGAME_API ABaseballGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void OnPostLogin(AController* NewPlayer) override; // 로그인 확인

	FString GenerateSecretNumber(); // 난수 생성

	bool IsGuessNumberString(const FString& InNumberString); // 유효 숫자 확인 (중복 체크)

	FString JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString);

	virtual void BeginPlay() override;

	void PrintChatMessageString(ABaseballPlayerController* InChattingPlayerController, const FString& InChatMessageString);

	void IncreaseGuessCount(ABaseballPlayerController* InChattingPlayerController);

	void ResetGame();

	void JudgeGame(ABaseballPlayerController* InChattingPlayerController, int InStrikeCount);

protected:
	FString SecretNumberString;

	TArray<TObjectPtr<ABaseballPlayerController>> AllPlayerControllers;
};
