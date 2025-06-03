#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Ladder.generated.h"
UCLASS()
class TECHDARKNESS_DEV_API ALadder : public AActor
{
    GENERATED_BODY()
public:
    ALadder();
    // Компонент статической меш-лестницы
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder")
    UStaticMeshComponent* LadderMesh;
    // Основной триггер для взаимодействия с лестницей (коллизия)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder")
    UBoxComponent* LadderTrigger;
    // Высота лестницы, настраивается в редакторе
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ladder")
    float LadderHeight = 400.f;
    // Возвращает нижнюю точку лестницы
    FVector GetLadderBottomPoint() const;
    // Возвращает верхнюю точку лестницы
    FVector GetLadderTopPoint() const;
protected:
    virtual void BeginPlay() override;
    // Обработчик входа персонажа в триггер лестницы
    UFUNCTION()
    void OnLadderBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    // Обработчик выхода персонажа из триггера лестницы
    UFUNCTION()
    void OnLadderEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};