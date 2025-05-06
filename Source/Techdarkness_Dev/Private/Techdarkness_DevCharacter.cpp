// --- Techdarkness_DevCharacter.cpp ---
#include "Techdarkness_DevCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);


ATechdarkness_DevCharacter::ATechdarkness_DevCharacter()
{
    GetCapsuleComponent()->InitCapsuleSize(35.f, 96.0f);


    // КАМЕРА — чуть впереди и выше "шейного позвонка"
    FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCameraComponent->SetupAttachment(GetMesh(), TEXT("CameraSocket"));
    //FirstPersonCameraComponent->SetRelativeLocation(FVector(12.f, 0.f, 74.f)); // настрой и высоту, и смещение вперёд
    FirstPersonCameraComponent->bUsePawnControlRotation = true;


    // Один тпс-меш (GetMesh) — для всех режимов
    GetMesh()->SetupAttachment(GetCapsuleComponent());
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
    GetMesh()->SetOwnerNoSee(false); // видим всегда (self-body)
    GetMesh()->SetOnlyOwnerSee(false);
    GetMesh()->CastShadow = true;
    GetMesh()->bCastDynamicShadow = true;
    GetMesh()->bCastHiddenShadow = true;
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);


    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
    bUseControllerRotationYaw = true;
    bIsClimbing = false;
}


void ATechdarkness_DevCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Инпут система
    if (APlayerController* PC = Cast<APlayerController>(Controller))
        if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
            if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
                if (DefaultMappingContext)
                    InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
}


void ATechdarkness_DevCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    check(PlayerInputComponent);


    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATechdarkness_DevCharacter::Move);
        if (LookAction)
            EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATechdarkness_DevCharacter::Look);
        if (CrouchAction) {
            EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &ATechdarkness_DevCharacter::StartCrouch);
            EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ATechdarkness_DevCharacter::StopCrouch);
        }
        if (ClimbAction) {
            EnhancedInput->BindAction(ClimbAction, ETriggerEvent::Started, this, &ATechdarkness_DevCharacter::TryClimb);
            EnhancedInput->BindAction(ClimbAction, ETriggerEvent::Completed, this, &ATechdarkness_DevCharacter::StopClimb);
        }
    }
}


void ATechdarkness_DevCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller && !bIsClimbing) {
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


    // ПЕРЕДАЕМ Pitch в AnimBP для наклона тела (корректно Clamp в пределах)
    if (FirstPersonCameraComponent) {
        float Pitch = FirstPersonCameraComponent->GetComponentRotation().Pitch;
        LookPitch = UKismetMathLibrary::NormalizeAxis(Pitch);
    }
}


void ATechdarkness_DevCharacter::StartCrouch()   { Crouch();  }
void ATechdarkness_DevCharacter::StopCrouch()    { UnCrouch();}
void ATechdarkness_DevCharacter::TryClimb()      { bIsClimbing = true;  }
void ATechdarkness_DevCharacter::StopClimb()     { bIsClimbing = false; }
void ATechdarkness_DevCharacter::SetCinematicMode(bool bCinematic)
{
    GetMesh()->SetVisibility(!bCinematic, true);
}
