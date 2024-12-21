// Fill out your copyright notice in the Description page of Project Settings.


#include "Techdarkness_RealMoveCharacter.h"
#include "Techdarkness_RealCMC.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"


// Sets default values
ATechdarkness_RealMoveCharacter::ATechdarkness_RealMoveCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UTechdarkness_RealCMC>(ACharacter::CharacterMovementComponentName))
{
	Techdarkness_RealCMC = Cast<UTechdarkness_RealCMC>(GetCharacterMovement());

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATechdarkness_RealMoveCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}
// Called every frame
void ATechdarkness_RealMoveCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ATechdarkness_RealMoveCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATechdarkness_RealMoveCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATechdarkness_RealMoveCharacter::Look);

		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ATechdarkness_RealMoveCharacter::HandleCrouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ATechdarkness_RealMoveCharacter::HandleUnCrouch);
		//EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ATechdarkness_RealMoveCharacter::HandleCrouchToggle);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ATechdarkness_RealMoveCharacter::SprintPressed);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATechdarkness_RealMoveCharacter::SprintReleased);
		
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."));
	}
}

FCollisionQueryParams ATechdarkness_RealMoveCharacter::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams params;

	TArray<AActor*> characterChildren;
	GetAllChildActors(characterChildren);
	params.AddIgnoredActors(characterChildren);
	params.AddIgnoredActor(this);
	return params;
}



void ATechdarkness_RealMoveCharacter::HandleCrouch()
{
	Techdarkness_RealCMC->ChrouchPressed();
}
void ATechdarkness_RealMoveCharacter::HandleUnCrouch()
{
	Techdarkness_RealCMC->ChrouchReleased();
}

void ATechdarkness_RealMoveCharacter::HandleCrouchToggle()
{
	if (Controller != nullptr)
	{
		if (ACharacter::bIsCrouched)
		{
			ACharacter::UnCrouch();
		}
		else
		{
			ACharacter::Crouch();
		}
	}
}

void ATechdarkness_RealMoveCharacter::SprintPressed() {
	Techdarkness_RealCMC->SprintPressed();
}

void ATechdarkness_RealMoveCharacter::SprintReleased() {
	Techdarkness_RealCMC->SprintReleased();
}


void ATechdarkness_RealMoveCharacter::Move(const FInputActionValue& Value) {
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ATechdarkness_RealMoveCharacter::Look(const FInputActionValue& Value) {
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

