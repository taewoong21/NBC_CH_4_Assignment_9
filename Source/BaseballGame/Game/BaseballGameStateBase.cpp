// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BaseballGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BaseballPlayerController.h"

void ABaseballGameStateBase::MulticastRPCBroadcastLoginMessage_Implementation(const FString& InNameString)
{
	if (HasAuthority() == false)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PC) == true)
		{
			ABaseballPlayerController* BBPC = Cast<ABaseballPlayerController>(PC);
			if (IsValid(BBPC) == true)
			{
				FString NotificationString = InNameString + TEXT(" has Joined the game.");
				BBPC->PrintChatMessageString(NotificationString);
			}
		}
	}
}
