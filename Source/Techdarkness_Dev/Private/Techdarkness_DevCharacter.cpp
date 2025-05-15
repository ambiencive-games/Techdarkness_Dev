#include "Techdarkness_DevCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Ladder.h"
#include "DrawDebugHelpers.h"

ATechdarkness_DevCharacter::ATechdarkness_DevCharacter()
{
    GetCapsuleComponent()->InitCapsuleSize(35.f, 96.0f);

    // Mesh и камера
    GetMesh()->SetupAttachment(GetCapsuleComponent());
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
    GetMesh()->SetOwnerNoSee(false);
    GetMesh()->SetOnlyOwnerSee(false);
    GetMesh()->CastShadow = true;
    GetMesh()->bCastDynamicShadow = true;
    GetMesh()->bCastHiddenShadow = true;
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCameraComponent->SetupAttachment(GetMesh(), TEXT("CameraSocket"));
    FirstPersonCameraComponent->bUsePawnControlRotation = true;

    bIsClimbing = false;
    bCanClimbLadder = false;
    CurrentLadder = nullptr;
    LastVerticalInput = 0.f;
}

void ATechdarkness_DevCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                if (DefaultMappingContext)
                    InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }
}

void ATechdarkness_DevCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    check(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATechdarkness_DevCharacter::Move);
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &ATechdarkness_DevCharacter::OnMoveActionReleased);
        }
        if (LookAction)
            EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATechdarkness_DevCharacter::Look);
        if (ClimbAction)
            EnhancedInput->BindAction(ClimbAction, ETriggerEvent::Started, this, &ATechdarkness_DevCharacter::TryClimb);
    }
}

void ATechdarkness_DevCharacter::OnMoveActionReleased()
{
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

        // ==== ВЫХОД ВНИЗ ====
        if (NextT < 0.f)
        {
            StopClimb();
            // Смещение вниз и немного "назад"
            FVector ExitPoint = Start - LadderForward* (Offset + 10.f) - FVector(0,0,50.f);
            SetActorLocation(ExitPoint, true);
            bCanClimbLadder = false;
            CurrentLadder = nullptr;
            UE_LOG(LogTemp, Warning, TEXT("Ladder down"));
            return;
        }

        // ==== ВЫХОД ВВЕРХ ====
        if (NextT > Length - ExitThreshold)
        {
            StopClimb();
            // Сначала попробуем найти платформу над верхом лестницы (LineTrace вниз)
            FVector CapsuleTop = End + LadderForward* (Offset + 20.f) + FVector(0,0,50.f);
            FVector TraceStart = CapsuleTop;
            FVector TraceEnd = CapsuleTop - FVector(0,0,120.f);

            FHitResult HitResult;
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(this);
            UE_LOG(LogTemp, Warning, TEXT("WTF"));

            bool bFoundPlatform = GetWorld()->LineTraceSingleByChannel
            (
                HitResult,
                TraceStart,
                TraceEnd,
                ECC_Visibility,
                Params
            );
            
            if (bFoundPlatform)
            {
                // Смещение от края лестницы вперёд, чтобы капсула точно встала на платформу
                FVector PlaceOnTop = HitResult.ImpactPoint 
                    + LadderForward * (Offset + 5.f)                      // чуть дальше от края
                    + FVector(0, 0, GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() + 2.f); // и сразу на платформу
                SetActorLocation(PlaceOnTop, true);
                UE_LOG(LogTemp, Warning, TEXT("Found platform, placed on top"));
            }
            else
            {
                // fallback: стандарт
                FVector ExitPoint = End + LadderForward * (Offset + 20.f) + FVector(0,0,60.f);
                SetActorLocation(ExitPoint, true);
                UE_LOG(LogTemp, Warning, TEXT("NOT Found platform"));
            }
            bCanClimbLadder = false;
            CurrentLadder = nullptr;
            return;
        }

        // ==== ДВИЖЕНИЕ ПО ЛЕСТНИЦЕ ====
        GetCharacterMovement()->Velocity = LadderDir * VerticalInput * ClimbSpeed;

        if (FMath::Abs(VerticalInput) < 0.1f) // если нет input — стоп
        {
            GetCharacterMovement()->Velocity = FVector::ZeroVector;
        }
    }
    else
    {
        // Обычное передвижение
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

void ATechdarkness_DevCharacter::TryClimb()
{
    if (bCanClimbLadder && !bIsClimbing && CurrentLadder)
    {
        FVector Start = CurrentLadder->GetLadderBottomPoint();
        FVector End = CurrentLadder->GetLadderTopPoint();
        FVector LadderDir = (End - Start).GetSafeNormal();
        float Length = FVector::Dist(Start, End);
        float T = FVector::DotProduct(GetActorLocation() - Start, LadderDir);
        T = FMath::Clamp(T, 0.f, Length);
        FVector ClosestPoint = Start + LadderDir * T;
        FVector LadderForward = CurrentLadder->GetActorForwardVector();
        float Offset = GetCapsuleComponent()->GetUnscaledCapsuleRadius() + 1.f;
        FVector PlaceInFront = ClosestPoint + LadderForward * Offset;
        SetActorLocation(PlaceInFront, true);
        FRotator FaceLadderRotation = LadderForward.Rotation();
        SetActorRotation(FRotator(0.f, FaceLadderRotation.Yaw, 0.f));

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