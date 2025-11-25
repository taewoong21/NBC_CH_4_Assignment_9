// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BaseballGameModeBase.h"
#include "Game/BaseballGameStateBase.h"

void ABaseballGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	ABaseballGameStateBase* BBGameStateBase = GetGameState<ABaseballGameStateBase>();
	if (IsValid(BBGameStateBase) == true)
	{
		BBGameStateBase->MulticastRPCBroadcastLoginMessage(TEXT("XXXXXXX"));
	}
}
