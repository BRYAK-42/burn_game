// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShadersGameMode.h"
#include "ShadersHUD.h"
#include "ShadersCharacter.h"
#include "UObject/ConstructorHelpers.h"

AShadersGameMode::AShadersGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AShadersHUD::StaticClass();
}
