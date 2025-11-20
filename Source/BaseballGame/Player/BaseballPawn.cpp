// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BaseballPawn.h"
#include "BaseballGame.h"



void ABaseballPawn::BeginPlay()
{
	Super::BeginPlay();
	
	FString NetRoleString = BaseballGameFunctionLibrary::GetRoleString(this);
	FString CombinedString = FString::Printf(TEXT("BaseballPawn::BeginPlay() %s [%s]"), *BaseballGameFunctionLibrary::GetNetModeString(this), *NetRoleString);
	BaseballGameFunctionLibrary::MyPrintString(this, CombinedString, 10.f);
}

void ABaseballPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	FString NetRoleString = BaseballGameFunctionLibrary::GetRoleString(this);
	FString CombinedString = FString::Printf(TEXT("BaseballPawn::PossessedBy() %s [%s]"), *BaseballGameFunctionLibrary::GetNetModeString(this), *NetRoleString);
	BaseballGameFunctionLibrary::MyPrintString(this, CombinedString, 10.f);
}


