// Techdarkness_DevCharacter.cpp
#include "Techdarkness_DevCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Ladder.h"
#include "DrawDebugHelpers.h"
#include "Techdarkness_DevHealthStaminaComponent.h"
#include "TimerManager.h"

ATechdarkness_DevCharacter::ATechdarkness_DevCharacter()
{
    // Инициализация капсулы персонажа
    GetCapsuleComponent()->InitCapsuleSize(35.f, DefaultCapsuleHalfHeight);
    // Настройка меша
    GetMesh()->SetupAttachment(GetCapsuleComponent());
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -DefaultCapsuleHalfHeight));
    GetMesh()->SetOwnerNoSee(false);
    GetMesh()->SetOnlyOwnerSee(false);
    GetMesh()->CastShadow = true;
    GetMesh()->bCastDynamicShadow = true;
    GetMesh()->bCastHiddenShadow = true;
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    // Создание камеры первого лица
    FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCameraComponent->SetupAttachment(GetMesh(), TEXT("CameraSocket"));
    FirstPersonCameraComponent->bUsePawnControlRotation = true;
    // Инициализация флагов и указателей для лестницы
    bIsClimbing = false;
    bCanClimbLadder = false;
    CurrentLadder = nullptr;
    LastVerticalInput = 0.f;
    // Отключаем тик, не требуется
    PrimaryActorTick.bCanEverTick = false;

    Techdarkness_DevHealthStaminaComponent = CreateDefaultSubobject<UTechdarkness_DevHealthStaminaComponent>(TEXT("Techdarkness_DevHealthStaminaComponent"));
    Techdarkness_DevHealthStaminaComponent->GetHealth();
    Techdarkness_DevHealthStaminaComponent->MaxHealth = 100.0f; 
    Techdarkness_DevHealthStaminaComponent->CurrentHealth = Techdarkness_DevHealthStaminaComponent->MaxHealth;
    Techdarkness_DevHealthStaminaComponent->MaxStamina = 100.0f;
    Techdarkness_DevHealthStaminaComponent->CurrentStamina = Techdarkness_DevHealthStaminaComponent->MaxStamina;
}

void ATechdarkness_DevCharacter::BeginPlay()
{
    Super::BeginPlay();
    // Сохраняем стандартные параметры коллизии и трения
    DefaultCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
    DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
    DefaultBrakingFriction = GetCharacterMovement()->BrakingFrictionFactor;

    BaseSpeed = GetCharacterMovement()->MaxWalkSpeed;

    // Добавляем контекст управления (input mapping)
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                if (DefaultMappingContext)
                {
                    InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
                }
            }
        }
    }
}

void ATechdarkness_DevCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    check(PlayerInputComponent);
    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Биндим движения
        if (MoveAction)
        {
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATechdarkness_DevCharacter::Move);
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &ATechdarkness_DevCharacter::OnMoveActionReleased);
        }
        // Биндим камеру
        if (LookAction)
            EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATechdarkness_DevCharacter::Look);
        // Биндим полезание по лестнице
        if (ClimbAction)
            EnhancedInput->BindAction(ClimbAction, ETriggerEvent::Started, this, &ATechdarkness_DevCharacter::TryClimb);
        // Биндим скольжение (slide)
        if (SlideAction)
        {
            EnhancedInput->BindAction(SlideAction, ETriggerEvent::Started, this, &ATechdarkness_DevCharacter::StartSlide);
            EnhancedInput->BindAction(SlideAction, ETriggerEvent::Completed, this, &ATechdarkness_DevCharacter::OnSlideReleased);
        }
        if(SprintAction)
        {
            EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ATechdarkness_DevCharacter::SprintStart);
            EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATechdarkness_DevCharacter::SprintEnds);
        }
    }
}

void ATechdarkness_DevCharacter::OnMoveActionReleased()
{
    // Если персонаж лазает по лестнице и отпускает движение, он останавливается
    if (bIsClimbing && GetCharacterMovement())
    {
        GetCharacterMovement()->Velocity = FVector::ZeroVector;
        LastVerticalInput = 0.f;
    }
}

