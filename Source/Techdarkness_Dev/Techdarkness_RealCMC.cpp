// Fill out your copyright notice in the Description page of Project Settings.
#include "Techdarkness_RealCMC.h"
#include "GameFramework/Character.h"


bool UTechdarkness_RealCMC::FSavedMove_Real::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const {
	FSavedMove_Real* NewRealMove = static_cast<FSavedMove_Real*>(NewMove.Get());

	if (Saved_bWantsToSprint != NewRealMove->Saved_bWantsToSprint) {
		return false;
	}
	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UTechdarkness_RealCMC::FSavedMove_Real::Clear() {
	FSavedMove_Character::Clear();

	Saved_bWantsToSprint = 0;
}

uint8 UTechdarkness_RealCMC::FSavedMove_Real::GetCompressedFlags() const {
	uint8 Result = Super::GetCompressedFlags();

	if (Saved_bWantsToSprint) Result = FLAG_Custom_0;

	return Result;
}

void UTechdarkness_RealCMC::FSavedMove_Real::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) {
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	UTechdarkness_RealCMC* CharacterMovement = Cast<UTechdarkness_RealCMC>(C->GetCharacterMovement());

	Saved_bWantsToSprint = CharacterMovement->Safe_bWantsToSprint;
}

void UTechdarkness_RealCMC::FSavedMove_Real::PrepMoveFor(ACharacter* C) {
	Super::PrepMoveFor(C);

	UTechdarkness_RealCMC* CharacterMovement = Cast<UTechdarkness_RealCMC>(C->GetCharacterMovement());

	CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;
}

FNetworkPredictionData_Client* UTechdarkness_RealCMC::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr)

	if (ClientPredictionData == nullptr) {
		UTechdarkness_RealCMC* MutableThis = const_cast<UTechdarkness_RealCMC*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Real(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}
	return ClientPredictionData;

		
}

void UTechdarkness_RealCMC::UpdateFromCompressedFlags(uint8 Flags) {
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void UTechdarkness_RealCMC::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) {
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	if (MovementMode == MOVE_Walking) {
		
		if (Safe_bWantsToSprint) {
			MaxWalkSpeed = Sprint_MaxWalkSpeed;
		}
		else {
			MaxWalkSpeed = Walk_MaxWalkSpeed;
		}
	}
}

UTechdarkness_RealCMC::UTechdarkness_RealCMC() {
}

void UTechdarkness_RealCMC::SprintPressed()
{
	Safe_bWantsToSprint = true;
}

void UTechdarkness_RealCMC::SprintReleased()
{
	Safe_bWantsToSprint = false;
}

UTechdarkness_RealCMC::FNetworkPredictionData_Client_Real::FNetworkPredictionData_Client_Real(const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement) {
	
}

FSavedMovePtr UTechdarkness_RealCMC::FNetworkPredictionData_Client_Real::AllocateNewMove() {
	return FSavedMovePtr(new FSavedMove_Real());
}
