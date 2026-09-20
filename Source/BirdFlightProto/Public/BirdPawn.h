#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "BirdPawn.generated.h"

class USpringArmComponent;
class UCapsuleComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;


// Main gameplay flight states used by the bird controller.
UENUM(BlueprintType)
enum class EFlightState : uint8
{
    Grounded,
    PoweredFlight,
    Gliding
};


UCLASS()
class BIRDFLIGHTPROTO_API ABirdPawn : public APawn
{
    GENERATED_BODY()

public:

    ABirdPawn();

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(
        UInputComponent* PlayerInputComponent) override;

    // Returns the current flight state for the Animation Blueprint.
    UFUNCTION(BlueprintPure, Category = "Flight")
    EFlightState GetFlightState() const;


protected:

    virtual void BeginPlay() override;


    // Root collision used for bird movement.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCapsuleComponent* CapsuleComponent;

    // Visual skeletal mesh of the bird.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* BirdMesh;

    // Camera boom used for third-person framing and lag.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USpringArmComponent* SpringArm;

    // Main gameplay camera.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* Camera;


    // Default Enhanced Input mapping context.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;


    // Current gameplay flight state.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Runtime")
    EFlightState CurrentFlightState;

    // Current forward movement speed.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Runtime")
    float CurrentThrustSpeed;

    // Desired forward movement speed.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Runtime")
    float TargetThrustSpeed;

    // Target visual banking angle.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Runtime")
    float TargetBankAngle;

    // Current smoothed visual banking angle.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight|Runtime")
    float CurrentBankAngle;


    // Maximum powered flight speed.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Tuning")
    float MaxThrustSpeed;

    // Acceleration and deceleration rate.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Tuning")
    float AccelerationRate;

    // Vertical mouse steering sensitivity.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Tuning")
    float PitchSensitivity;

    // Horizontal mouse steering sensitivity.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Tuning")
    float YawSensitivity;

    // Maximum visual banking angle.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Tuning")
    float MaxBankAngle;

    // Speed of the visual bank interpolation.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Tuning")
    float BankInterpSpeed;

    // Target forward speed while gliding.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Tuning")
    float GlideSpeed;

    // Constant downward speed applied during glide.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Tuning")
    float GlideDescentSpeed;

    // Distance used to detect the ground while gliding.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Tuning")
    float GroundCheckDistance;

    // Speed used to settle rotation after landing.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Tuning")
    float LandingRotationInterpSpeed;

    // Small vertical lift applied during takeoff.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Tuning")
    float TakeoffLiftDistance;

    // Duration of the smooth takeoff lift.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Tuning")
    float TakeoffLiftDuration;


    // Original mesh rotation used as the base for visual banking.
    FRotator BaseMeshRotation;

    // Raw mouse input consumed by the steering system.
    float RawPitchInput;
    float RawYawInput;

    // Runtime values for the smooth takeoff assist.
    bool bTakeoffAssistActive;
    float TakeoffElapsedTime;
    float AppliedTakeoffLift;


    // Smoothed horizontal input used for visual banking.
    float SmoothedYawInput;

    // Controls how quickly visual bank input follows mouse movement.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight|Tuning")
    float BankInputInterpSpeed;


    // Default camera FOV at low speed.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Tuning")
    float BaseFOV;

    // Maximum camera FOV at full flight speed.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Tuning")
    float MaxSpeedFOV;

    // Controls how quickly FOV reacts to speed changes.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Tuning")
    float FOVInterpSpeed;


    // Handles vertical and horizontal mouse input.
    void Look(const FInputActionValue& Actions);

    // Handles powered flight and glide input.
    void Move(const FInputActionValue& Actions);

    // Applies the smooth vertical takeoff assist.
    void CalculateTakeoffMovement(float DeltaTime);

    // Handles acceleration and forward movement.
    void CalculateForwardMovement(float DeltaTime);

    // Handles pitch, yaw and visual banking.
    void CalculateSteering(float DeltaTime);

    // Applies downward movement while gliding.
    void CalculateGlideMovement(float DeltaTime);

    // Handles state changes and state-specific setup.
    void SetFlightState(EFlightState NewState);

    // Checks whether the bird is close enough to the ground to land.
    bool IsGroundNearby() const;

    // Updates camera FOV based on current flight speed.
    void UpdateCameraFOV(float DeltaTime);
};