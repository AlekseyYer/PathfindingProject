#include "UGridNode.h"
#include "Math/UnrealMathUtility.h"

UGridNode::UGridNode()
{
	// Indicates that the node is not assigned to a grid position yet
	GridX = -1;
	GridY = -1;

	// Updated during grid generation
	WorldPosition = FVector::ZeroVector; 
	InstanceIndex = -1;
	
	Weight = FMath::RandRange(1.0f, 5.0f); // Random movement cost

	// Updated in FindPath()
	GCost = 0.0f;
	HCost = 0.0f;
	ParentNode = nullptr;
	bIsObstacle = false; // Neutral by default
}

// Toggles a node's obstacle state
void UGridNode::SetObstacle(bool bObstacle)
{
	bIsObstacle = bObstacle;
}

void UGridNode::FindNeighbors(const TArray<TArray<UGridNode*>>& Grid, int GridSizeX, int GridSizeY)
{
	Neighbors.Empty(); // Clear the array, ensuring we don't carry over neighbors from previous grid

	if (GridY % 2 == 0) // Checks for even rows. Each offset designates how far to move in x and y directions
	{
		int EvenOffsets[6][2] = {
			{-1,  0}, // Left
			{ 1,  0}, // Right
			{ 0, -1}, // Bottom Left
			{-1, -1}, // Top Left
			{ 0,  1}, // Bottom Right
			{-1,  1} // Top Right
		};

		for (int i = 0; i < 6; i++)
		{
			//Applies the offset to find the neighbors
			int32 NeighborX = GridX + EvenOffsets[i][0];
			int32 NeighborY = GridY + EvenOffsets[i][1];

			// Checks for out of bound neighbors
			if (NeighborX >= 0 && NeighborX < GridSizeX && NeighborY >= 0 && NeighborY < GridSizeY)
			{
				// Retrieves the node at that location, and sets it as a neighbor
				UGridNode* Neighbor = Grid[NeighborX][NeighborY];
				if (Neighbor && !Neighbor->bIsObstacle)
				{
					// Adds it to the neighbor array
					Neighbors.Add(Neighbor);
				}
			}
		}
	}
	else // Odd rows that were given a shift. The values are slightly different therefore
	{
		int OddOffsets[6][2] = {
			{-1,  0}, // Left
			{ 1,  0}, // Right
			{ 0, -1}, // Bottom Left
			{ 1, -1}, // Top Left
			{ 0,  1}, // Bottom Right
			{ 1,  1} // Top Right
		};

		for (int i = 0; i < 6; i++)
		{
			int32 NeighborX = GridX + OddOffsets[i][0];
			int32 NeighborY = GridY + OddOffsets[i][1];

			if (NeighborX >= 0 && NeighborX < GridSizeX && NeighborY >= 0 && NeighborY < GridSizeY)
			{
				UGridNode* Neighbor = Grid[NeighborX][NeighborY];
				if (Neighbor && !Neighbor->bIsObstacle)
				{
					Neighbors.Add(Neighbor);
				}
			}
		}
	}
}
