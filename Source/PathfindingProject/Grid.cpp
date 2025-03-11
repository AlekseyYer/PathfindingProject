#include "Grid.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "UGridNode.h"
#include "Math/UnrealMathUtility.h"
#include "GridPlayerController.h"
#include "Engine/World.h"

AGrid::AGrid()
{
    PrimaryActorTick.bCanEverTick = true;

    //Creates the root and instanced static mesh component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    InstancedMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedMesh"));

    //Attaches the mesh component to the root
    InstancedMesh->SetupAttachment(RootComponent);
    
    // Reserve one custom data float for each instance, allowing for setting weights or obstacle states
    InstancedMesh->NumCustomDataFloats = 1;

    GridCount = 10; // Default grid size.
}

void AGrid::BeginPlay()
{
    Super::BeginPlay();
}

void AGrid::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    GenerateGrid(); // Runs whenever the grid is placed in the scene and modified before runtime
}

void AGrid::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}


void AGrid::ClearTextComponents()
{
    //Loop through each text component pointer stored in the array
    for (UTextRenderComponent* TextComp : NodeTextComponents)
    {
        if (TextComp)
        {
            TextComp->DestroyComponent(); // If the text component exists, destroy it
        }
    }
    NodeTextComponents.Empty(); // Clear the array
}

/// 
/// @param Node The node to attach the text component to
/// @return The UTextRenderComponent with the weight value and position
UTextRenderComponent* AGrid::CreateTextComponentForNode(UGridNode* Node)
{
    UTextRenderComponent* TextComp = NewObject<UTextRenderComponent>(this);
    if (TextComp && Node)
    {
        // Convert weight to integer string
        FString WeightString = FString::Printf(TEXT("%d"), FMath::RoundToInt(Node->Weight));
        TextComp->SetText(FText::FromString(WeightString));

        // Set properties for readability.
        TextComp->SetHorizontalAlignment(EHTA_Center);
        TextComp->SetVerticalAlignment(EVRTA_TextCenter);
        TextComp->SetTextRenderColor(FColor::Red);
        TextComp->SetWorldSize(50.f);

        // Position text at the node's center, with a slight upward offset.
        FVector TextLocation = Node->WorldPosition;
        TextLocation.Z += 50.f;
        TextComp->SetWorldLocation(TextLocation);

        // Rotate text so it lies flat on the tile
        FRotator TextRotation = FRotator(90.f, 180.f, 0.f);
        TextComp->SetWorldRotation(TextRotation);

        // Attach to the grid's root and register, allowing it to appear during runtime
        TextComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
        TextComp->RegisterComponent();

        
    }
    return TextComp;
}


void AGrid::BuildNeighbors()
{
    //Loop through the grid array, and run the FindNeighbors function for each node
    for (int32 x = 0; x < GridNodes.Num(); x++)
    {
        for (int32 y = 0; y < GridNodes[x].Num(); y++)
        {
            GridNodes[x][y]->FindNeighbors(GridNodes, GridCount, GridCount);
        }
    }
}


