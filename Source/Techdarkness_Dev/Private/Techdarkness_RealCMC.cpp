#include "Techdarkness_RealCMC.h"
#include "Techdarkness_DevCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

UTechdarkness_RealCMC::UTechdarkness_RealCMC()
{
    SlideCapsuleHalfHeight = 48.f;
    DefaultCapsuleHalfHeight = 96.f;
    DefaultGroundFriction = 8.f;
    DefaultBrakingFriction = 2.f;
    WalkSpeed = 500.f;
}

void UTechdarkness_RealCMC::InitializeComponent()
{
    Super::InitializeComponent();
    OwningCharacter = Cast<ATechdarkness_DevCharacter>(GetOwner());
    if (OwningCharacter)
        DefaultCapsuleHalfHeight = OwningCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
}

void UTechdarkness_RealCMC::BeginPlay()
{
    Super::BeginPlay();
    if (OwningCharacter)
        DefaultCapsuleHalfHeight = OwningCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
    MaxWalkSpeed = WalkSpeed;
}

bool UTechdarkness_RealCMC::IsMovingOnGround() const
{
    return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide);
}

void UTechdarkness_RealCMC::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
    Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

    if (MovementMode == MOVE_Walking)
        MaxWalkSpeed = bWantsToSprint ? SprintSpeed : WalkSpeed;
}

void UTechdarkness_RealCMC::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
    if (IsCustomMovementMode(CMOVE_Slide) && MovementMode == MOVE_Falling)
        StopSlide();
}

void UTechdarkness_RealCMC::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
    if (IsCustomMovementMode(CMOVE_Slide) && !bIsSliding)
        StopSlide();

    Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UTechdarkness_RealCMC::PhysCustom(float deltaTime, int32 Iterations)
{
    if (CustomMovementMode == CMOVE_Slide)
        PhysSlide(deltaTime, Iterations);
    else
        Super::PhysCustom(deltaTime, Iterations);
}

// ----- API для персонажа (input) -----
void UTechdarkness_RealCMC::Input_SlidePressed()
{
    if (bReadyForSlide)
    {
        StartSlide();
        bReadyForSlide = false;
    }
}
void UTechdarkness_RealCMC::Input_SlideReleased()
{
    bReadyForSlide = true;
    StopSlide();
}
void UTechdarkness_RealCMC::Input_SprintPressed()  { SprintStart(); }
void UTechdarkness_RealCMC::Input_SprintReleased() { SprintEnds(); }

// --- Спринт ---
void UTechdarkness_RealCMC::SprintStart()
{
    bWantsToSprint = true;
    // Здесь можно запустить таймер дренажа стамины через OwningCharacter->HealthStaminaComponent
}

void UTechdarkness_RealCMC::SprintEnds()
{
    bWantsToSprint = false;
    MaxWalkSpeed = WalkSpeed;
    // Тут можно завершить таймер стамины
}

void UTechdarkness_RealCMC::SprintLoop()
{
    if (!OwningCharacter) return;
    // Вызов OwningCharacter->HealthStaminaComponent->DrainStamina(...) при необходимости
}

// --- Слайд ---
void UTechdarkness_RealCMC::StartSlide()
{
    if (bIsSliding || !OwningCharacter || IsFalling())
    {
        return;
    }
    
    float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastSlideEndTime < SlideCooldown) return;

    bIsSliding = true;
    DefaultCapsuleHalfHeight = OwningCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
    DefaultGroundFriction = GroundFriction;
    DefaultBrakingFriction = BrakingFrictionFactor;

    BrakingFrictionFactor = SlideFriction;
    GroundFriction = SlideFriction;
    OwningCharacter->GetCapsuleComponent()->SetCapsuleHalfHeight(SlideCapsuleHalfHeight);

    Velocity = OwningCharacter->GetActorForwardVector() * SlideImpulse;
    EnterSlide();

    GetWorld()->GetTimerManager().SetTimer(SlideTimerHandle, this, &UTechdarkness_RealCMC::StopSlide, MaxSlideTime, false);
}

void UTechdarkness_RealCMC::StopSlide()
{
    if (!bIsSliding) return;
    bIsSliding = false;
    LastSlideEndTime = GetWorld()->GetTimeSeconds();

    if (OwningCharacter)
        OwningCharacter->GetCapsuleComponent()->SetCapsuleHalfHeight(DefaultCapsuleHalfHeight);
    GroundFriction = DefaultGroundFriction;
    BrakingFrictionFactor = DefaultBrakingFriction;

    ExitSlide();
    GetWorld()->GetTimerManager().ClearTimer(SlideTimerHandle);
}

void UTechdarkness_RealCMC::EnterSlide()
{
    SetMovementMode(MOVE_Custom, CMOVE_Slide);
}

void UTechdarkness_RealCMC::ExitSlide()
{
    SetMovementMode(MOVE_Walking); // Вернуться к обычному режиму ходьбы
}

void UTechdarkness_RealCMC::PhysSlide(float deltaTime, int32 Iterations)
{
    if (deltaTime < MIN_TICK_TIME) return;

    FHitResult surfaceHit;
    if (!GetSlideSurface(surfaceHit) || Velocity.Size() < 200.f)
    {
        StopSlide();
        StartNewPhysics(deltaTime, Iterations);
        return;
    }

    Velocity += FVector::DownVector * 980.0f * deltaTime; // Gravity

    FVector Adjusted = Velocity * deltaTime;
    SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentRotation().Quaternion(), true, surfaceHit);

    if (surfaceHit.bBlockingHit)
        SlideAlongSurface(Adjusted, 1.f - surfaceHit.Time, surfaceHit.Normal, surfaceHit, true);
}

bool UTechdarkness_RealCMC::GetSlideSurface(FHitResult& Hit) const
{
    if (!OwningCharacter) return false;
    FVector Start = UpdatedComponent->GetComponentLocation();
    FVector End = Start + FVector::DownVector * (DefaultCapsuleHalfHeight + 10.f);
    return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility);
}

bool UTechdarkness_RealCMC::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
    return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}