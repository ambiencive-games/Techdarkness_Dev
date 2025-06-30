// Fill out your copyright notice in the Description page of Project Settings.

#include "TEST_ACTOR.h"
#include "Techdarkness_DevHealthStaminaComponent.h"
#include "Techdarkness_DevCharacter.h"

// Sets default values
ATEST_ACTOR::ATEST_ACTOR()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATEST_ACTOR::BeginPlay()
{
	Super::BeginPlay();

	// Подписка на событие начала оверлапа
	OnActorBeginOverlap.AddDynamic(this, &ATEST_ACTOR::OnOverlapBegin);

}

void ATEST_ACTOR::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATEST_ACTOR::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	float DamageAmount = 10.0f;

	if (ATechdarkness_DevCharacter* PlayerCharacter = Cast<ATechdarkness_DevCharacter>(OtherActor))
	{
		if (PlayerCharacter->HealthStaminaComponent)
		{
			PlayerCharacter->HealthStaminaComponent->TakeDamage(DamageAmount);
			UE_LOG(LogTemp, Warning, TEXT("Player took damage: %f"), DamageAmount);
			UE_LOG(LogTemp, Warning, TEXT("Player's current health: %f"), PlayerCharacter->HealthStaminaComponent->GetHealth());
		}
	}
}