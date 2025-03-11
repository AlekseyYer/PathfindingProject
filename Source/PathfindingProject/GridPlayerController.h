#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "UGridNode.h"
#include "GridPlayerController.generated.h"

UCLASS()
class PATHFINDINGPROJECT_API AGridPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaSeconds) override;

	// Input handling functions
	void HandleCameraRotation(const FInputActionValue& Value);
	void HandleLeftClick(const FInputActionValue& Value);
	void HandleRightClick(const FInputActionValue& Value);

	// Helper to get the instance index of the tile under the mouse cursor
	bool GetInstanceUnderCursor(int32& OutInstanceIndex) const;

	// Constructs the visual A* path
	void DrawPath();

private:
	// Holds a reference to the camera pawn used for rotation
	TObjectPtr<class AGridCameraPawn> CameraPawn;

	// Stores the mapping context for camera input
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class UInputMappingContext> CameraControlContext;

	// Stores the action that triggers camera rotation, like A or D
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class UInputAction> RotateCameraAction;

	// Stores the mapping context for click input
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class UInputMappingContext> ClickMappingContext;

	// Stores the action for left mouse click, used to set or clear start/goal nodes
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class UInputAction> LeftClickAction;

	// Stores the action for right mouse click, used to toggle obstacles
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class UInputAction> RightClickAction;

	// Stores a reference to the grid action in the scene
	UPROPERTY(EditInstanceOnly, Category = "Grid")
	TObjectPtr<class AActor> GridActor;

	// Stores the indices of grid tiles marked as obstacles
	TSet<int32> ObstacleIndices;

public:

	//Indicate that no start/goal node has been selected by default
	int32 StartNodeIndex = -1;
	int32 GoalNodeIndex = -1;

	// Clears start/goal nodes and obstacles
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void ResetGridState();

	// Stores a reference to our grid widget
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> GridControlWidgetClass;
	// Stores a reference to our grid class, which will be spawned by the player controller
	UPROPERTY(EditDefaultsOnly, Category = "Grid")
	TSubclassOf<class AGrid> GridClass;
};
