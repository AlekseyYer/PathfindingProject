# A* Pathfinding on a Hexagonal Grid in Unreal Engine 5
##  ** Where to find my source code **
- Click on Source - > PathfindingProject - > Feel free to check out my code :)
## Overview
This project is a custom implementation of the A* pathfinding algorithm in Unreal Engine 5 tailored for a hexagonal grid, built entirely from an empty project with a primary focus on C++. 

## Features
- **Custom A* Pathfinding Algorithm**
  - Implemented a fully functional A* pathfinding system in C++, visualizing the search process and final path.
- **Hexagonal Grid System**
  - Utilized a hex-based grid layout instead of a traditional square grid, introducing unique movement calculations.
- **Pathfinding Visualization**
  - A debug line follows the computed path from the start node to the goal while avoiding obstacles.
  - Each node displays its weight as a text indicator, showcasing the algorithm's ability to choose the best path based on multiple factors.
- **User-Controlled Grid Interaction**
  - Left-click to set the start and goal nodes.
  - Right-click to designate obstacle tiles that the pathfinder must navigate around.
  - UI panel features:
    - Randomization of start and end nodes.
    - Randomization of obstacles.
    - Randomization of node weights.

## Development Approach
- **C++ First** 
  - I prioritized C++, only using Blueprints for Widget functionality, to maximize performance and gain a deeper understanding of Unreal Engine's code architecture and capabilities.
- **Unreal Enhanced Input System**
  - Utilized to handle player interactions efficiently.
- **Camera System** 
  - Implemented a free-moving camera that follows a circular path around the grid, allowing for an intuitive view of the pathfinding process.

## Future Improvements
-  Optimization of the A* algorithm for larger grids through the use of a priority queue.
-  Additional pathfinding heuristics for varied movement behavior.
-  Updated UI and visual effects
