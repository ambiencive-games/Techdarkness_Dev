#include "Techdarkness_RealCMC.h"
#include "Techdarkness_RealMoveCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

UTechdarkness_RealCMC::UTechdarkness_RealCMC() {}

void UTechdarkness_RealCMC::InitializeComponent()
{
	Super::InitializeComponent();
	RealMoveCharacterOwner = Cast<ATechdarkness_RealMoveCharacter>(GetOwner());
}

bool UTechdarkness_RealCMC::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide);
}

bool UTechdarkness_RealCMC::CanCrouchInCurrentState() const
{
	return Super::CanCrouchInCurrentState() && IsMovingOnGround();
}

void UTechdarkness_RealCMC::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if (MovementMode == MOVE_Walking) {
		MaxWalkSpeed = bWantsToSprint ? Sprint_MaxWalkSpeed : Walk_MaxWalkSpeed;
	}

	bPrevWantsToCrouch = bWantsToCrouch;
	Parkour();
}

void UTechdarkness_RealCMC::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if (MovementMode == MOVE_Walking && bWantsToCrouch) {
		if (bReadyForSlide) {
			FHitResult PotentialSlideSurface;
			if (Velocity.SizeSquared() > FMath::Square(Slide_MinSpeed) && GetSlideSurface(PotentialSlideSurface)) {
				bReadyForSlide = false;
				EnterSlide();
			}
		}
	}

	if (IsCustomMovementMode(CMOVE_Slide) && !bWantsToCrouch) {
		ExitSlide();
	}

	if (!bReadyForSlide && !bCrouchPressed) {
		bReadyForSlide = true;
	}

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UTechdarkness_RealCMC::PhysCustom(float deltaTime, int32 Iterations)
{
	switch (CustomMovementMode) {
	case CMOVE_Slide:
		PhysSlide(deltaTime, Iterations);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Invalid custom movement mode"));
		break;
	}
}

void UTechdarkness_RealCMC::Parkour()
{
	FHitResult Hit;
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start + (CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius() + 1.f) * CharacterOwner->GetActorForwardVector();
	FName ProfileName = TEXT("BlockAll");

#if WITH_EDITOR
	DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 1.f, 0.f, 2.f);
#endif

	GetWorld()->LineTraceSingleByProfile(Hit, Start, End, ProfileName, RealMoveCharacterOwner->GetIgnoreCharacterParams());

	if (!Hit.bBlockingHit)
		return;

	TArray<FHitResult, TSizedDefaultAllocator<32>> HitUp;
	FVector StartUp = Hit.Location;
	float heightParkour = 200.f;
	FVector EndUp = StartUp + FVector::UpVector * heightParkour;

	GetWorld()->LineTraceMultiByProfile(HitUp, EndUp, StartUp, ProfileName, RealMoveCharacterOwner->GetIgnoreCharacterParams());

#if WITH_EDITOR
	DrawDebugLine(GetWorld(), EndUp, StartUp, FColor::Yellow, false, 1.f, 0.f, 2.f);
#endif

	for (FHitResult h : HitUp) {
		if (h.bBlockingHit) {
#if WITH_EDITOR
			DrawDebugSphere(GetWorld(), h.Location, 10, 10, FColor::Orange, 0.f, 2.f);
#endif
			const FVector TeleportLocation = h.Location + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * FVector::UpVector;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(CharacterOwner);

			bool bHasOverlap = GetWorld()->OverlapAnyTestByChannel(
				TeleportLocation,
				FQuat::Identity,
				ECC_Pawn,
				FCollisionShape::MakeCapsule(
					CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius(),
					CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()
				),
				Params);

			if (!bHasOverlap) {
				CharacterOwner->TeleportTo(TeleportLocation, CharacterOwner->GetActorRotation());
			}
		}
	}
}

void UTechdarkness_RealCMC::EnterSlide()
{
	bWantsToCrouch = true;
	Velocity += Velocity.GetSafeNormal2D() * Slide_EnterImpulse; // вернул импульс для старта
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
	if (!GetSlideSurface(surfaceHit) || Velocity.SizeSquared() < FMath::Square(Slide_MinSpeed)) {
		ExitSlide();
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	Velocity += Slide_GravityForce * FVector::DownVector * deltaTime;

	if (FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector())) > 0.5f) {
		Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector());
	} else {
		Acceleration = FVector::ZeroVector;
	}

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity()) {
		CalcVelocity(deltaTime, Slide_Friction, true, GetMaxBrakingDeceleration());
	}

	ApplyRootMotionToVelocity(deltaTime);

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FQuat OldRotation = UpdatedComponent->GetComponentRotation().Quaternion();

	FHitResult Hit(1.f);
	FVector Adjusted = Velocity * deltaTime;
	FVector VelPlaneDir = FVector::VectorPlaneProject(Velocity, surfaceHit.Normal).GetSafeNormal();
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(VelPlaneDir, surfaceHit.Normal).ToQuat();
	SafeMoveUpdatedComponent(Adjusted, NewRotation, true, Hit);

	if (Hit.Time < 1.f) {
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, 1.f - Hit.Time, Hit.Normal, Hit, true);
	}

	FHitResult NewSurfaceHit;
	if (!GetSlideSurface(NewSurfaceHit) || Velocity.SizeSquared() < FMath::Square(Slide_MinSpeed)) {
		ExitSlide();
	}

	if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity()) {
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}
}

bool UTechdarkness_RealCMC::GetSlideSurface(FHitResult& Hit) const
{
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f * FVector::DownVector;
	FName ProfileName = TEXT("BlockAll");

	return GetWorld()->LineTraceSingleByProfile(Hit, Start, End, ProfileName, RealMoveCharacterOwner ? RealMoveCharacterOwner->GetIgnoreCharacterParams() : FCollisionQueryParams());
}

void UTechdarkness_RealCMC::SprintPressed()
{
	bWantsToSprint = true;
}

void UTechdarkness_RealCMC::SprintReleased()
{
	bWantsToSprint = false;
}

void UTechdarkness_RealCMC::CrouchPressed()
{
	bWantsToCrouch = true;
	bCrouchPressed = true;
}

void UTechdarkness_RealCMC::CrouchReleased()
{
	bWantsToCrouch = false;
	bCrouchPressed = false;
}

bool UTechdarkness_RealCMC::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}