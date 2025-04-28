// Techdarkness_DevCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "InputActionValue.h"
#include "Techdarkness_DevCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ATechdarkness_DevCharacter : public ACharacter
{
    GENERATED_BODY()

    // === FIRST PERSON COMPONENTS ===
    /** Pawn mesh: 1st person view (arms; seen only by self) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = true))
    USkeletalMeshComponent* Mesh1P;

    /** First person camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = true))
    UCameraComponent* FirstPersonCameraComponent;

    // === INPUT SYSTEM ===
    /** Input mapping context */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = true))
    UInputMappingContext* DefaultMappingContext;

    /** Jump Input Action */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = true))
    UInputAction* JumpAction;

    /** Move Input Action */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = true))
    UInputAction* MoveAction;

    /** Look Input Action */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = true))
    UInputAction* LookAction;
    
public:
    ATechdarkness_DevCharacter();

    /** Returns Mesh1P subobject **/
    USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
    /** Returns FirstPersonCameraComponent subobject **/
    UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

protected:
    virtual void BeginPlay() override;

    /** Called for movement input */
    void Move(const FInputActionValue& Value);

    /** Called for looking input */
    void Look(const FInputActionValue& Value);

    // APawn interface
    virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
};