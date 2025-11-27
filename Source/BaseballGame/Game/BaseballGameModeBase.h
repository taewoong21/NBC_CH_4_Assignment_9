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
	ABaseballGameModeBase();

	virtual void OnPostLogin(AController* NewPlayer) override; // 로그인 확인

	FString GenerateSecretNumber(); // 난수 생성

	bool IsGuessNumberString(const FString& InNumberString); // 유효 숫자 확인 (중복 체크)

	FString JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString); // 답안 결과 판정

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	void PrintChatMessageString(ABaseballPlayerController* InChattingPlayerController, const FString& InChatMessageString); // 메시지 출력

	void IncreaseGuessCount(ABaseballPlayerController* InChattingPlayerController); // 턴수 증가

	void ResetGame(); // 게임 리셋

	void JudgeGame(ABaseballPlayerController* InChattingPlayerController, int InStrikeCount); // 게임 결과 판단

protected:
	FString SecretNumberString; // 게임 답안

	TArray<TObjectPtr<ABaseballPlayerController>> AllPlayerControllers; // 접속 플레이어 목록

private:
	float MaxTurnDuration = 30.0f;
};
