// Techdarkness_DevCharacter.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Techdarkness_DevCharacter.generated.h"
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class ALadder;
/**
 * Класс персонажа для проекта Techdarkness.
 * Реализует управление, лазание по лестнице и скольжение (slide).
 */
UCLASS(config=Game)
class TECHDARKNESS_DEV_API ATechdarkness_DevCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    // Конструктор персонажа
    ATechdarkness_DevCharacter();
    // --- Камера ---
    /** Компонент камеры от первого лица */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
    UCameraComponent* FirstPersonCameraComponent;
    // --- Input Mapping ---
    /** Контекст для настроек управления */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
    UInputMappingContext* DefaultMappingContext;
    /** Экшен передвижения */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
    UInputAction* MoveAction;
    /** Экшен обзора */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
    UInputAction* LookAction;
    /** Экшен попытки залезть по лестнице */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
    UInputAction* ClimbAction;
    /** Экшен скольжения */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
    UInputAction* SlideAction;
    // --- Значение Pitch камеры, для анимаций ---
    UPROPERTY(BlueprintReadOnly, Category="Anim")
    float LookPitch = 0.f;
    // --- Переопределённые функции ---
    /** Вызывается при начале игры */
    virtual void BeginPlay() override;
    /** Сопряжение input'ов с функциями */
    virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
    /** Логика движения (ходьба/лазание) */
    void Move(const FInputActionValue& Value);
    /** Логика обзора */
    void Look(const FInputActionValue& Value);
    /** Попытка начать лазание */
    void TryClimb();
    /** Прекратить лазание */
    void StopClimb();
    /** Прекратить лазание и телепортироваться */
    void StopClimbAndTeleport(const FVector& NewLocation);
    /** Сброс скорости при отпускании клавиши хождения, если персонаж лазит */
    void OnMoveActionReleased();
    // --- Slide System (Скольжение) ---
    /** Макс. длительность скольжения */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slide")
    float MaxSlideTime = 1.1f;
    /** КД на повторное скольжение */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slide")
    float SlideCooldown = 0.3f;
    /** Импульс при начале скольжения */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slide")
    float SlideImpulse = 1700.f;
    /** Фрикция во время скольжения */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slide")
    float SlideFriction = 0.12f;
    /** Высота капсулы при скольжении */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slide")
    float SlideCapsuleHalfHeight = 48.f;
    /** Обычная высота капсулы */
    UPROPERTY(EditDefaultsOnly, Category="Slide")
    float DefaultCapsuleHalfHeight = 96.f;
    /** Обычная фрикция на земле */
    UPROPERTY(EditDefaultsOnly, Category="Slide")
    float DefaultGroundFriction = 8.f;
    /** Обычный коэффициент торможения */
    UPROPERTY(EditDefaultsOnly, Category="Slide")
    float DefaultBrakingFriction = 2.f;
    /** Флаг: персонаж скользит */
    bool bIsSliding = false;
    /** Последнее время окончания скольжения */
    float LastSlideEndTime = -100.f;
    /** Таймер для скольжения */
    FTimerHandle SlideTimerHandle;
    /** Начать скольжение */
    void StartSlide();
    /** Прервать скольжение */
    void StopSlide();
    /** Выйти из скольжения по отпущенной кнопке */
    void OnSlideReleased();
    /** Обработка смены режима движения */
    virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
    // --- Ladder System (Лестницы) ---
    /** Флаг: в состоянии лазания */
    bool bIsClimbing = false;
    /** Указатель на текущую лестницу */
    UPROPERTY()
    ALadder* CurrentLadder = nullptr;
    /** Можно ли начать лазать по нынешней лестнице */
    bool bCanClimbLadder = false;
    /** Последнее вертикальное значение input'а */
    float LastVerticalInput = 0.f;
};