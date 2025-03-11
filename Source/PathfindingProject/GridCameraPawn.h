#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"      
#include "GameFramework/SpringArmComponent.h" 
#include "GridCameraPawn.generated.h"

UCLASS()
class PATHFINDINGPROJECT_API AGridCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AGridCameraPawn();

	// Moves the camera left or right based on the input
	void AdjustCameraRotation(float Value);

	// Sets the orbit center (the grid's center) that the camera should target
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetOrbitCenter(const FVector& NewCenter) { OrbitCenter = NewCenter; }

protected:
	virtual void BeginPlay() override;

public:    
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Editable orbit radius (how far the camera should orbit around the center
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	float Radius = 1000.0f;

private:

	// Represents the in-game camera object to be created
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* Camera;

	// A spring arm used for managing collisions and adjusting the camera's distance
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* SpringArm;

	// Controls how fast the camera moves in its orbit
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	float RotationSpeed = 50.0f;

	// Default orbit center set here but updated by SetOrbitCenter
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	FVector OrbitCenter = FVector::ZeroVector;

	// Tracks the current rotation angle of the camera in its orbit around the grid
	float CurrentAngle;
};
