#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Techdarkness_RealCMC.generated.h"

// Кастомные режимы движения
UENUM(BlueprintType)
enum ECustomMovementMode {
    CMOVE_None UMETA(Hidden),
    CMOVE_Slide UMETA(DisplayName = "Slide"),
    CMOVE_MAX UMETA(Hidden)
};

UCLASS()
class TECHDARKNESS_DEV_API UTechdarkness_RealCMC : public UCharacterMovementComponent
{
    GENERATED_BODY()
public:
    UTechdarkness_RealCMC();

    // --- Sprint ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sprint", meta=(ToolTip="Скорость персонажа во время спринта."))
    float SprintSpeed = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sprint", meta=(ToolTip="Базовая скорость персонажа."))
    float WalkSpeed = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sprint", meta=(ToolTip="Скорость интерполяции ускорения при спринте."))
    float AccelerationInterpSpeed = 8.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sprint", meta=(ToolTip="Расход стамины во время спринта (в секунду)."))
    float StaminaDrainPerSecond = 15.f;

    FTimerHandle SprintTimerHandle;
    bool bWantsToSprint = false;

    // --- Slide ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Slide", meta=(ToolTip="Максимальная длительность скольжения."))
    float MaxSlideTime = 1.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Slide", meta=(ToolTip="Время перезарядки скольжения."))
    float SlideCooldown = 3.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Slide", meta=(ToolTip="Сила импульса при начале скольжения."))
    float SlideImpulse = 1700.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Slide", meta=(ToolTip="Сопротивление при скольжении (friction)."))
    float SlideFriction = 0.12f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Slide", meta=(ToolTip="Высота капсулы персонажа во время скольжения."))
    float SlideCapsuleHalfHeight = 48.f;

    float DefaultCapsuleHalfHeight = 96.f;
    float DefaultGroundFriction = 8.f;
    float DefaultBrakingFriction = 2.f;

    bool bIsSliding = false;
    float LastSlideEndTime = -100.f;
    bool bReadyForSlide = true;
    FTimerHandle SlideTimerHandle;

    UPROPERTY(Transient)
    class ATechdarkness_DevCharacter* OwningCharacter = nullptr; // Владелец

    // --- API для персонажа ---
    void Input_SlidePressed();
    void Input_SlideReleased();
    void Input_SprintPressed();
    void Input_SprintReleased();

    // --- Unreal Overrides ---
    virtual void InitializeComponent() override;
    virtual void BeginPlay() override;
    virtual bool IsMovingOnGround() const override;
    virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
    virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
    virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
    virtual void PhysCustom(float deltaTime, int32 Iterations) override;

    // Внутренняя логика
    void SprintStart();
    void SprintEnds();
    void SprintLoop();

    void StartSlide();
    void StopSlide();
    void EnterSlide();
    void ExitSlide();
    void PhysSlide(float deltaTime, int32 Iterations);
    bool GetSlideSurface(FHitResult& Hit) const;

    bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
};