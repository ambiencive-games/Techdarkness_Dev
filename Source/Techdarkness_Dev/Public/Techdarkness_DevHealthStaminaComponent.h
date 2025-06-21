#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Techdarkness_DevHealthStaminaComponent.generated.h"

// Делегат, вызываемый при смерти персонажа.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

/**
 * Компонент управления здоровьем и стаминой персонажа.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TECHDARKNESS_DEV_API UTechdarkness_DevHealthStaminaComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTechdarkness_DevHealthStaminaComponent();

protected:
    virtual void BeginPlay() override;

public:
    // --- Здоровье ---

    /** Максимальное здоровье персонажа. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health", meta=(ToolTip="Максимальное здоровье персонажа."))
    float MaxHealth = 100.0f;

    /** Текущее здоровье персонажа. */
    UPROPERTY(BlueprintReadOnly, Category="Health", meta=(ToolTip="Текущее здоровье персонажа."))
    float CurrentHealth = 0.0f;

    /** Наносит урон персонажу. */
    UFUNCTION(BlueprintCallable, Category="Health", meta=(ToolTip="Наносит урон персонажу на заданное значение."))
    void TakeDamage(float DamageAmount);

    /** Лечит персонажа на указанное значение. */
    UFUNCTION(BlueprintCallable, Category="Health", meta=(ToolTip="Лечит персонажа на заданное количество здоровья."))
    void Heal(float HealAmount);

    /** Возвращает текущее здоровье персонажа. */
    UFUNCTION(BlueprintCallable, Category="Health", meta=(ToolTip="Возвращает текущее здоровье персонажа."))
    float GetHealth() const { return CurrentHealth; }

    /** Делегат, вызываемый при смерти персонажа. */
    UPROPERTY(BlueprintAssignable, Category="Health", meta=(ToolTip="Делегат, вызываемый при смерти персонажа."))
    FOnDeath OnDeath;

    // --- Стамина ---

    /** Максимальная стамина персонажа. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stamina", meta=(ToolTip="Максимальный запас стамины персонажа."))
    float MaxStamina = 100.0f;

    /** Текущий запас стамины. */
    UPROPERTY(BlueprintReadOnly, Category="Stamina", meta=(ToolTip="Текущее количество стамины у персонажа."))
    float CurrentStamina = 0.0f;

    /** Снижает запас стамины на указанную величину. */
    UFUNCTION(BlueprintCallable, Category="Stamina", meta=(ToolTip="Снижает запас стамины на указанное значение."))
    void DrainStamina(float StaminaAmount);

    /** Восстанавливает запас стамины на указанную величину. */
    UFUNCTION(BlueprintCallable, Category="Stamina", meta=(ToolTip="Восстанавливает запас стамины на указанное значение."))
    void RestoreStamina(float StaminaAmount);

    /** Возвращает текущий запас стамины. */
    UFUNCTION(BlueprintCallable, Category="Stamina", meta=(ToolTip="Возвращает текущее значение стамины."))
    float GetStamina() const { return CurrentStamina; }

    /** Возвращает true, если стамина полная. */
    UFUNCTION(BlueprintCallable, Category="Stamina", meta=(ToolTip="Проверяет, заполнена ли стамина полностью."))
    bool IsStaminaFull() const { return CurrentStamina >= MaxStamina; }
};