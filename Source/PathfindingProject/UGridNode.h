#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UGridNode.generated.h"

UCLASS()
class PATHFINDINGPROJECT_API UGridNode : public UObject
{
	GENERATED_BODY()
    
public:
	UGridNode();

	//Position of the node
	int32 GridX;
	int32 GridY;

	// 3D World Location
	FVector WorldPosition;

	// Instance index corresponding to the instanced mesh
	int32 InstanceIndex;

	// Random weight (movement cost) used for weighted pathfinding
	float Weight;

	// A* costs.
	float GCost; // Movement cost from start node to current (this) node
	float HCost; // Estimated cost from start node to goal through straight line distance
	float FCost() const { return GCost + HCost; } // F Cost is used to select the shortest path

	// Parent node for path reconstruction
	UPROPERTY()
	UGridNode* ParentNode;

	// Indicates whether this node is an obstacle
	bool bIsObstacle;

	// Array of node's adjacent neighbors
	TArray<UGridNode*> Neighbors;

	// Marks or unmarks this node as an obstacle
	void SetObstacle(bool bObstacle);

	// Finds the node's adjacent neighbors through the use of positions and offsets
	void FindNeighbors(const TArray<TArray<UGridNode*>>& Grid, int GridSizeX, int GridSizeY);
};
