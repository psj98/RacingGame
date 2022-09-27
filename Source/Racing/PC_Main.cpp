// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_Main.h"

void APC_Main::BeginPlay()
{
	Super::BeginPlay();
	SetInputMode(FInputModeGameAndUI());
}