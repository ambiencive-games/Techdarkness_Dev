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

/**
 * Перечисление состояний действия персонажа.
 */
UENUM(BlueprintType)
enum class EActionState : uint8
{
    EAS_Unoccupied      UMETA(DisplayName = "Unoccupied"),
    EAS_Sprinting       UMETA(DisplayName = "Sprinting")
};

/**
 * Главный игровой персонаж.
 */
UCLASS(config=Game)
class TECHDARKNESS_DEV_API ATechdarkness_DevCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ATechdarkness_DevCharacter();

    // --- CAMERA ---

    /** Компонент камеры от первого лица. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera, meta=(ToolTip="Компонент камеры от первого лица."))
    UCameraComponent* FirstPersonCameraComponent;

    // --- STATS COMPONENT ---

    /** Компонент управления здоровьем и стаминой. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats", meta=(AllowPrivateAccess="true", ToolTip="Компонент управления здоровьем и стаминой персонажа."))
    UTechdarkness_DevHealthStaminaComponent* HealthStaminaComponent;

    // --- INPUT ---

    /** Контекст привязки управления. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input, meta=(ToolTip="Контекст привязки управления."))
    UInputMappingContext* DefaultMappingContext;

    /** Действие движения. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input, meta=(ToolTip="Input Action для движения."))
    UInputAction* MoveAction;

    /** Действие поворота камеры. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input, meta=(ToolTip="Input Action для обзора/поворота камеры."))
    UInputAction* LookAction;

    /** Действие для лазающей лестницы. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input, meta=(ToolTip="Действие для начала лазания по лестнице."))
    UInputAction* ClimbAction;

    /** Действие для скольжения. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input, meta=(ToolTip="Действие для начала скольжения."))
    UInputAction* SlideAction;

    /** Действие для бега. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input, meta=(ToolTip="Действие для начала спринта."))
    UInputAction* SprintAction;

    /** Pitch камеры. */
    UPROPERTY(BlueprintReadOnly, Category="Anim", meta=(ToolTip="Pitch (наклон) камеры для анимации."))
    float LookPitch = 0.f;

    // --- LADDER ---

    /** Активная лестница для взаимодействия. */
    UPROPERTY(meta=(ToolTip="Указатель на текущую лестницу, с которой взаимодействует персонаж."))
    ALadder* CurrentLadder = nullptr;

    /** Флаг: персонаж лезет по лестнице. */
    bool bIsClimbing = false;

    /** Флаг: персонаж может залезть на лестницу. */
    bool bCanClimbLadder = false;

    /** Последнее вертикальное направление движения (лестница). */
    float LastVerticalInput = 0.f;

    // --- SLIDE ---

    /** Максимальное время скольжения. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slide", meta=(ToolTip="Максимальная длительность скольжения."))
    float MaxSlideTime = 1.1f;

    /** Время перезарядки скольжения. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slide", meta=(ToolTip="Время перезарядки скольжения."))
    float SlideCooldown = 0.3f;

    /** Импульс, с которым начинается скольжение. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slide", meta=(ToolTip="Сила импульса при начале скольжения."))
    float SlideImpulse = 1700.f;

    /** Сопротивление (трение) при скольжении. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slide", meta=(ToolTip="Сопротивление при скольжении (friction)."))
    float SlideFriction = 0.12f;

    /** Высота капсулы во время скольжения. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slide", meta=(ToolTip="Высота капсулы персонажа во время скольжения."))
    float SlideCapsuleHalfHeight = 48.f;

    /** Обычная высота капсулы персонажа. */
    float DefaultCapsuleHalfHeight = 96.f;

    /** Обычное трение земли. */
    float DefaultGroundFriction = 8.f;

    /** Обычное тормозное трение. */
    float DefaultBrakingFriction = 2.f;

    /** Флаг: персонаж находится в состоянии скольжения. */
    bool bIsSliding = false;

    /** Время окончания последнего скольжения. */
    float LastSlideEndTime = -100.f;

    /** Таймер скольжения. */
    FTimerHandle SlideTimerHandle;

    // --- SPRINT ---

    /** Запустить спринт персонажа. */
    UFUNCTION(BlueprintCallable, Category="Sprint", meta=(ToolTip="Запустить спринт персонажа (увеличение скорости и дренаж стамины)."))
    void SprintStart();

    /** Остановить спринт персонажа. */
    UFUNCTION(BlueprintCallable, Category="Sprint", meta=(ToolTip="Закончить спринт персонажа."))
    void SprintEnds();

    /** Функция обновления спринта (в цикле). */
    UFUNCTION(BlueprintCallable, Category="Sprint", meta=(ToolTip="Обновлять параметры во время спринта (дренаж стамины и скорость)."))
    void SprintLoop();

    /** Скорость спринта персонажа. */
    UPROPERTY(BlueprintReadWrite, Category="Sprint", meta=(ToolTip="Скорость персонажа во время спринта."))
    float SprintSpeed = 1200.f;

    /** Скорость интерполяции ускорения при спринте. */
    UPROPERTY(BlueprintReadWrite, Category="Sprint", meta=(ToolTip="Скорость интерполяции ускорения для плавного старта спринта."))
    float AccelerationInterpSpeed = 8.f;

    /** Расход стамины в секунду во время спринта. */
    UPROPERTY(BlueprintReadWrite, Category="Sprint", meta=(ToolTip="Расход стамины во время спринта (в секунду)."))
    float StaminaDrainPerSecond = 15.f;

    /** Таймер спринта. */
    FTimerHandle SprintTimerHandle;

    /** Базовая скорость персонажа (до изменений). */
    float BaseSpeed = 0.f;

    // --- STAMINA ---

    /** Циклическое восстановление стамины start. */
    UFUNCTION(BlueprintCallable, Category="Stats", meta=(ToolTip="Начать циклическое восстановление стамины."))
    void RestoreStaminaLoop();

    /** Начать восстановление стамины. */
    UFUNCTION(BlueprintCallable, Category="Stats", meta=(ToolTip="Начать восстановление стамины для персонажа."))
    void RestoreStaminaStart();

    /** Завершить восстановление стамины. */
    UFUNCTION(BlueprintCallable, Category="Stats", meta=(ToolTip="Завершить процесс восстановления стамины."))
    void RestoreStaminaEnd();

    /** Скорость восстановления стамины. */
    UPROPERTY(EditDefaultsOnly, Category="Stats", meta=(ToolTip="Скорость восстановления стамины (единиц в секунду)."))
    float RestoreStaminaRate = 500.f; 

    /** Таймер восстановления стамины. */
    FTimerHandle StaminaRegenTimerHandle;

    // --- STATE ---

    /** Текущее состояние действия персонажа. */
    UPROPERTY(meta=(ToolTip="Текущее состояние действия персонажа (например, спринт, бездействие)."))
    EActionState CurrentActionState = EActionState::EAS_Unoccupied;

    // --- LADDER CLIMB ---

    /** Попытаться начать лазанье по лестнице. */
    void TryClimb();

    /** Остановить лазанье по лестнице. */
    void StopClimb();

    /** Остановить лазанье по лестнице и телепортировать персонажа. */
    void StopClimbAndTeleport(const FVector& NewLocation);

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
    virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

    // --- MOVEMENT ---

    /** Перемещение персонажа. */
    void Move(const FInputActionValue& Value);

    /** Вращение камеры (обзор). */
    void Look(const FInputActionValue& Value);

    /** Обработчик отпускания кнопки движения. */
    void OnMoveActionReleased();

    // --- SLIDE ---

    /** Начать скольжение. */
    void StartSlide();

    /** Остановить скольжение. */
    void StopSlide();

    /** Обработчик отпускания кнопки скольжения. */
    void OnSlideReleased();

};