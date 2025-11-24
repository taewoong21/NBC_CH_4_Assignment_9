// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BaseballGameModeBase.h"
#include "Game/BaseballGameStateBase.h"
#include "Player/BaseballPlayerController.h"
#include "Player/BaseballPlayerState.h"
#include "EngineUtils.h"

void ABaseballGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	/*ABaseballGameStateBase* BBGameStateBase = GetGameState<ABaseballGameStateBase>();
	if (IsValid(BBGameStateBase) == true)
	{
		BBGameStateBase->MulticastRPCBroadcastLoginMessage(TEXT("XXXXXXX"));
	}

	ABaseballPlayerController* BBPlayerController = Cast<ABaseballPlayerController>(NewPlayer);
	if (IsValid(BBPlayerController) == true)
	{
		AllPlayerControllers.Add(BBPlayerController);
	}*/

	ABaseballPlayerController* BBPlayerController = Cast<ABaseballPlayerController>(NewPlayer);
	if (IsValid(BBPlayerController) == true)
	{
		BBPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));

		AllPlayerControllers.Add(BBPlayerController);

		ABaseballPlayerState* BBPS = BBPlayerController->GetPlayerState<ABaseballPlayerState>();
		if (IsValid(BBPS) == true)
		{
			BBPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
		}

		ABaseballGameStateBase* BBGameState = GetGameState<ABaseballGameStateBase>();
		if (IsValid(BBGameState) == true)
		{
			BBGameState->MulticastRPCBroadcastLoginMessage(BBPS->PlayerNameString);
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

	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp, Error, TEXT("%s"), *SecretNumberString);
}

void ABaseballGameModeBase::PrintChatMessageString(ABaseballPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	int Index = InChatMessageString.Len() - 3;
	FString GuessNumberString = InChatMessageString.RightChop(Index);
	if (IsGuessNumberString(GuessNumberString) == true) // 게임 진행에 필요한 입력인지 유효성 체크
	{
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
	SecretNumberString = GenerateSecretNumber();

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
