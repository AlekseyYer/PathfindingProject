#include "GridCameraPawn.h"
#include "Kismet/KismetMathLibrary.h"

AGridCameraPawn::AGridCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm")); // Spring arm object creation
	SpringArm->SetupAttachment(RootComponent); // Attaches the spring arm to the root component
	SpringArm->TargetArmLength = 1000.0f;

	// Disables collision and smoothing effects
	SpringArm->bDoCollisionTest = false; 
	SpringArm->bEnableCameraLag = false;

	// Creates the camera object and has its rotation set to look down at the grid
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));

	// Stores the current rotational position
	CurrentAngle = 0.0f;

	// Determines how far the camera stays from the orbit center.
	Radius = 1000.0f;
}

void AGridCameraPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AGridCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Center = OrbitCenter;

	// Gives us the horizontal offset from the center
	float X = FMath::Cos(FMath::DegreesToRadians(CurrentAngle)) * Radius;

	// Gives us the vertical offset from the center
	float Y = FMath::Sin(FMath::DegreesToRadians(CurrentAngle)) * Radius;

	// This is the location that the camera should move to
	FVector TargetLocation = Center + FVector(X, Y, 0.0f);

	// We interpolate the camera from its current position to the TargetLocation
	FVector NewLocation = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, 5.0f);
	SetActorLocation(NewLocation);

	// Calculates the rotation the camera should face in order to always look at the grid center
	FRotator TargetRotation = FRotationMatrix::MakeFromX(Center - NewLocation).Rotator();

	// Applies this rotation and interpolates from the current rotation to the target
	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 5.0f);
	SetActorRotation(NewRotation);
}

void AGridCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AGridCameraPawn::AdjustCameraRotation(float Value)
{
	// Takes in the player's input and moves the camera
	CurrentAngle += Value * RotationSpeed;

	// Caps the angle between 0 and 360 to keep the values clean and avoid floating-point drift
	CurrentAngle = FMath::Fmod(CurrentAngle, 360.0f);
}
