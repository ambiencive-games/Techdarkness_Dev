// --- Techdarkness_DevCharacter.h ---
#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "InputActionValue.h"
#include "Techdarkness_DevCharacter.generated.h"


class UCameraComponent;
class UInputComponent;
class UInputMappingContext;
class UInputAction;


DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);


UCLASS(config=Game)
class ATechdarkness_DevCharacter : public ACharacter
{
	GENERATED_BODY()
    
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ClimbAction;


	ATechdarkness_DevCharacter();


protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;


	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);


	void StartCrouch();
	void StopCrouch();


	void TryClimb();
	void StopClimb();


	void SetCinematicMode(bool bCinematic);


	// --- Real Body ---
	/** Передача Pitch камеры для анимационного наклона туловища */
	UPROPERTY(BlueprintReadOnly, Category="Look")
	float LookPitch = 0;


	bool bIsClimbing = false;
};