void ATechdarkness_DevCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    if (!Controller)
        return;
    float VerticalInput = MovementVector.Y;
    LastVerticalInput = VerticalInput;
    if (bIsClimbing && CurrentLadder)
    {
        // --- Логика лазания по лестнице ---
        FVector LadderDir = (CurrentLadder->GetLadderTopPoint() - CurrentLadder->GetLadderBottomPoint()).GetSafeNormal();
        FVector Start = CurrentLadder->GetLadderBottomPoint();
        FVector End = CurrentLadder->GetLadderTopPoint();
        FVector Current = GetActorLocation();
        float T = FVector::DotProduct(Current - Start, LadderDir);
        float Length = FVector::Dist(Start, End);
        float ExitThreshold = 20.f;
        float ClimbSpeed = 150.f;
        float Delta = GetWorld()->GetDeltaSeconds();
        float Offset = GetCapsuleComponent()->GetUnscaledCapsuleRadius() + 1.f;
        FVector LadderForward = CurrentLadder->GetActorForwardVector();
        float NextT = T + VerticalInput * ClimbSpeed * Delta;
        // Проверка выхода вниз с лестницы
        if (NextT < 0.f)
        {
            StopClimb();
            FVector ExitPoint = Start - LadderForward * (Offset + 10.f) - FVector(0,0,50.f);
            SetActorLocation(ExitPoint, true);
            bCanClimbLadder = false;
            CurrentLadder = nullptr;
            return;
        }
        // Проверка выхода наверх
        if (NextT > Length - ExitThreshold)
        {
            StopClimb();
            // Проверяем, есть ли платформа наверху
            FVector CapsuleTop = End + LadderForward * (Offset + 20.f) + FVector(0,0,50.f);
            FVector TraceStart = CapsuleTop;
            FVector TraceEnd = CapsuleTop - FVector(0,0,120.f);
            FHitResult HitResult;
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(this);
            bool bFoundPlatform = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);
            if (bFoundPlatform)
            {
                // Ставим персонажа на найденную платформу
                FVector PlaceOnTop = HitResult.ImpactPoint 
                    + LadderForward * (Offset + 5.f)
                    + FVector(0, 0, GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() + 2.f);
                SetActorLocation(PlaceOnTop, true);
            }
            else
            {
                // Нет платформы — ставим чуть выше лестницы
                FVector ExitPoint = End + LadderForward * (Offset + 20.f) + FVector(0,0,60.f);
                SetActorLocation(ExitPoint, true);
            }
            bCanClimbLadder = false;
            CurrentLadder = nullptr;
            return;
        }
        // Движение по лестнице
        GetCharacterMovement()->Velocity = LadderDir * VerticalInput * ClimbSpeed;
        // Если нет вертикального input'а — персонаж замирает
        if (FMath::Abs(VerticalInput) < 0.1f)
        {
            GetCharacterMovement()->Velocity = FVector::ZeroVector;
        }
    }
    else
    {
        // --- Обычное передвижение по полу ---
        AddMovementInput(GetActorForwardVector(), MovementVector.Y);
        AddMovementInput(GetActorRightVector(), MovementVector.X);
    }
}

void ATechdarkness_DevCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    if (Controller && !bIsClimbing)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
    if (FirstPersonCameraComponent)
    {
        // Сохраняем Pitch для использования в анимациях
        float Pitch = FirstPersonCameraComponent->GetComponentRotation().Pitch;
        LookPitch = UKismetMathLibrary::NormalizeAxis(Pitch);
    }
}

void ATechdarkness_DevCharacter::TryClimb()
{
    // Проверяем, можно ли начать лазить
    if (bCanClimbLadder && !bIsClimbing && CurrentLadder)
    {
        FVector Start = CurrentLadder->GetLadderBottomPoint();
        FVector End = CurrentLadder->GetLadderTopPoint();
        FVector LadderDir = (End - Start).GetSafeNormal();
        float Length = FVector::Dist(Start, End);
        // Находим ближайшую точку на лестнице
        float T = FVector::DotProduct(GetActorLocation() - Start, LadderDir);
        T = FMath::Clamp(T, 0.f, Length);
        FVector ClosestPoint = Start + LadderDir * T;
        FVector LadderForward = CurrentLadder->GetActorForwardVector();
        float Offset = GetCapsuleComponent()->GetUnscaledCapsuleRadius() + 1.f;
        // Ставим персонажа впритык к лестнице
        FVector PlaceInFront = ClosestPoint + LadderForward * Offset;
        SetActorLocation(PlaceInFront, true);
        FRotator FaceLadderRotation = LadderForward.Rotation();
        SetActorRotation(FRotator(0.f, FaceLadderRotation.Yaw, 0.f));
        // Активируем лазание — режим Flying и отключаем гравитацию
        bIsClimbing = true;
        GetCharacterMovement()->SetMovementMode(MOVE_Flying);
        GetCharacterMovement()->GravityScale = 0.0f;
        GetCharacterMovement()->Velocity = FVector::ZeroVector;
        LastVerticalInput = 0.f;
    }
}

void ATechdarkness_DevCharacter::StopClimb()
{
    if (bIsClimbing)
    {
        bIsClimbing = false;
        // Возвращаем обычный режим ходьбы и гравитацию
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        GetCharacterMovement()->GravityScale = 1.0f;
        GetCharacterMovement()->Velocity = FVector::ZeroVector;
        LastVerticalInput = 0.f;
    }
}
void ATechdarkness_DevCharacter::StopClimbAndTeleport(const FVector& NewLocation)
{
    StopClimb();
    SetActorLocation(NewLocation, true);
}

