#include "BirdPawn.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"


ABirdPawn::ABirdPawn()
{
    PrimaryActorTick.bCanEverTick = true;


    // Create the root collision capsule.
    CapsuleComponent =
        CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));

    SetRootComponent(CapsuleComponent);

    CapsuleComponent->SetCapsuleHalfHeight(30.f);
    CapsuleComponent->SetCapsuleRadius(20.f);


    // Create and attach the bird mesh.
    BirdMesh =
        CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BirdMesh"));

    BirdMesh->SetupAttachment(GetRootComponent());


    // Create the third-person camera boom.
    SpringArm =
        CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));

    SpringArm->SetupAttachment(GetRootComponent());

    SpringArm->TargetArmLength = 300.f;
    SpringArm->bUsePawnControlRotation = false;


    // Create the gameplay camera.
    Camera =
        CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));

    Camera->SetupAttachment(SpringArm);

    Camera->bUsePawnControlRotation = false;


    // Start the bird in the Grounded state.
    CurrentFlightState = EFlightState::Grounded;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;


    // Powered flight defaults.
    MaxThrustSpeed = 600.f;
    AccelerationRate = 300.f;

    CurrentThrustSpeed = 0.f;
    TargetThrustSpeed = 0.f;

    PitchSensitivity = 0.5f;
    YawSensitivity = 0.5f;


    // Visual bank defaults.
    MaxBankAngle = 50.f;
    BankInterpSpeed = 5.f;

    TargetBankAngle = 0.f;
    CurrentBankAngle = 0.f;


    // Input defaults.
    RawPitchInput = 0.f;
    RawYawInput = 0.f;


    // Glide defaults.
    GlideSpeed = 400.f;
    GlideDescentSpeed = 60.f;


    // Landing defaults.
    GroundCheckDistance = 24.f;
    LandingRotationInterpSpeed = 5.f;


    // Takeoff defaults.
    TakeoffLiftDistance = 12.f;
    TakeoffLiftDuration = 0.25f;

    bTakeoffAssistActive = false;
    TakeoffElapsedTime = 0.f;
    AppliedTakeoffLift = 0.f;


    // Visual bank smoothing.
    BankInputInterpSpeed = 6.f;
    SmoothedYawInput = 0.f;


    // Dynamic FOV defaults.
    BaseFOV = 50.f;
    MaxSpeedFOV = 125.f;
    FOVInterpSpeed = 3.f;
}


void ABirdPawn::BeginPlay()
{
    Super::BeginPlay();


    // Save the mesh's original relative rotation.
    BaseMeshRotation = BirdMesh->GetRelativeRotation();


    // Add the Enhanced Input mapping context for the local player.
    if (APlayerController* PlayerController =
        Cast<APlayerController>(GetController()))
    {
        if (ULocalPlayer* LocalPlayer =
            PlayerController->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
                ULocalPlayer::GetSubsystem<
                    UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
            {
                if (DefaultMappingContext)
                {
                    Subsystem->AddMappingContext(
                        DefaultMappingContext,
                        0);
                }
            }
        }
    }
}


void ABirdPawn::CalculateTakeoffMovement(float DeltaTime)
{
    // Skip the function when no takeoff assist is active.
    if (!bTakeoffAssistActive)
    {
        return;
    }


    TakeoffElapsedTime += DeltaTime;


    // Prevent division by zero.
    const float SafeDuration =
        FMath::Max(
            TakeoffLiftDuration,
            KINDA_SMALL_NUMBER);


    // Convert elapsed takeoff time to a normalized value.
    const float Alpha =
        FMath::Clamp(
            TakeoffElapsedTime / SafeDuration,
            0.f,
            1.f);


    // Smooth the takeoff lift in and out.
    const float SmoothAlpha =
        FMath::InterpEaseInOut(
            0.f,
            1.f,
            Alpha,
            2.f);


    const float TargetTotalLift =
        SmoothAlpha * TakeoffLiftDistance;


    // Apply only the remaining lift for this frame.
    const float FrameLiftDelta =
        TargetTotalLift - AppliedTakeoffLift;


    AddActorWorldOffset(
        FVector(
            0.f,
            0.f,
            FrameLiftDelta),
        true);


    AppliedTakeoffLift =
        TargetTotalLift;


    // Stop the takeoff assist when the lift is complete.
    if (Alpha >= 1.f)
    {
        bTakeoffAssistActive = false;

        TakeoffElapsedTime = 0.f;
        AppliedTakeoffLift = 0.f;
    }
}


void ABirdPawn::Look(const FInputActionValue& Actions)
{
    const FVector2D LocalAxisValue =
        Actions.Get<FVector2D>();


    // Store the latest mouse movement for steering.
    if (GetController())
    {
        RawYawInput = LocalAxisValue.X;
        RawPitchInput = LocalAxisValue.Y;
    }
}


