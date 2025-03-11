#include "GridPlayerController.h"
#include "GridCameraPawn.h"
#include "Grid.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Blueprint/UserWidget.h"
#include "GridControlWidget.h"
#include "EnhancedInputSubsystems.h"

void AGridPlayerController::BeginPlay()
{
    Super::BeginPlay();

    bShowMouseCursor = true; // Allows the mouse cursor to be visible
    SetInputMode(FInputModeGameAndUI()); // Allows player to interact with in-game elements and UI

    // Get the camera pawn and cast it to our camera class
    CameraPawn = Cast<AGridCameraPawn>(GetPawn());
    
    if (GridClass)
    {
        // Spawn the grid actor 
        FTransform SpawnTransform;
        SpawnTransform.SetLocation(FVector::ZeroVector);
        GridActor = GetWorld()->SpawnActor<AGrid>(GridClass, SpawnTransform);
        
        if (GridActor)
        {
            AGrid* Grid = Cast<AGrid>(GridActor);
            if (Grid && CameraPawn)
            {
                //Get the grid center, and set the camera to orbit around that point
                FVector GridCenter = Grid->GetGridCenter();
                CameraPawn->SetOrbitCenter(GridCenter);
                UE_LOG(LogTemp, Log, TEXT("Camera targeting OrbitCenter: %s"), *GridCenter.ToString());

                // Sets the camera's position above the grid
                FVector InitialLocation = GridCenter + FVector(CameraPawn->Radius, 0, 300);
                CameraPawn->SetActorLocation(InitialLocation);
            }
        }
    }

    if (GridControlWidgetClass)
    {
        // Call create widget to instantiate the UI widget
        UUserWidget* Widget = CreateWidget<UUserWidget>(this, GridControlWidgetClass);
        if (Widget)
        {
            // Add the widget to the player's screen 
            Widget->AddToViewport();

            // Cast the UWidget to our widget class and assign the grid actor to the controlled grid property in the widget
            if (auto* GridWidget = Cast<class UGridControlWidget>(Widget))
            {
                GridWidget->ControlledGrid = Cast<AGrid>(GridActor); // Allows our widget to interact with the grid
            }
        }
    }
    

    if (ULocalPlayer* LocalPlayer = GetLocalPlayer()) // Retrieve the player controlling this instance of the game
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            // Add our mapping contexts 
            Subsystem->AddMappingContext(CameraControlContext, 0);
            Subsystem->AddMappingContext(ClickMappingContext, 1);
        }
    }
}

void AGridPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (auto* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
    {
        // Bind our actions to the proper functions, allowing input to execute logic
        EnhancedInput->BindAction(RotateCameraAction, ETriggerEvent::Triggered, this, &AGridPlayerController::HandleCameraRotation);

        // ::Started used to avoid continuous firing while the left/right mouse button is held down
        EnhancedInput->BindAction(LeftClickAction, ETriggerEvent::Started, this, &AGridPlayerController::HandleLeftClick);
        EnhancedInput->BindAction(RightClickAction, ETriggerEvent::Started, this, &AGridPlayerController::HandleRightClick);
    }
}

void AGridPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    DrawPath(); // Draws the path visualizer each frame
}

void AGridPlayerController::HandleCameraRotation(const FInputActionValue& Value)
{
    if (CameraPawn)
    {
        float InputValue = Value.Get<float>(); // Extracts the float value from the player's input (1 for A, -1 for D)
        CameraPawn->AdjustCameraRotation(50.0f * InputValue); // Passes in our input to our camera's movement function
    }
}

void AGridPlayerController::HandleLeftClick(const FInputActionValue& Value)
{
    int32 InstanceIndex; // Stores the index of the tile under the mouse cursor
    if (GetInstanceUnderCursor(InstanceIndex))
    {
        // Retrieve the instanced static mesh component of the Grid Actor
        UInstancedStaticMeshComponent* InstancedMesh = Cast<UInstancedStaticMeshComponent>(GridActor->GetComponentByClass(UInstancedStaticMeshComponent::StaticClass()));
        if (!InstancedMesh)
        {
            return;
        }

        // If the player left-clicks on the start tile, reset it to a default tile
        if (InstanceIndex == StartNodeIndex)
        {
            InstancedMesh->SetCustomDataValue(InstanceIndex, 0, 0.0f, true);
            StartNodeIndex = -1;
        }
        // If the player left-clicks on the goal tile, reset it to a default tile
        else if (InstanceIndex == GoalNodeIndex)
        {
            InstancedMesh->SetCustomDataValue(InstanceIndex, 0, 0.0f, true);
            GoalNodeIndex = -1;
        }
        else
        {
            // If the start node hasn't been set and the player left-clicks on a tile, mark it as the start node
            if (StartNodeIndex == -1)
            {
                StartNodeIndex = InstanceIndex;
                InstancedMesh->SetCustomDataValue(InstanceIndex, 0, 1.0f, true);
                UE_LOG(LogTemp, Log, TEXT("Set Start Tile: InstanceIndex %d"), InstanceIndex);
            }

            // If the goal node hasn't been set and the tile that is clicked on is not the start node, mark this tile as the goal
            else if (GoalNodeIndex == -1 && InstanceIndex != StartNodeIndex)
            {
                GoalNodeIndex = InstanceIndex;
                InstancedMesh->SetCustomDataValue(InstanceIndex, 0, 2.0f, true);
                UE_LOG(LogTemp, Log, TEXT("Set Goal Tile: InstanceIndex %d"), InstanceIndex);
            }
        }
    }
}