void ATechdarkness_DevCharacter::StartSlide()
{
    // Если уже скользим или в воздухе/слишком рано — не даём начать slide
    if (bIsSliding)
        return;
    if (GetCharacterMovement()->IsFalling())
        return;
    float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastSlideEndTime < SlideCooldown)
        return;
    // Вычисляем направление slide — по input или по forward
    FVector InputDir;
    if (Controller)
    {
        const FRotator YawRot(0, Controller->GetControlRotation().Yaw, 0);
        InputDir = UKismetMathLibrary::GetForwardVector(YawRot) * LastVerticalInput;
        if (InputDir.IsNearlyZero())
            InputDir = GetActorForwardVector();
    }
    else
    {
        InputDir = GetActorForwardVector();
    }
    bIsSliding = true;
    // Сохраняем обычные значения на случай если были изменены
    DefaultCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
    DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
    DefaultBrakingFriction = GetCharacterMovement()->BrakingFrictionFactor;
    // Задаём параметры slide
    GetCharacterMovement()->BrakingFrictionFactor = SlideFriction;
    GetCharacterMovement()->GroundFriction = SlideFriction;
    GetCharacterMovement()->bWantsToCrouch = true;
    GetCapsuleComponent()->SetCapsuleHalfHeight(SlideCapsuleHalfHeight);
    // Задаём стартовую скорость скольжения
    GetCharacterMovement()->Velocity = InputDir * SlideImpulse;
    // Запускаем таймер авто-выхода из slide
    GetWorldTimerManager().SetTimer(SlideTimerHandle, this, &ATechdarkness_DevCharacter::StopSlide, MaxSlideTime, false);
}
void ATechdarkness_DevCharacter::StopSlide()
{
    if (!bIsSliding)
        return;
    bIsSliding = false;
    LastSlideEndTime = GetWorld()->GetTimeSeconds();
    // Возвращаем обычные параметры движка и капсулы
    GetCharacterMovement()->BrakingFrictionFactor = DefaultBrakingFriction;
    GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
    GetCharacterMovement()->bWantsToCrouch = false;
    GetCapsuleComponent()->SetCapsuleHalfHeight(DefaultCapsuleHalfHeight);
    // Очищаем таймер, если активен
    GetWorldTimerManager().ClearTimer(SlideTimerHandle);
}
void ATechdarkness_DevCharacter::OnSlideReleased()
{
    StopSlide();
}
void ATechdarkness_DevCharacter::OnMovementModeChanged(EMovementMode PrevMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PrevMode, PreviousCustomMode);
    // Если во время скольжения персонаж начал падать — завершить slide
    if (bIsSliding && GetCharacterMovement()->IsFalling())
    {
        StopSlide();
    }
}

// ----- Функции для спринта -----
void ATechdarkness_DevCharacter::SprintStart()
{
    if (CurrentActionState == EActionState::EAS_Unoccupied && 
        Techdarkness_DevHealthStaminaComponent && 
        Techdarkness_DevHealthStaminaComponent->GetStamina() > 0.f)
    {
        CurrentActionState = EActionState::EAS_Sprinting;
        GetWorld()->GetTimerManager().SetTimer(SprintTimerHandle, this, &ATechdarkness_DevCharacter::SprintLoop, 0.01f, true);
        RestoreStaminaEnd();
    } 
}

void ATechdarkness_DevCharacter::SprintEnds()
{
    if (EActionState::EAS_Sprinting == CurrentActionState)
    {
        CurrentActionState = EActionState::EAS_Unoccupied;
        GetWorld()->GetTimerManager().ClearTimer(SprintTimerHandle);
        GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
        RestoreStaminaStart();
    }
}

void ATechdarkness_DevCharacter::SprintLoop()
{
    if (!Techdarkness_DevHealthStaminaComponent) return;

    const float DeltaTime = GetWorld()->GetDeltaSeconds();

    if (Techdarkness_DevHealthStaminaComponent->GetStamina() <= 0.f)
    {
        Techdarkness_DevHealthStaminaComponent->CurrentStamina = 0.f;
        SprintEnds();
        return;
    }

    Techdarkness_DevHealthStaminaComponent->DrainStamina(StaminaDrainPerSecond * DeltaTime);

    // Интерполяция скорости
    float CurrentSpeed = GetCharacterMovement()->MaxWalkSpeed;
    float NewSpeed = FMath::FInterpTo(CurrentSpeed, SprintSpeed, DeltaTime, AccelerationInterpSpeed);
    GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}

// ----- Функции для стамины -----
void ATechdarkness_DevCharacter::RestoreStaminaStart()
{
    if (!Techdarkness_DevHealthStaminaComponent || Techdarkness_DevHealthStaminaComponent->IsStaminaFull() || EActionState::EAS_Sprinting == CurrentActionState)
    {
        return;
    }
    
    GetWorldTimerManager().SetTimer(StaminaRegenTimerHandle, this, &ATechdarkness_DevCharacter::RestoreStaminaLoop, 0.5f, true);
    
}

void ATechdarkness_DevCharacter::RestoreStaminaEnd()
{
    GetWorldTimerManager().ClearTimer(StaminaRegenTimerHandle);
}

void ATechdarkness_DevCharacter::RestoreStaminaLoop()
{
    if (!Techdarkness_DevHealthStaminaComponent || Techdarkness_DevHealthStaminaComponent->IsStaminaFull())
    {
        RestoreStaminaEnd();
        return;
    }

    const float DeltaTime = GetWorld()->GetDeltaSeconds();

    Techdarkness_DevHealthStaminaComponent->RestoreStamina(RestoreStaminaRate * DeltaTime); 
}