void ABirdPawn::Move(const FInputActionValue& Actions)
{
    const FVector2D LocalAxisValue =
        Actions.Get<FVector2D>();


    switch (CurrentFlightState)
    {
    case EFlightState::Grounded:
    {
        // Start powered flight when forward input is pressed.
        if (LocalAxisValue.Y > 0.f)
        {
            SetFlightState(
                EFlightState::PoweredFlight);


            TargetThrustSpeed =
                LocalAxisValue.Y * MaxThrustSpeed;
        }

        break;
    }


    case EFlightState::PoweredFlight:
    {
        // Update powered flight speed while input is held.
        if (LocalAxisValue.Y > 0.f)
        {
            TargetThrustSpeed =
                LocalAxisValue.Y * MaxThrustSpeed;
        }
        else
        {
            // Releasing forward input enters Glide.
            SetFlightState(
                EFlightState::Gliding);
        }

        break;
    }


    case EFlightState::Gliding:
    {
        // Resume powered flight from Glide.
        if (LocalAxisValue.Y > 0.f)
        {
            SetFlightState(
                EFlightState::PoweredFlight);


            TargetThrustSpeed =
                LocalAxisValue.Y * MaxThrustSpeed;
        }

        break;
    }
    }
}


void ABirdPawn::CalculateForwardMovement(float DeltaTime)
{
    // Smoothly move current speed toward the target speed.
    CurrentThrustSpeed =
        FMath::FInterpConstantTo(
            CurrentThrustSpeed,
            TargetThrustSpeed,
            DeltaTime,
            AccelerationRate);


    FVector ForwardDirection =
        GetActorForwardVector();


    // Prevent Glide from gaining altitude from forward movement.
    if (CurrentFlightState == EFlightState::Gliding)
    {
        ForwardDirection.Z =
            FMath::Min(
                ForwardDirection.Z,
                0.f);
    }


    const FVector DeltaLocation =
        ForwardDirection *
        CurrentThrustSpeed *
        DeltaTime;


    AddActorWorldOffset(
        DeltaLocation,
        true);
}


void ABirdPawn::CalculateSteering(float DeltaTime)
{
    // Ignore steering while the bird is grounded.
    if (CurrentFlightState == EFlightState::Grounded)
    {
        RawYawInput = 0.f;
        RawPitchInput = 0.f;

        return;
    }


    // Calculate gameplay pitch and yaw.
    const float DeltaPitch =
        RawPitchInput * PitchSensitivity;


    const float DeltaYaw =
        RawYawInput * YawSensitivity;


    AddActorLocalRotation(
        FRotator(
            DeltaPitch,
            DeltaYaw,
            0.f),
        true);


    // Smooth horizontal input before using it for visual banking.
    SmoothedYawInput =
        FMath::FInterpTo(
            SmoothedYawInput,
            RawYawInput,
            DeltaTime,
            BankInputInterpSpeed);


    // Calculate the target visual bank angle.
    TargetBankAngle =
        FMath::Clamp(
            SmoothedYawInput * MaxBankAngle,
            -MaxBankAngle,
            MaxBankAngle);


    // Smoothly move toward the target bank.
    CurrentBankAngle =
        FMath::FInterpTo(
            CurrentBankAngle,
            TargetBankAngle,
            DeltaTime,
            BankInterpSpeed);


    // Apply visual banking to the crow mesh.
    BirdMesh->SetRelativeRotation(
        FRotator(
            BaseMeshRotation.Pitch + CurrentBankAngle,
            BaseMeshRotation.Yaw,
            BaseMeshRotation.Roll));


    // Clear raw mouse input after processing.
    RawYawInput = 0.f;
    RawPitchInput = 0.f;
}


void ABirdPawn::SetFlightState(EFlightState NewState)
{
    // Ignore duplicate state requests.
    if (NewState == CurrentFlightState)
    {
        return;
    }


    const EFlightState PreviousState =
        CurrentFlightState;


    CurrentFlightState =
        NewState;


    // Reset movement values when landing.
    if (NewState == EFlightState::Grounded)
    {
        CurrentThrustSpeed = 0.f;
        TargetThrustSpeed = 0.f;

        TargetBankAngle = 0.f;

        RawPitchInput = 0.f;
        RawYawInput = 0.f;

        bTakeoffAssistActive = false;
        TakeoffElapsedTime = 0.f;
        AppliedTakeoffLift = 0.f;

        SmoothedYawInput = 0.f;
    }


    // Set Glide speed while preserving slower existing momentum.
    if (NewState == EFlightState::Gliding)
    {
        TargetThrustSpeed =
            FMath::Min(
                CurrentThrustSpeed,
                GlideSpeed);


        bTakeoffAssistActive = false;
        TakeoffElapsedTime = 0.f;
        AppliedTakeoffLift = 0.f;
    }


    // Start the takeoff assist only when leaving the ground.
    if (PreviousState == EFlightState::Grounded &&
        NewState == EFlightState::PoweredFlight)
    {
        bTakeoffAssistActive = true;
        TakeoffElapsedTime = 0.f;
        AppliedTakeoffLift = 0.f;
    }


    UE_LOG(
        LogTemp,
        Verbose,
        TEXT("Flight State: %s"),
        *UEnum::GetValueAsString(CurrentFlightState));
}


