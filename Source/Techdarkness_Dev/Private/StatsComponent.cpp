// Fill out your copyright notice in the Description page of Project Settings.


#include "StatsComponent.h"

// Sets default values for this component's properties
UStatsComponent::UStatsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UStatsComponent::TakeDamage(float DamageAmount)
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

void UStatsComponent::Heal(float HealAmount)
{
    if (CurrentHealth >= MaxHealth || HealAmount <= 0.f)
	{
        return;
	}
    CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.f, MaxHealth);
}

void UStatsComponent::DrainStamina(float StaminaAmount)
{
	if (CurrentStamina <= 0.f || StaminaAmount <= 0.f)
	{
		return;
	}

	CurrentStamina = FMath::Clamp(CurrentStamina - StaminaAmount, 0.f, MaxStamina);
}

void UStatsComponent::RestoreStamina(float StaminaAmount)
{
	if (CurrentStamina >= MaxStamina)
	{
		return;
	}

	CurrentStamina = FMath::Clamp(CurrentStamina + StaminaAmount, 0.f, MaxStamina);
}