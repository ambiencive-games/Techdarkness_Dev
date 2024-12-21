// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Techdarkness_RealMoveCharacter.h"
#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Techdarkness_RealCMC.generated.h"

// Made with this guide https://youtu.be/17D4SzewYZ0

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

	class FSavedMove_Real : public FSavedMove_Character {
		typedef FSavedMove_Character Super;

		uint8 Saved_bWantsToSprint : 1;

		uint8 Saved_bPrevWantsToCrouch : 1;

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FNetworkPredictionData_Client_Real : public FNetworkPredictionData_Client_Character {
	public:
		FNetworkPredictionData_Client_Real(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};

	UPROPERTY(EditDefaultsOnly) float Sprint_MaxWalkSpeed = 1000;
	UPROPERTY(EditDefaultsOnly) float Walk_MaxWalkSpeed = 500;

	UPROPERTY(EditDefaultsOnly) float Slide_MinSpeed = 600;
	UPROPERTY(EditDefaultsOnly) float Slide_EnterImpulse = 500;
	UPROPERTY(EditDefaultsOnly) float Slide_GravityForce = 5000;
	UPROPERTY(EditDefaultsOnly) float Slide_Friction = 0.8;

	UPROPERTY(Transient) ATechdarkness_RealMoveCharacter* RealMoveCharacterOwner;

	bool Safe_bWantsToSprint;
	bool Safe_bPrevWantsToCrouch;

	bool bCrouchPressed = false;
	bool bReadyForSlide = true;

public:
	UTechdarkness_RealCMC();

protected:
	virtual void InitializeComponent() override;

public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;
protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

private:
	void EnterSlide();
	void ExitSlide();
	void PhysSlide(float deltaTime, int32 Iterations);
	bool GetSlideSurface(FHitResult& Hit) const;
	

public:
	UFUNCTION(BlueprintCallable) void SprintPressed();
	UFUNCTION(BlueprintCallable) void SprintReleased();
	UFUNCTION(BlueprintCallable) void ChrouchPressed();
	UFUNCTION(BlueprintCallable) void ChrouchReleased();
	UFUNCTION(BlueprintPure) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
	
};
