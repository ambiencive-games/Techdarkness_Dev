// Copyright Epic Games, Inc. All Rights Reserved.

#include "Techdarkness_DevGameMode.h"
#include "Techdarkness_DevCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATechdarkness_DevGameMode::ATechdarkness_DevGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
