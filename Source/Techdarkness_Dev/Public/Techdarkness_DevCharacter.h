#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Techdarkness_DevCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class ALadder;
class UTechdarkness_DevHealthStaminaComponent;

UENUM(BlueprintType)
enum class EActionState : uint8
{
    EAS_Unoccupied      UMETA(DisplayName = "Unoccupied"),
    EAS_Sprinting       UMETA(DisplayName = "Sprinting")
};

UCLASS(config=Game)
class TECHDARKNESS_DEV_API ATechdarkness_DevCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    ATechdarkness_DevCharacter(const FObjectInitializer& ObjectInitializer);

    // --- CAMERA ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
    UCameraComponent* FirstPersonCameraComponent;

    // --- STATS COMPONENT ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
    UTechdarkness_DevHealthStaminaComponent* HealthStaminaComponent;

    // --- INPUT ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
    UInputMappingContext* DefaultMappingContext;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
    UInputAction* MoveAction;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
    UInputAction* LookAction;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
    UInputAction* ClimbAction;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
    UInputAction* SlideAction;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
    UInputAction* SprintAction;

    UPROPERTY(BlueprintReadWrite, Category="Camera")
    float LookPitch = 0.f;

    /** Состояния для лестницы и хар-ки персонажа (оставь как было) */
    ALadder* CurrentLadder = nullptr;
    bool bIsClimbing = false;
    bool bCanClimbLadder = false;
    float LastVerticalInput = 0.f;

    // --- STATE ---
    EActionState CurrentActionState = EActionState::EAS_Unoccupied;
    float BaseSpeed = 500.f;

    void TryClimb();
    void StopClimb();
    void StopClimbAndTeleport(const FVector& NewLocation);

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
    virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void OnMoveActionReleased();



    // --- Для Enhanced Input, привязка к Sprint и Slide:
    UFUNCTION()
    void OnSlideStarted(const FInputActionInstance& Instance);
    UFUNCTION()
    void OnSlideReleased(const FInputActionInstance& Instance);
    UFUNCTION()
    void OnSprintStarted(const FInputActionInstance& Instance);
    UFUNCTION()
    void OnSprintReleased(const FInputActionInstance& Instance);

    // --- Весь input для скольжения и спринта теперь прокидываем в MovementComponent!
};