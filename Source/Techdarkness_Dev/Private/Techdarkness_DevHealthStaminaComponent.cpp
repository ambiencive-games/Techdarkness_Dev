#include "Techdarkness_DevHealthStaminaComponent.h"

UTechdarkness_DevHealthStaminaComponent::UTechdarkness_DevHealthStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // не нужен Tick по умолчанию
}

void UTechdarkness_DevHealthStaminaComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina;
}

void UTechdarkness_DevHealthStaminaComponent::TakeDamage(float DamageAmount)
{
	if (CurrentHealth <= 0.f || DamageAmount <= 0.f) return;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.f, MaxHealth);
	if (CurrentHealth <= 0.f)
	{
		OnDeath.Broadcast();
	}
}

void UTechdarkness_DevHealthStaminaComponent::Heal(float HealAmount)
{
	if (CurrentHealth <= 0.f || HealAmount <= 0.f) return; // мертвого не лечим
	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.f, MaxHealth);
}

void UTechdarkness_DevHealthStaminaComponent::DrainStamina(float StaminaAmount)
{
	if (CurrentStamina <= 0.f || StaminaAmount <= 0.f) return;
	CurrentStamina = FMath::Clamp(CurrentStamina - StaminaAmount, 0.f, MaxStamina);
}

void UTechdarkness_DevHealthStaminaComponent::RestoreStamina(float StaminaAmount)
{
	if (CurrentStamina >= MaxStamina || StaminaAmount <= 0.f) return;
	CurrentStamina = FMath::Clamp(CurrentStamina + StaminaAmount, 0.f, MaxStamina);
}