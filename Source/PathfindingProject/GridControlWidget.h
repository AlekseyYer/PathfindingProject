#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GridControlWidget.generated.h"

// Forward declaration.
class AGrid;

UCLASS()
class PATHFINDINGPROJECT_API UGridControlWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Updates the grid's count (size) when the slider value changes
	UFUNCTION(BlueprintCallable, Category = "Grid Control")
	void OnGridCountChanged(int32 NewCount);

	// Reference to the grid actor this widget controls
	UPROPERTY(BlueprintReadWrite, Category = "Grid Control")
	AGrid* ControlledGrid;

	// Called when the "Randomize Grid" button is pressed to generate a new randomized grid
	UFUNCTION(BlueprintCallable, Category = "Grid Control")
	void OnRandomizeButtonClicked();

	// Called when the "Randomize Weights" button is pressed to randomize the movement cost of each tile
	UFUNCTION(BlueprintCallable, Category = "Grid Control")
	void OnRandomizeWeightsButtonClicked();
};
