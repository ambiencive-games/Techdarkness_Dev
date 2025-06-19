// Fill out your copyright notice in the Description page of Project Settings.


#include "Techdarkness_DevHealthStaminaComponent.h"

// Sets default values for this component's properties
UTechdarkness_DevHealthStaminaComponent::UTechdarkness_DevHealthStaminaComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTechdarkness_DevHealthStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTechdarkness_DevHealthStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTechdarkness_DevHealthStaminaComponent::TakeDamage(float DamageAmount)
{
    if (CurrentHealth <= 0.f || DamageAmount <= 0.f)
        return;

    CurrentHealth -= DamageAmount;

    if (CurrentHealth <= 0.f)
    {
        CurrentHealth = 0.f;
        OnDeath.Broadcast();
    }
}

void UTechdarkness_DevHealthStaminaComponent::Heal(float HealAmount)
{
    if (CurrentHealth >= MaxHealth || HealAmount <= 0.f)
	{
        return;
	}
    CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.f, MaxHealth);
}

void UTechdarkness_DevHealthStaminaComponent::DrainStamina(float StaminaAmount)
{
	if (CurrentStamina <= 0.f || StaminaAmount <= 0.f)
	{
		return;
	}

	CurrentStamina = FMath::Clamp(CurrentStamina - StaminaAmount, 0.f, MaxStamina);
}

void UTechdarkness_DevHealthStaminaComponent::RestoreStamina(float StaminaAmount)
{
	if (CurrentStamina >= MaxStamina)
	{
		return;
	}

	CurrentStamina = FMath::Clamp(CurrentStamina + StaminaAmount, 0.f, MaxStamina);
}