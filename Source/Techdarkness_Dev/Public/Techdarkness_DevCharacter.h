#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Techdarkness_DevCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class ALadder;

UCLASS(config=Game)
class TECHDARKNESS_DEV_API ATechdarkness_DevCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ATechdarkness_DevCharacter();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
    UCameraComponent* FirstPersonCameraComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
    UInputMappingContext* DefaultMappingContext;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
    UInputAction* MoveAction;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
    UInputAction* LookAction;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
    UInputAction* ClimbAction;

    UPROPERTY(BlueprintReadOnly, Category="Anim")
    float LookPitch = 0.f;

    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);

    void TryClimb();
    void StopClimb();
    void StopClimbAndTeleport(const FVector& NewLocation);

    // Для отслеживания отпускания кнопки движения (на лестнице)
    void OnMoveActionReleased();

    bool bIsClimbing = false;
    UPROPERTY()
    ALadder* CurrentLadder = nullptr;
    bool bCanClimbLadder = false;
    float LastVerticalInput = 0.f;
};