void AGridPlayerController::HandleRightClick(const FInputActionValue& Value)
{
    int32 InstanceIndex;
    if (GetInstanceUnderCursor(InstanceIndex))
    {
        UInstancedStaticMeshComponent* InstancedMesh = Cast<UInstancedStaticMeshComponent>(GridActor->GetComponentByClass(UInstancedStaticMeshComponent::StaticClass()));
        if (!InstancedMesh)
        {
            return;
        }

        AGrid* Grid = Cast<AGrid>(GridActor);
        if (!Grid)
        {
            return;
        }

        // If the player right-clicks on an obstacle, reset its status to a default node and change its appearance
        if (ObstacleIndices.Contains(InstanceIndex))
        {
            ObstacleIndices.Remove(InstanceIndex);
            InstancedMesh->SetCustomDataValue(InstanceIndex, 0, 0.0f, true);
            Grid->SetNodeObstacle(InstanceIndex, false);
            UE_LOG(LogTemp, Log, TEXT("Reset to Default: InstanceIndex %d"), InstanceIndex);
        }
        // Otherwise, a right-click will mark a tile as an obstacle
        else
        {
            ObstacleIndices.Add(InstanceIndex);
            InstancedMesh->SetCustomDataValue(InstanceIndex, 0, 3.0f, true);
            Grid->SetNodeObstacle(InstanceIndex, true);
            UE_LOG(LogTemp, Log, TEXT("Set Obstacle Tile: InstanceIndex %d"), InstanceIndex);
        }
    }
}

bool AGridPlayerController::GetInstanceUnderCursor(int32& OutInstanceIndex) const
{
    // Store the collision details of a raycast
    FHitResult HitResult;
    GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

    // If the raycast hit something
    if (HitResult.Component.IsValid())
    {
        // And if the component hit by the raycast is the proper component, extract its index
        if (UInstancedStaticMeshComponent* InstancedMesh = Cast<UInstancedStaticMeshComponent>(HitResult.Component))
        {
            OutInstanceIndex = HitResult.Item;
            return true;
        }
    }
    // If no tile was found under the cursor
    return false;
}

void AGridPlayerController::DrawPath()
{
    AGrid* Grid = Cast<AGrid>(GridActor);
    if (!Grid)
    {
        return;
    }

    // Check that both the start and goal nodes are set
    if (StartNodeIndex != -1 && GoalNodeIndex != -1)
    {
        UE_LOG(LogTemp, Log, TEXT("DrawPath: StartNodeIndex = %d, GoalNodeIndex = %d"), StartNodeIndex, GoalNodeIndex);

        // Run the A* algorithm and store the path 
        TArray<UGridNode*> Path = Grid->FindPath(StartNodeIndex, GoalNodeIndex);
        
        UE_LOG(LogTemp, Log, TEXT("DrawPath: Computed path with %d nodes."), Path.Num());

        // Ensure that the path has at least two nodes before drawing
        if (Path.Num() > 1)
        {
            for (int32 i = 0; i < Path.Num() - 1; i++)
            {
                FVector StartLocation = Path[i]->WorldPosition; // Location of the current node
                FVector EndLocation = Path[i + 1]->WorldPosition; // Location of the next node
                float LineOffset = 50.0f; // Places the visual line above the grid
                StartLocation.Z += LineOffset;
                EndLocation.Z += LineOffset;

                // Draws the debug line between each node until it completes the path
                DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Green, false, 0.1f, 0, 5.0f);

                UE_LOG(LogTemp, Log, TEXT("DrawPath: Drawing line from (%.2f, %.2f, %.2f) to (%.2f, %.2f, %.2f)"),
                    StartLocation.X, StartLocation.Y, StartLocation.Z,
                    EndLocation.X, EndLocation.Y, EndLocation.Z);
            }
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("DrawPath: Path has fewer than 2 nodes, no debug line drawn."));
        }
    }
}

void AGridPlayerController::ResetGridState()
{
    // -1 Indicates that the start and goal nodes have not been set
    StartNodeIndex = -1;
    GoalNodeIndex = -1;
    ObstacleIndices.Empty(); // Clear the array of obstacles
}
