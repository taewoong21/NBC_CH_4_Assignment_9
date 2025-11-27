// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BaseballGameModeBase.h"
#include "Game/BaseballGameStateBase.h"
#include "Player/BaseballPlayerController.h"
#include "Player/BaseballPlayerState.h"
#include "EngineUtils.h"

ABaseballGameModeBase::ABaseballGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABaseballGameModeBase::OnPostLogin(AController* NewPlayer) // 로그인 확인
{
	Super::OnPostLogin(NewPlayer);

	ABaseballPlayerController* BBPlayerController = Cast<ABaseballPlayerController>(NewPlayer);
	if (IsValid(BBPlayerController) == true)
	{
		BBPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server.")); // Notification Text 등록

		AllPlayerControllers.Add(BBPlayerController); // 접속자 추가 등록

		ABaseballPlayerState* BBPS = BBPlayerController->GetPlayerState<ABaseballPlayerState>();
		if (IsValid(BBPS) == true)
		{
			BBPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num()); // 접속자명 할당
		}

		ABaseballGameStateBase* BBGameState = GetGameState<ABaseballGameStateBase>();
		if (IsValid(BBGameState) == true)
		{
			BBGameState->MulticastRPCBroadcastLoginMessage(BBPS->PlayerNameString); // 추가 접속자 발생 시 MulticastRPC 브로드캐스트
		}
	}
}

FString ABaseballGameModeBase::GenerateSecretNumber()
{
	TArray<int32> Numbers;
	for (int32 i = 1; i <= 9; ++i)
	{
		Numbers.Add(i);
	}

	FMath::RandInit(FDateTime::Now().GetTicks());
	Numbers = Numbers.FilterByPredicate([](int32 Num) { return Num > 0; });

	FString Result;
	for (int32 i = 0; i < 3; ++i)
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}

bool ABaseballGameModeBase::IsGuessNumberString(const FString& InNumberString)
{
	bool bCanPlay = false;

	do
	{
		if (InNumberString.Len() != 3)
		{
			break;
		}

		bool bIsUnique = true;
		TSet<TCHAR> UniqueDigits;
		for (TCHAR C : InNumberString)
		{
			if (FChar::IsDigit(C) == false || C == '0')
			{
				bIsUnique = false;
				break;
			}

			UniqueDigits.Add(C);
		}

		if (bIsUnique == false)
		{
			break;
		}

		bCanPlay = true;
	} while (false);

	return bCanPlay;
}

FString ABaseballGameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString)
{
	int32 StrikeCount = 0;
	int32 BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (InSecretNumberString[i] == InGuessNumberString[i]) // 숫자 + 자리 동일 시
		{
			StrikeCount++;
		}
		else
		{
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (InSecretNumberString.Contains(PlayerGuessChar)) // 숫자가 포함될 시 (자리는 동일 X)
			{
				BallCount++;
			}
		}
	}

	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void ABaseballGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SecretNumberString = GenerateSecretNumber(); // 게임 시작 시, 난수 생성
	UE_LOG(LogTemp, Error, TEXT("%s"), *SecretNumberString);
}

void ABaseballGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (ABaseballPlayerController* PC : AllPlayerControllers)
	{
		if (IsValid(PC) == true)
		{
			ABaseballPlayerState* PS = PC->GetPlayerState<ABaseballPlayerState>();
			if (IsValid(PS) == true)
			{
				if (PS->TurnRemainingTime > 0.0f) // Delta Time 만큼 시간 차감
				{
					PS->TurnRemainingTime -= DeltaTime;
				}

				if (PS->TurnRemainingTime <= 0.0f) // 시간 초과 시, 턴 소진
				{
					if (PS->bHasGuessTrurn == false) // 예측 시도를 하지 않았을 시, 턴 소진
					{
						IncreaseGuessCount(PC); // 턴 소진

						/* 턴 소진 결과 출력 */
						FString Message = FString::Printf(TEXT("%s Time Out! Lost your turn."), *PS->PlayerNameString);
						PC->ClientRPCPrintChatMessageString(Message);

						JudgeGame(PC, 0);
					}

					/* 시간 및 상태 초기화 (다음 턴 시작) */
					PS->TurnRemainingTime = MaxTurnDuration;
					PS->bHasGuessTrurn = false;
				}
			}
		}
	}
}

