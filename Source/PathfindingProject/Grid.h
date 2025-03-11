#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "Grid.generated.h"

// Forward declarations.
class UGridNode;
class UInstancedStaticMeshComponent;
class UStaticMesh;
class UTextRenderComponent; 

UCLASS()
class PATHFINDINGPROJECT_API AGrid : public AActor
{
    GENERATED_BODY()
    
public:    
    AGrid();

    //Stores the number of tiles per row/column in the grid
    UPROPERTY(EditAnywhere, Category = "Grid")
    int GridCount;

    // Instanced mesh reference to represent tile meshes
    UPROPERTY(EditAnywhere, Category = "Grid")
    UInstancedStaticMeshComponent* InstancedMesh;

    // Mesh used to display the hex tile
    UPROPERTY(EditAnywhere, Category = "Grid")
    UStaticMesh* TileMesh;

    // Returns the center of the grid.
    FVector GetGridCenter() const { return GridCenter; }

    // A* Pathfinding function that finds the path between a start and goal tile
    TArray<UGridNode*> FindPath(int32 StartInstanceIndex, int32 GoalInstanceIndex);

    // Regenerates the grid when changes are made
    void UpdateGrid();

    // Toggles a node’s obstacle state.
    void SetNodeObstacle(int32 InstanceIndex, bool bObstacle);

    // Randomly assigns an obstacle state to tiles in the grid
    void RandomizeObstacles(float ObstacleChance, int32 ExcludeIndex1 = -1, int32 ExcludeIndex2 = -1);

    // Assigns random movement costs to tiles
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void RandomizeWeights();

    // Regenerates the grid with randomized start/goal nodes and obstacles
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void RandomizeGrid();


protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Tick(float DeltaTime) override;

    // Generates the base grid by placing the tiles and initializing their data
    void GenerateGrid();
    
    // Deletes all text components (weight values) currently attached to the grid
    void ClearTextComponents();

    // Generates a text component for a specific tile to display its weight
    UTextRenderComponent* CreateTextComponentForNode(UGridNode* Node);
    
    // Connects tiles to their adjacent neighbors
    void BuildNeighbors();

private:
    // A 2D array of UGridNode pointers, each corresponding to a tile in the grid
    TArray<TArray<UGridNode*>> GridNodes;

    /*A hash map that maps the index of each tile in the Instanced Static Mesh Component
      to its corresponding UGridNode*/
    TMap<int32, UGridNode*> NodeMap;

    // Stores the center point of the grid 
    FVector GridCenter;

    // Stores an array of Text Render Components for the tiles
    TArray<UTextRenderComponent*> NodeTextComponents;
};
