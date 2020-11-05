# Minesweeper.NET
Remake of old school puzzle game using .NET Framework. Has a standalone game logic library, 
so it only needs to be integrated with the UI framework. See another branch for the old native version.

## How to play
To win the game complete all boxes from the board containing hidden mines without exploding one of them, 
with help from clues about the number of neighboring mines in each box. Boxes have three states: uncompleted, completed and flagged. 
An uncompleted box is clickable. Flagged boxes is marked by the player to indicate a potential mine location, and its unclickable.
Press left mouse button to complete a box, right mouse button to add/clear box flag. The first completed box will never be a mine.

## How to build
1. Download and Install [Visual Studio 2019 Community Edition](https://visualstudio.microsoft.com/vs/community/) from Microsoft site
2. Add .NET desktop development packages and C++/CLI support from Visual Studio Installer
3. After install completed open the project file and select Build Solution from Build menu