void ABaseballGameModeBase::PrintChatMessageString(ABaseballPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	/* 턴 시간 소진 시, 플레이 불가 */
	ABaseballPlayerState* PS = InChattingPlayerController->GetPlayerState<ABaseballPlayerState>();
	if (IsValid(PS) == true && PS->TurnRemainingTime <= 0)
	{
		return;
	}

	int Index = InChatMessageString.Len() - 3;
	FString GuessNumberString = InChatMessageString.RightChop(Index);

	if (IsGuessNumberString(GuessNumberString) == true) // 게임 진행에 필요한 입력(숫자 예측 입력)인지 유효성 체크
	{
		if (IsValid(PS) == true)
		{
			PS->bHasGuessTrurn = true; // 턴 진행
		}

		FString JudgeResultString = JudgeResult(SecretNumberString, GuessNumberString); // 게임 진행 및 결과 판단

		IncreaseGuessCount(InChattingPlayerController);

		for (TActorIterator<ABaseballPlayerController> It(GetWorld()); It; ++It)
		{
			ABaseballPlayerController* BBPlayerController = *It;
			if (IsValid(BBPlayerController) == true)
			{
				FString CombinedMessageString = InChatMessageString + TEXT(" -> ") + JudgeResultString;
				BBPlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);

				int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));
				JudgeGame(InChattingPlayerController, StrikeCount);
			}
		}
	}
	else // 게임 진행에 필요한 입력이 아닌 경우 (일반 대화)
	{
		for (TActorIterator<ABaseballPlayerController> It(GetWorld()); It; ++It)
		{
			ABaseballPlayerController* BBPlayerController = *It;
			if (IsValid(BBPlayerController) == true)
			{
				BBPlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
			}
		}
	}
}

void ABaseballGameModeBase::IncreaseGuessCount(ABaseballPlayerController* InChattingPlayerController)
{
	ABaseballPlayerState* BBPS = InChattingPlayerController->GetPlayerState<ABaseballPlayerState>();
	if (IsValid(BBPS) == true)
	{
		BBPS->CurrentGuessCount++;
	}
}

void ABaseballGameModeBase::ResetGame()
{
	UE_LOG(LogTemp, Warning, TEXT("Reset the Game. Start next round."));

	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp, Error, TEXT("%s"), *SecretNumberString);

	for (const auto& BBPlayerController : AllPlayerControllers)
	{
		ABaseballPlayerState* BBPS = BBPlayerController->GetPlayerState<ABaseballPlayerState>();
		if (IsValid(BBPS) == true)
		{
			BBPS->CurrentGuessCount = 0;
		}
	}
}

void ABaseballGameModeBase::JudgeGame(ABaseballPlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (InStrikeCount == 3)
	{
		ABaseballPlayerState* BBPS = InChattingPlayerController->GetPlayerState<ABaseballPlayerState>();

		for (const auto& BBPlayerController : AllPlayerControllers)
		{
			if (IsValid(BBPS) == true)
			{
				FString CombinedMessageString = BBPS->PlayerNameString + TEXT(" has won the game.");
				BBPlayerController->NotificationText = FText::FromString(CombinedMessageString);

				ResetGame();
			}
		}
	}
	else
	{
		bool bIsDraw = true;

		for (const auto& BBPlayerController : AllPlayerControllers)
		{
			ABaseballPlayerState* BBPS = BBPlayerController->GetPlayerState<ABaseballPlayerState>();
			if (IsValid(BBPS) == true)
			{
				if (BBPS->CurrentGuessCount < BBPS->MaxGuessCount)
				{
					bIsDraw = false;
					break;
				}
			}
		}

		if (bIsDraw == true)
		{
			for (const auto& BBPlayerController : AllPlayerControllers)
			{
				BBPlayerController->NotificationText = FText::FromString(TEXT("Draw..."));

				ResetGame();
			}
		}
	}
}
