#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Techdarkness_RealCMC.generated.h"

UENUM(BlueprintType)
enum ECustomMovementMode {
	CMOVE_None UMETA(Hidden),
	CMOVE_Slide UMETA(DisplayName = "Slide"),
	CMOVE_MAX UMETA(Hidden),
};

UCLASS()
class TECHDARKNESS_DEV_API UTechdarkness_RealCMC : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly) float Sprint_MaxWalkSpeed = 1000;
	UPROPERTY(EditDefaultsOnly) float Walk_MaxWalkSpeed = 500;
	UPROPERTY(EditDefaultsOnly) float Slide_MinSpeed = 600;
	UPROPERTY(EditDefaultsOnly) float Slide_EnterImpulse = 500;
	UPROPERTY(EditDefaultsOnly) float Slide_GravityForce = 5000;
	UPROPERTY(EditDefaultsOnly) float Slide_Friction = 0.8;

	UPROPERTY(Transient) class ATechdarkness_RealMoveCharacter* RealMoveCharacterOwner = nullptr;

	bool bWantsToSprint = false;
	bool bPrevWantsToCrouch = false;
	bool bCrouchPressed = false;
	bool bReadyForSlide = true;

public:
	UTechdarkness_RealCMC();

protected:
	virtual void InitializeComponent() override;

public:
	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;

protected:
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	void Parkour();

private:
	void EnterSlide();
	void ExitSlide();
	void PhysSlide(float deltaTime, int32 Iterations);
	bool GetSlideSurface(FHitResult& Hit) const;

public:
	UFUNCTION(BlueprintCallable) void SprintPressed();
	UFUNCTION(BlueprintCallable) void SprintReleased();
	UFUNCTION(BlueprintCallable) void CrouchPressed();
	UFUNCTION(BlueprintCallable) void CrouchReleased();
	UFUNCTION(BlueprintPure) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
};