void ABirdPawn::CalculateGlideMovement(float DeltaTime)
{
    // Apply constant downward movement while gliding.
    const float DescentDistance =
        GlideDescentSpeed * DeltaTime;


    const FVector DeltaLocation(
        0.f,
        0.f,
        -DescentDistance);


    AddActorWorldOffset(
        DeltaLocation,
        true);
}


bool ABirdPawn::IsGroundNearby() const
{
    const FVector Start =
        GetActorLocation();


    const FVector End =
        Start -
        FVector(
            0.f,
            0.f,
            GroundCheckDistance);


    FHitResult HitResult;


    FCollisionQueryParams TraceParams;

    // Ignore the bird during the ground trace.
    TraceParams.AddIgnoredActor(this);


    return GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        TraceParams);
}


void ABirdPawn::UpdateCameraFOV(float DeltaTime)
{
    // Convert current flight speed into a normalized value.
    float SpeedAlpha =
        FMath::Clamp(
            CurrentThrustSpeed / MaxThrustSpeed,
            0.f,
            1.f);


    // Calculate the target FOV based on speed.
    float TargetFOV =
        FMath::Lerp(
            BaseFOV,
            MaxSpeedFOV,
            SpeedAlpha);


    // Smoothly move the camera toward the target FOV.
    float SmoothedFOV =
        FMath::FInterpTo(
            Camera->FieldOfView,
            TargetFOV,
            DeltaTime,
            FOVInterpSpeed);


    Camera->SetFieldOfView(
        SmoothedFOV);
}


void ABirdPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);


    switch (CurrentFlightState)
    {
    case EFlightState::Grounded:
    {
        // Smoothly return the bird to a level grounded rotation.
        const FRotator CurrentRotation =
            GetActorRotation();


        const FRotator TargetRotation(
            0.f,
            CurrentRotation.Yaw,
            0.f);


        const FRotator SmoothRotation =
            FMath::RInterpTo(
                CurrentRotation,
                TargetRotation,
                DeltaTime,
                LandingRotationInterpSpeed);


        SetActorRotation(
            SmoothRotation);


        // Smoothly remove any remaining visual bank.
        CurrentBankAngle =
            FMath::FInterpTo(
                CurrentBankAngle,
                0.f,
                DeltaTime,
                LandingRotationInterpSpeed);


        BirdMesh->SetRelativeRotation(
            FRotator(
                BaseMeshRotation.Pitch + CurrentBankAngle,
                BaseMeshRotation.Yaw,
                BaseMeshRotation.Roll));


        break;
    }


    case EFlightState::PoweredFlight:
    {
        CalculateTakeoffMovement(
            DeltaTime);


        CalculateForwardMovement(
            DeltaTime);


        CalculateSteering(
            DeltaTime);


        break;
    }


    case EFlightState::Gliding:
    {
        CalculateForwardMovement(
            DeltaTime);


        CalculateGlideMovement(
            DeltaTime);


        CalculateSteering(
            DeltaTime);


        // Land when the bird reaches the configured ground distance.
        if (IsGroundNearby())
        {
            SetFlightState(
                EFlightState::Grounded);
        }


        break;
    }
    }


    // Update speed-based camera FOV every frame.
    UpdateCameraFOV(DeltaTime);
}


void ABirdPawn::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(
        PlayerInputComponent);


    if (UEnhancedInputComponent* EnhancedInputComponent =
        Cast<UEnhancedInputComponent>(
            PlayerInputComponent))
    {
        // Bind powered flight input.
        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(
                MoveAction,
                ETriggerEvent::Triggered,
                this,
                &ABirdPawn::Move);


            EnhancedInputComponent->BindAction(
                MoveAction,
                ETriggerEvent::Completed,
                this,
                &ABirdPawn::Move);
        }


        // Bind mouse steering input.
        if (LookAction)
        {
            EnhancedInputComponent->BindAction(
                LookAction,
                ETriggerEvent::Triggered,
                this,
                &ABirdPawn::Look);
        }
    }
}


EFlightState ABirdPawn::GetFlightState() const
{
    return CurrentFlightState;
}