#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Ladder.generated.h"

/**
 * Актор лестницы для взаимодействия с персонажем.
 */
UCLASS()
class TECHDARKNESS_DEV_API ALadder : public AActor
{
    GENERATED_BODY()

public:
    ALadder();

    /** Статическая меш-лестница. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder", meta=(ToolTip="Меш лестницы, используемый для визуализации."))
    UStaticMeshComponent* LadderMesh;

    /** Боксовый коллайдер для взаимодействия. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder", meta=(ToolTip="Box Collision для определения области взаимодействия с лестницей."))
    UBoxComponent* LadderTrigger;

    /** Высота лестницы. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder", meta=(ToolTip="Высота лестницы в сантиметрах."))
    float LadderHeight = 400.f;

    /** Возвращает координаты нижней точки лестницы. */
    FVector GetLadderBottomPoint() const;

    /** Возвращает координаты верхней точки лестницы. */
    FVector GetLadderTopPoint() const;

protected:
    virtual void BeginPlay() override;

    /** Обработчик входа персонажа в триггер лестницы. */
    UFUNCTION()
    void OnLadderBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /** Обработчик выхода персонажа из триггера лестницы. */
    UFUNCTION()
    void OnLadderEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};