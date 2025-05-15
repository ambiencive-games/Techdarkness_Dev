// Ladder.cpp
#include "Ladder.h"
#include "Components/StaticMeshComponent.h"
#include "Techdarkness_DevCharacter.h"
ALadder::ALadder()
{
    PrimaryActorTick.bCanEverTick = false;
    // Создаём компонент меша лестницы (визуализация)
    LadderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LadderMesh"));
    RootComponent = LadderMesh;
    LadderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    // Создаём box trigger для коллизии
    LadderTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("LadderTrigger"));
    LadderTrigger->SetupAttachment(RootComponent);
    LadderTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // Только для оверлапа
    LadderTrigger->SetCollisionObjectType(ECC_WorldDynamic);
    LadderTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    LadderTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // Только Pawn
    LadderTrigger->SetGenerateOverlapEvents(true);
    // Устанавливаем размеры box триггера (ширина, глубина, половина высоты лестницы)
    LadderTrigger->SetBoxExtent(FVector(50, 50, LadderHeight * 0.5f));
    // Сдвигаем вверх box так, чтобы низ совпадал с низом лестницы
    LadderTrigger->SetRelativeLocation(FVector(0, 0, LadderHeight * 0.5f));
}
void ALadder::BeginPlay()
{
    Super::BeginPlay();
    // Подписка на события входа и выхода из триггера
    LadderTrigger->OnComponentBeginOverlap.AddDynamic(this, &ALadder::OnLadderBeginOverlap);
    LadderTrigger->OnComponentEndOverlap.AddDynamic(this, &ALadder::OnLadderEndOverlap);
}
// Обработчик, когда персонаж входит в область триггера лестницы
void ALadder::OnLadderBeginOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    // Если это наш персонаж — разрешаем ему забираться на лестницу
    if (ATechdarkness_DevCharacter* Char = Cast<ATechdarkness_DevCharacter>(OtherActor))
    {
        Char->bCanClimbLadder = true;
        Char->CurrentLadder = this;
    }
}
// Обработчик, когда персонаж выходит из области триггера
void ALadder::OnLadderEndOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32)
{
    // Если это наш персонаж — сбрасываем параметры лестницы у него
    if (ATechdarkness_DevCharacter* Char = Cast<ATechdarkness_DevCharacter>(OtherActor))
    {
        Char->bCanClimbLadder = false;
        Char->CurrentLadder = nullptr;
        Char->StopClimb();
    }
}
// Нижняя точка лестницы (позиция актора)
FVector ALadder::GetLadderBottomPoint() const
{
    return GetActorLocation();
}
// Верхняя точка лестницы (позиция актора + высота по Z)
FVector ALadder::GetLadderTopPoint() const
{
    return GetActorLocation() + FVector(0, 0, LadderHeight);
}