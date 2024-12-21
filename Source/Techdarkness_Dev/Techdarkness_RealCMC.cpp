// Fill out your copyright notice in the Description page of Project Settings.
#include "Components/CapsuleComponent.h"
#include "Techdarkness_RealMoveCharacter.h"

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
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("clear"));
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
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("setmovefor"));
	Saved_bWantsToSprint = CharacterMovement->Safe_bWantsToSprint;
	if (CharacterMovement->Safe_bWantsToSprint) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("he want sprint"));
	}
	//Saved_bPrevWantsToCrouch = CharacterMovement->Safe_bPrevWantsToCrouch;
}

void UTechdarkness_RealCMC::FSavedMove_Real::PrepMoveFor(ACharacter* C) {
	Super::PrepMoveFor(C);

	UTechdarkness_RealCMC* CharacterMovement = Cast<UTechdarkness_RealCMC>(C->GetCharacterMovement());
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("prep move for"));
	//CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;
	//CharacterMovement->Safe_bPrevWantsToCrouch = Saved_bPrevWantsToCrouch;
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
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("upd comp flag"));
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
	Safe_bPrevWantsToCrouch = bWantsToCrouch;
}

bool UTechdarkness_RealCMC::IsMovingOnGround() const
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("is moving on ground check"));
	return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide);
}

bool UTechdarkness_RealCMC::CanCrouchInCurrentState() const
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("can crouch check"));
	return Super::CanCrouchInCurrentState() && IsMovingOnGround();
}

void UTechdarkness_RealCMC::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if (MovementMode == MOVE_Walking && bWantsToCrouch) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ch cr"));
		if (bReadyForSlide) {
			FHitResult PotentialSlideSurface;
			if (Velocity.SizeSquared() > pow(Slide_MinSpeed, 2) && GetSlideSurface(PotentialSlideSurface)) {
				bReadyForSlide = false;
				EnterSlide();
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("enterslide"));
			}
		}
	}

	if (IsCustomMovementMode(CMOVE_Slide) && !bWantsToCrouch) {
		ExitSlide();
	}
	if (bReadyForSlide == false) {
		if (bCrouchPressed == false) {
			bReadyForSlide = true;
		}
	}

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UTechdarkness_RealCMC::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("physcustom"));
	switch (CustomMovementMode) {
	case CMOVE_Slide:
		PhysSlide(deltaTime, Iterations);
		break;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"));
	}
}

UTechdarkness_RealCMC::UTechdarkness_RealCMC() {
}

void UTechdarkness_RealCMC::InitializeComponent()
{
	Super::InitializeComponent();

	RealMoveCharacterOwner = Cast<ATechdarkness_RealMoveCharacter>(GetOwner());
}

void UTechdarkness_RealCMC::EnterSlide()
{
	bWantsToCrouch = true;
	//Velocity += Velocity.GetSafeNormal2D() * Slide_EnterImpulse;
	SetMovementMode(MOVE_Custom, CMOVE_Slide);
}

void UTechdarkness_RealCMC::ExitSlide()
{
	bWantsToCrouch = bCrouchPressed;

	FQuat NewRotation = FRotationMatrix::MakeFromXZ(UpdatedComponent->GetForwardVector().GetSafeNormal2D(), FVector::UpVector).ToQuat();
	FHitResult Hit;
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, true, Hit);
	SetMovementMode(MOVE_Walking);
}

void UTechdarkness_RealCMC::PhysSlide(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME) {
		return;
	}

	RestorePreAdditiveRootMotionVelocity();

	FHitResult surfaceHit;
	if (!GetSlideSurface(surfaceHit) || Velocity.SizeSquared() < pow(Slide_MinSpeed, 2)) {
		ExitSlide();
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	Velocity += Slide_GravityForce * FVector::DownVector * deltaTime;

	if (FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector())) > .5) {
		Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector());
	}
	else {
		Acceleration = FVector::ZeroVector;
	}

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity()) {
		CalcVelocity(deltaTime, Slide_Friction, true, GetMaxBrakingDeceleration());
	}
	ApplyRootMotionToVelocity(deltaTime);

	Iterations++;
	bJustTeleported = false;

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FQuat OldRotation = UpdatedComponent->GetComponentRotation().Quaternion();
	FHitResult Hit(1.f);
	FVector Adjusted = Velocity * deltaTime;
	FVector VelPlaneDir = FVector::VectorPlaneProject(Velocity, surfaceHit.Normal).GetSafeNormal();
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(VelPlaneDir, surfaceHit.Normal).ToQuat();
	SafeMoveUpdatedComponent(Adjusted, NewRotation, true, Hit);

	if (Hit.Time < 1.f) {
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}

	FHitResult NewSurfaceHit;
	if (!GetSlideSurface(NewSurfaceHit) || Velocity.SizeSquared() < pow(Slide_MinSpeed, 2)) {
		ExitSlide();
	}

	if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity()) {
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}
}

bool UTechdarkness_RealCMC::GetSlideSurface(FHitResult& Hit) const
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("getslidesurf"));
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.f * FVector::DownVector;
	FName ProfileName = TEXT("BlockAll");
	return GetWorld()->LineTraceSingleByProfile(Hit, Start, End, ProfileName, RealMoveCharacterOwner->GetIgnoreCharacterParams());
}

void UTechdarkness_RealCMC::SprintPressed()
{
	Safe_bWantsToSprint = true;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("SPRINT"));
}

void UTechdarkness_RealCMC::SprintReleased()
{
	Safe_bWantsToSprint = false;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("unSPRINT"));
}

void UTechdarkness_RealCMC::ChrouchPressed()
{
	bWantsToCrouch = true;
	bCrouchPressed = true;
	//Super::Crouch();
}

void UTechdarkness_RealCMC::ChrouchReleased()
{
	bWantsToCrouch = false;
	bCrouchPressed = false;
	//Super::Crouch();
}

bool UTechdarkness_RealCMC::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

UTechdarkness_RealCMC::FNetworkPredictionData_Client_Real::FNetworkPredictionData_Client_Real(const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement) {
	
}

FSavedMovePtr UTechdarkness_RealCMC::FNetworkPredictionData_Client_Real::AllocateNewMove() {
	return FSavedMovePtr(new FSavedMove_Real());
}
