// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BaseballPlayerState.h"
#include "Net/UnrealNetwork.h"

ABaseballPlayerState::ABaseballPlayerState(): PlayerNameString(TEXT("None")), CurrentGuessCount(0), MaxGuessCount(3), TurnRemainingTime(30.0f), bHasGuessTrurn(false)
{
	bReplicates = true;
}

void ABaseballPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PlayerNameString);
	DOREPLIFETIME(ThisClass, CurrentGuessCount);
	DOREPLIFETIME(ThisClass, MaxGuessCount);
	DOREPLIFETIME(ThisClass, bHasGuessTrurn);
	DOREPLIFETIME(ThisClass, TurnRemainingTime); 
}

FString ABaseballPlayerState::GetPlayerInfoString()
{
	FString PlayerInfoString = PlayerNameString + TEXT("(") + FString::FromInt(CurrentGuessCount) + TEXT("/") + FString::FromInt(MaxGuessCount) + TEXT(")");

	return PlayerInfoString;
}
