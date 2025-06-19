// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Techdarkness_DevHealthStaminaComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TECHDARKNESS_DEV_API UTechdarkness_DevHealthStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTechdarkness_DevHealthStaminaComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Система здоровья ---
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health")
    float MaxHealth = 100.0f;

    float CurrentHealth;

    UFUNCTION(BlueprintCallable, Category="Health")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category="Health")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category="Health")
    float GetHealth() const { return CurrentHealth; }

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
    UPROPERTY(BlueprintAssignable, Category="Health")
    FOnDeath OnDeath; // Функция, вызываемая при смерти или разрушении чего-то

    // --- Система выносливости ---
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stamina")
    float MaxStamina = 100.0f;

    float CurrentStamina;

    UFUNCTION(BlueprintCallable, Category="Stamina")
    void DrainStamina(float StaminaAmount);

    UFUNCTION(BlueprintCallable, Category="Stamina")
    void RestoreStamina(float StaminaAmount);

    UFUNCTION(BlueprintCallable, Category="Health")
    float GetStamina() const { return CurrentStamina; }

    UFUNCTION(BlueprintCallable, Category="Stamina")
    bool IsStaminaFull() const { return CurrentStamina >= MaxStamina; }
};