void AGrid::GenerateGrid()
{
    if (!InstancedMesh)
    {
        return; // If the grid's instanced static mesh component does not exist, exit early
    }

    // Clear previous text components and grid data
    ClearTextComponents();
    InstancedMesh->ClearInstances();
    GridNodes.Empty();
    NodeMap.Empty();

    // Hex grid parameters for placing the tiles
    float HexRadius = 100.0f;
    float HorizontalShift = HexRadius * FMath::Sqrt(3.);
    float VerticalShift = HexRadius * 1.5f;

    // Bounding box initialization
    FVector MinPos(FLT_MAX, FLT_MAX, FLT_MAX);
    FVector MaxPos(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    // Create grid nodes
    for (int32 x = 0; x < GridCount; x++) // each column
    {
        TArray<UGridNode*> RowNodes;
        for (int32 y = 0; y < GridCount; y++) // each row
        {
            FVector TileLocation;
            if (y % 2 == 0) // if even numbered row, place tile without offset
                TileLocation = FVector(HorizontalShift * x, VerticalShift * y, 0.0f);
            else // otherwise, apply a horizontal shift 
                TileLocation = FVector(HorizontalShift * x + HorizontalShift / 2.0f, VerticalShift * y, 0.0f);

            //Stores the transform of the tile with the proper location and zero rotation
            FTransform TileTransform(FRotator::ZeroRotator, TileLocation);

            //Creates a new instance of the tile mesh and applies a neutral color
            int32 InstanceIndex = InstancedMesh->AddInstance(TileTransform); // returns the index of the new tile
            InstancedMesh->SetCustomDataValue(InstanceIndex, 0, 0.0f, true);

            //Updates MinPos to find the smallest coordinates of the grid
            MinPos.X = FMath::Min(MinPos.X, TileLocation.X);
            MinPos.Y = FMath::Min(MinPos.Y, TileLocation.Y);
            MinPos.Z = FMath::Min(MinPos.Z, TileLocation.Z);

            //Updates MaxPos to find the largest coordinates of the grid
            MaxPos.X = FMath::Max(MaxPos.X, TileLocation.X);
            MaxPos.Y = FMath::Max(MaxPos.Y, TileLocation.Y);
            MaxPos.Z = FMath::Max(MaxPos.Z, TileLocation.Z);

            //Generates a new instance of UGridNode at runtime and initialize its values
            UGridNode* NewNode = NewObject<UGridNode>(this);
            NewNode->GridX = x;
            NewNode->GridY = y;
            NewNode->WorldPosition = TileLocation;
            NewNode->InstanceIndex = InstanceIndex;

            //Adds the new node to the array and maps its index
            RowNodes.Add(NewNode);
            NodeMap.Add(InstanceIndex, NewNode);

            // Create and attach a text component to display the node's weight
            if (UTextRenderComponent* TextComp = CreateTextComponentForNode(NewNode))
            {
                NodeTextComponents.Add(TextComp);
            }
        }
        //Stores the row of UGridNode objects in this array, forming the 2D grid structure
        GridNodes.Add(RowNodes);
    }

    // Compute the grid center from bounding box
    GridCenter = (MinPos + MaxPos) * 0.5f;
    UE_LOG(LogTemp, Log, TEXT("AGrid::GenerateGrid: Computed GridCenter = %s"), *GridCenter.ToString());

    // Calls the function to link adjacent tiles
    BuildNeighbors();
}

TArray<UGridNode*> AGrid::FindPath(int32 StartInstanceIndex, int32 GoalInstanceIndex)
{
    // Stores the tiles that lead to the goal node
    TArray<UGridNode*> Path;

    // Look up the index of the start and goal node, returning a pointer-to-pointer
    UGridNode** StartNodePtr = NodeMap.Find(StartInstanceIndex);
    UGridNode** GoalNodePtr = NodeMap.Find(GoalInstanceIndex);
    if (!StartNodePtr || !GoalNodePtr)
    {
        // If either node not found, exit early
        UE_LOG(LogTemp, Warning, TEXT("FindPath: Could not find Start or Goal node."));
        return Path;
    }

    //Dereference the start/goal pointers to get the actual nodes
    UGridNode* StartNode = *StartNodePtr;
    UGridNode* GoalNode = *GoalNodePtr;

    // Log the start and goal node's world position
    UE_LOG(LogTemp, Log, TEXT("FindPath: StartNode at (%.2f, %.2f, %.2f), GoalNode at (%.2f, %.2f, %.2f)"),
        StartNode->WorldPosition.X, StartNode->WorldPosition.Y, StartNode->WorldPosition.Z,
        GoalNode->WorldPosition.X, GoalNode->WorldPosition.Y, GoalNode->WorldPosition.Z);

    // Reset costs and parent pointers
    for (int32 x = 0; x < GridNodes.Num(); x++)
    {
        for (int32 y = 0; y < GridNodes[x].Num(); y++)
        {
            UGridNode* Node = GridNodes[x][y];

            //G and HCost are set to infinity so that the nodes being searched will be updated
            Node->GCost = TNumericLimits<float>::Max(); 
            Node->HCost = TNumericLimits<float>::Max();
            Node->ParentNode = nullptr;
        }
    }

    // No movement cost since we start from this node
    StartNode->GCost = 0;

    // Calculates the HCost as the Euclidean distance from the start to the goal node
    StartNode->HCost = FVector::Dist(StartNode->WorldPosition, GoalNode->WorldPosition);

    
    TArray<UGridNode*> OpenSet; // Stores nodes that will have their FCosts compared
    TSet<UGridNode*> ClosedSet; // Stores nodes that 
    OpenSet.Add(StartNode); // Adds the start node to be examined first

    // Main A* loop.
    while (OpenSet.Num() > 0) // while there remain nodes to be examined 
    {
        UGridNode* CurrentNode = OpenSet[0]; // Current node is the first element in the array, starts with start node
        for (UGridNode* Node : OpenSet)
        {
            //Checks for lowest FCost among nodes, and if there is a tie, choose the lowest HCost
            if (Node->FCost() < CurrentNode->FCost() ||
                (FMath::IsNearlyEqual(Node->FCost(), CurrentNode->FCost()) && Node->HCost < CurrentNode->HCost))
            {
                CurrentNode = Node; // Best node is chosen
            }
        }
        
        if (CurrentNode == GoalNode)
        {
            UE_LOG(LogTemp, Log, TEXT("FindPath: Goal reached, reconstructing path."));
            UGridNode* PathNode = GoalNode;

            //If goal node found, reconstruct path by inserting each node and its parent node backwards
            while (PathNode != nullptr)
            {
                Path.Insert(PathNode, 0);
                PathNode = PathNode->ParentNode;
            }
            
            return Path;
        }

        // Remove the examined node from the open set and mark it as closed
        OpenSet.Remove(CurrentNode);
        ClosedSet.Add(CurrentNode);

        // Loop through the current node's neighbors
        for (UGridNode* Neighbor : CurrentNode->Neighbors)
        {
            // If the neighbor is an obstacle, or has been moved to closed set, skip this iteration (this node)
            if (Neighbor->bIsObstacle || ClosedSet.Contains(Neighbor))
            {
                continue;
            }

            // Calculate the base cost of moving to a node, determined by position
            float moveCost = FVector::Dist(CurrentNode->WorldPosition, Neighbor->WorldPosition);

            // Cost to reach the neighbor given the current GCost, the distance, and weight
            float TentativeGCost = CurrentNode->GCost + (moveCost * Neighbor->Weight);

            // This check passes for all nodes that are set to infinity. After that, we may update previously assigned GCosts
            if (TentativeGCost < Neighbor->GCost)
            {

                // Update the neighbor's values, overriding its default (infinity) values
                Neighbor->GCost = TentativeGCost;
                Neighbor->HCost = FVector::Dist(Neighbor->WorldPosition, GoalNode->WorldPosition);
                Neighbor->ParentNode = CurrentNode;

                // If the neighbor is not already in the open set, add it
                if (!OpenSet.Contains(Neighbor))
                {
                    OpenSet.Add(Neighbor);
                }
            }
        }
    }

    // Return an empty path 
    UE_LOG(LogTemp, Warning, TEXT("FindPath: No valid path found."));
    return Path;
}

// Regenerates the grid
void AGrid::UpdateGrid()
{
    GenerateGrid();
}

void AGrid::SetNodeObstacle(int32 InstanceIndex, bool bObstacle)
{
    if (UGridNode** NodePtr = NodeMap.Find(InstanceIndex)) // Search for the node 
    {
        (*NodePtr)->SetObstacle(bObstacle); // Update it's obstacle status
    }
}

void AGrid::RandomizeObstacles(float ObstacleChance, int32 ExcludeIndex1, int32 ExcludeIndex2)
{
    if (!InstancedMesh)
    {
        return;
    }

    int32 TotalNodes = InstancedMesh->GetInstanceCount();
    for (int32 i = 0; i < TotalNodes; i++)
    {
        if (i == ExcludeIndex1 || i == ExcludeIndex2) // Ensures that the start and goal nodes do not become obstacles
        {
            continue;
        }

        // Generate a random chance for a tile to become an obstacle. If it does, change its appearance
        bool bIsObstacle = (FMath::FRand() < ObstacleChance);
        SetNodeObstacle(i, bIsObstacle);
        float NewValue = bIsObstacle ? 3.0f : 0.0f;
        InstancedMesh->SetCustomDataValue(i, 0, NewValue, true);
    }
}

void AGrid::RandomizeWeights()
{
    int32 TotalNodes = InstancedMesh->GetInstanceCount();

    for (int32 i = 0; i < TotalNodes; i++)
    {
        if (UGridNode** NodePtr = NodeMap.Find(i)) // Find the corresponding UGridNode
        {
            UGridNode* Node = *NodePtr;
            Node->Weight = FMath::RandRange(1.0f, 5.0f); // Assign it a random weight
            if (NodeTextComponents.IsValidIndex(i))
            {
                FString WeightString = FString::Printf(TEXT("%d"), FMath::RoundToInt(Node->Weight));
                NodeTextComponents[i]->SetText(FText::FromString(WeightString)); // Set the text to represent the weight
            }
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Randomized weights for %d nodes"), TotalNodes);
}

void AGrid::RandomizeGrid()
{
    GridCount = FMath::RandRange(4, 10); //Random grid size
    UpdateGrid();

    int32 TotalNodes = GridCount * GridCount;
    if (TotalNodes < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("AGrid: Not enough nodes for random start/goal selection."));
        return;
    }

    // Select a random tile and set it to be the start goal
    int32 StartIndex = FMath::RandRange(0, TotalNodes - 1);
    int32 GoalIndex;

    // Select a random tile and set it to be the goal node while the tile selected is not the start node
    do
    {
        GoalIndex = FMath::RandRange(0, TotalNodes - 1);
    } while (GoalIndex == StartIndex);

    // Set the proper appearances for the start, goal, and neutral tiles
    if (InstancedMesh)
    {
        for (int32 i = 0; i < TotalNodes; i++)
        {
            InstancedMesh->SetCustomDataValue(i, 0, 0.0f, true);
        }
        InstancedMesh->SetCustomDataValue(StartIndex, 0, 1.0f, true);
        InstancedMesh->SetCustomDataValue(GoalIndex, 0, 2.0f, true);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AGrid: InstancedMesh not found."));
    }

    // Randomize the obstacles in the grid
    float ObstacleChance = 0.3f;
    RandomizeObstacles(ObstacleChance, StartIndex, GoalIndex);

    UE_LOG(LogTemp, Log, TEXT("AGrid: Randomized grid with grid count %d, start index %d, goal index %d."), GridCount, StartIndex, GoalIndex);

    if (AGridPlayerController* PC = Cast<AGridPlayerController>(GetWorld()->GetFirstPlayerController()))
    {
        // Update the player's Start and Goal node indices to reflect the new grid, allowing for pathfinding to begin
        PC->StartNodeIndex = StartIndex;
        PC->GoalNodeIndex = GoalIndex;
    }
}

