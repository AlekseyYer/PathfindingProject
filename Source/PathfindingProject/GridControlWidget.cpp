#include "GridControlWidget.h"
#include "GridPlayerController.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Grid.h"

void UGridControlWidget::OnGridCountChanged(int32 NewCount)
{
    if (ControlledGrid)
    {
        // Regenerate the grid with the new grid size
        ControlledGrid->GridCount = NewCount;
        ControlledGrid->UpdateGrid();
        UE_LOG(LogTemp, Log, TEXT("GridControlWidget: Changed grid count to %d and updated grid."), NewCount);

        // Resets all other properties of the grid through the player controller
        if (AGridPlayerController* PC = Cast<AGridPlayerController>(GetOwningPlayer()))
        {
            PC->ResetGridState();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GridControlWidget: No Grid set"));
    }
}

void UGridControlWidget::OnRandomizeButtonClicked()
{
    if (!ControlledGrid)
    {
        UE_LOG(LogTemp, Warning, TEXT("GridControlWidget: Grid is not set"));
        return;
    }

    // Call's the grid's randomize function to generate a new grid
    ControlledGrid->RandomizeGrid();
}


void UGridControlWidget::OnRandomizeWeightsButtonClicked()
{
    // Call's the grid's randomize weights function to set new movement costs for each tile
    if (ControlledGrid)
    {
        ControlledGrid->RandomizeWeights();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GridControlWidget: Grid is not set"));
    }
}
