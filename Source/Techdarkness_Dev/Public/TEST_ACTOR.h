// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TEST_ACTOR.generated.h"

class UStatsComponent;
class ATechdarkness_DevCharacter;

UCLASS()
class TECHDARKNESS_DEV_API ATEST_ACTOR : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATEST_ACTOR();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// ---- Функция оверлапа ----
	 UFUNCTION(BlueprintCallable, Category = "Overlap")
	void OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);

};
