#include "Techdarkness_DevCharacter.h"
#include "Techdarkness_DevHealthStaminaComponent.h"
#include "Techdarkness_RealCMC.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Ladder.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ATechdarkness_DevCharacter::ATechdarkness_DevCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UTechdarkness_RealCMC>(ACharacter::CharacterMovementComponentName))
{
    // Коллизия капсулы
    GetCapsuleComponent()->InitCapsuleSize(35.f, 96.f);

    // Камера первого лица
    FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCameraComponent->SetupAttachment(GetMesh(), TEXT("CameraSocket"));
    FirstPersonCameraComponent->bUsePawnControlRotation = true;

    // Компонент здоровья/стамины
    HealthStaminaComponent = CreateDefaultSubobject<UTechdarkness_DevHealthStaminaComponent>(TEXT("HealthStaminaComponent"));

    // Переменные по умолчанию
    bIsClimbing = false;
    bCanClimbLadder = false;
    CurrentLadder = nullptr;
    LastVerticalInput = 0.f;
    LookPitch = 0.f;
    BaseSpeed = 500.f; // default
    CurrentActionState = EActionState::EAS_Unoccupied;
}

void ATechdarkness_DevCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Сохраняем стартовые скорости
    BaseSpeed = GetCharacterMovement()->MaxWalkSpeed;

    // Input mapping (для EnhancedInputSubsystems)
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
        // Движение
        if (MoveAction)
        {
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATechdarkness_DevCharacter::Move);
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &ATechdarkness_DevCharacter::OnMoveActionReleased);
        }

        // Камера
        if (LookAction)
        {
            EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATechdarkness_DevCharacter::Look);
        }

        // Лестница
        if (ClimbAction)
        {
            EnhancedInput->BindAction(ClimbAction, ETriggerEvent::Started, this, &ATechdarkness_DevCharacter::TryClimb);
        }

        // --- Скользжение (Slide) ---
        if (SlideAction)
        {
            EnhancedInput->BindAction(SlideAction, ETriggerEvent::Started, this, &ATechdarkness_DevCharacter::OnSlideStarted);
            EnhancedInput->BindAction(SlideAction, ETriggerEvent::Completed, this, &ATechdarkness_DevCharacter::OnSlideReleased);
        }

        // --- Спринт (Sprint) ---
        if (SprintAction)
        {
            EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ATechdarkness_DevCharacter::OnSprintStarted);
            EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATechdarkness_DevCharacter::OnSprintReleased);
        }
    }
}

void ATechdarkness_DevCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    if (!Controller)
        return;

    float VerticalInput = MovementVector.Y;
    LastVerticalInput = VerticalInput;

    // Лазание по лестнице
    if (bIsClimbing && CurrentLadder)
    {
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

        // Сход вниз
        if (NextT < 0.f)
        {
            StopClimb();
            FVector ExitPoint = Start - LadderForward * (Offset + 10.f) - FVector(0,0,50.f);
            SetActorLocation(ExitPoint, true);
            bCanClimbLadder = false;
            CurrentLadder = nullptr;
            return;
        }
        // Сход вверх
        if (NextT > Length - ExitThreshold)
        {
            StopClimb();
            FVector CapsuleTop = End + LadderForward * (Offset + 20.f) + FVector(0,0,50.f);
            FVector TraceStart = CapsuleTop;
            FVector TraceEnd = CapsuleTop - FVector(0,0,120.f);
            FHitResult HitResult;
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(this);
            bool bFoundPlatform = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);
            if (bFoundPlatform)
            {
                FVector PlaceOnTop = HitResult.ImpactPoint 
                    + LadderForward * (Offset + 5.f)
                    + FVector(0, 0, GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() + 2.f);
                SetActorLocation(PlaceOnTop, true);
            }
            else
            {
                FVector ExitPoint = End + LadderForward * (Offset + 20.f) + FVector(0,0,60.f);
                SetActorLocation(ExitPoint, true);
            }
            bCanClimbLadder = false;
            CurrentLadder = nullptr;
            return;
        }
        // Движение по лестнице
        GetCharacterMovement()->Velocity = LadderDir * VerticalInput * ClimbSpeed;
        if (FMath::Abs(VerticalInput) < 0.1f)
            GetCharacterMovement()->Velocity = FVector::ZeroVector;
    }
    else
    {
        // Обычное движение по земле
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
        float Pitch = FirstPersonCameraComponent->GetComponentRotation().Pitch;
        LookPitch = UKismetMathLibrary::NormalizeAxis(Pitch);
    }
}

void ATechdarkness_DevCharacter::OnMoveActionReleased()
{
    // Остановить движение на лестнице, если нужно
    if (bIsClimbing && GetCharacterMovement())
    {
        GetCharacterMovement()->Velocity = FVector::ZeroVector;
        LastVerticalInput = 0.f;
    }
}

void ATechdarkness_DevCharacter::TryClimb()
{
    if (bCanClimbLadder && !bIsClimbing && CurrentLadder)
    {
        FVector Start = CurrentLadder->GetLadderBottomPoint();
        FVector End   = CurrentLadder->GetLadderTopPoint();
        FVector LadderDir = (End - Start).GetSafeNormal();
        float Length = FVector::Dist(Start, End);

        float T = FVector::DotProduct(GetActorLocation() - Start, LadderDir);
        T = FMath::Clamp(T, 0.f, Length);
        FVector ClosestPoint = Start + LadderDir * T;

        FVector LadderForward = CurrentLadder->GetActorForwardVector();
        float Offset = GetCapsuleComponent()->GetUnscaledCapsuleRadius() + 1.f;

        // Повернуть контроллер к лестнице
        FRotator FaceLadderRotation = LadderForward.Rotation();
        float LadderYaw = FaceLadderRotation.Yaw + 180.0f;
        FRotator FaceLadderYaw(0.f, LadderYaw, 0.f);
        SetActorRotation(FaceLadderYaw);

        if (Controller)
            Controller->SetControlRotation(FaceLadderYaw);

        FVector PlaceInFront = ClosestPoint + LadderForward * Offset;
        SetActorLocation(PlaceInFront, true);

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

void ATechdarkness_DevCharacter::OnMovementModeChanged(EMovementMode PrevMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PrevMode, PreviousCustomMode);
    if (bIsClimbing && GetCharacterMovement()->IsFalling())
        StopClimb();
    
}
void ATechdarkness_DevCharacter::OnSlideStarted(const FInputActionInstance& Instance)
{
    if (auto* MoveComp = Cast<UTechdarkness_RealCMC>(GetCharacterMovement()))
    {
        MoveComp->Input_SlidePressed();
        UE_LOG(LogTemp, Warning, TEXT("OnSlideStarted called"));
    }
}

void ATechdarkness_DevCharacter::OnSlideReleased(const FInputActionInstance& Instance)
{
    if (auto* MoveComp = Cast<UTechdarkness_RealCMC>(GetCharacterMovement()))
    {
        MoveComp->Input_SlideReleased();
    }
}

void ATechdarkness_DevCharacter::OnSprintStarted(const FInputActionInstance& Instance)
{
    if (auto* MoveComp = Cast<UTechdarkness_RealCMC>(GetCharacterMovement()))
    {
        MoveComp->Input_SprintPressed();
        UE_LOG(LogTemp, Warning, TEXT("OnSprintStarted called"));
    }
}

void ATechdarkness_DevCharacter::OnSprintReleased(const FInputActionInstance& Instance)
{
    if (auto* MoveComp = Cast<UTechdarkness_RealCMC>(GetCharacterMovement()))
    {
        MoveComp->Input_SprintReleased();
    }
}