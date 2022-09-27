// Fill out your copyright notice in the Description page of Project Settings.


#include "VehiclePawn.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

AVehiclePawn::AVehiclePawn() {
	UChaosWheeledVehicleMovementComponent* Vehicle = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());
	
	Vehicle->EngineSetup.MaxRPM = 10000.0f;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.0f;
	SpringArm->bUsePawnControlRotation = true;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Vehicle Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	SpringArm2 = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm2"));
	SpringArm2->SetupAttachment(RootComponent);
	SpringArm2->bDoCollisionTest = false;
	SpringArm2->bInheritPitch = false;
	SpringArm2->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	SpringArm2->TargetArmLength = 400.0f;
	SpringArm2->bUsePawnControlRotation = true;

	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Minimap"));
	CaptureComponent->SetupAttachment(SpringArm2, USpringArmComponent::SocketName);
	CaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	CaptureComponent->OrthoWidth = 7500.0f;
}

void AVehiclePawn::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	UpdateInAirControl(DeltaTime);
}

void AVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Throttle", this, &AVehiclePawn::SetThrottle);
	PlayerInputComponent->BindAxis("Steering", this, &AVehiclePawn::SetSteering);
	PlayerInputComponent->BindAxis("Brake", this, &AVehiclePawn::SetBrake);
	PlayerInputComponent->BindAxis("LookUp", this, &AVehiclePawn::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &AVehiclePawn::Turn);

	PlayerInputComponent->BindAction("Handbrake", IE_Pressed, this, &AVehiclePawn::SetHandBrakePressed);
	PlayerInputComponent->BindAction("Handbrake", IE_Released, this, &AVehiclePawn::SetHandBrakeReleased);
}

void AVehiclePawn::SetThrottle(float value) {
	GetVehicleMovementComponent()->SetThrottleInput(value);
}

void AVehiclePawn::SetSteering(float value) {
	GetVehicleMovementComponent()->SetSteeringInput(value);
}

void AVehiclePawn::SetBrake(float value) {
	GetVehicleMovementComponent()->SetBrakeInput(value);
}

void AVehiclePawn::SetHandBrakePressed() {
	GetVehicleMovementComponent()->SetHandbrakeInput(true);
}

void AVehiclePawn::SetHandBrakeReleased() {
	GetVehicleMovementComponent()->SetHandbrakeInput(false);
}

void AVehiclePawn::LookUp(float value)
{
	if (value != 0.f)
	{
		AddControllerPitchInput(value);
	}
}

void AVehiclePawn::Turn(float value)
{
	if (value != 0.f)
	{
		AddControllerYawInput(value);
	}
}


void AVehiclePawn::UpdateInAirControl(float DeltaTime)
{
	if (UChaosWheeledVehicleMovementComponent* Vehicle = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement()))
	{
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		const FVector TraceStart = GetActorLocation() + FVector(0.f, 0.f, 50.f);
		const FVector TraceEnd = GetActorLocation() - FVector(0.f, 0.f, 200.f);

		FHitResult Hit;

		const bool bInAir = !GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
		const bool bNotGrounded = FVector::DotProduct(GetActorUpVector(), FVector::UpVector) < 0.1f;

		if (bInAir || bNotGrounded)
		{
			const float ForwardInput = InputComponent->GetAxisValue("Throttle");
			const float RightInput = InputComponent->GetAxisValue("Steering");

			const float AirMovementForcePitch = 3.f;
			const float AirMovementForceRoll = !bInAir && bNotGrounded ? 20.f : 3.f;

			if (UPrimitiveComponent* VehicleMesh = Vehicle->UpdatedPrimitive)
			{
				const FVector MovementVector = FVector(RightInput * -AirMovementForceRoll, ForwardInput * AirMovementForcePitch, 0.f) * DeltaTime * 200.f;
				const FVector NewAngularMovement = GetActorRotation().RotateVector(MovementVector);

				VehicleMesh->SetPhysicsAngularVelocityInDegrees(NewAngularMovement, true);
			}
		}
	}
}