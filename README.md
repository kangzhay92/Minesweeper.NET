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
4. The output file is in WPF/bin folder

## Features
1. Classic user interface, like the original application on early version of Windows.
2. Highscore system, stored in persistent data.
3. Three levels difficulty (Beginner, Advanced, and Expert), and custom level.

![Image1](https://user-images.githubusercontent.com/13499151/98462812-67f2e400-21e9-11eb-98a4-6789df96417f.png)
![Image2](https://user-images.githubusercontent.com/13499151/98462817-7a6d1d80-21e9-11eb-88d5-f8364e612af6.png)
![Image3](https://user-images.githubusercontent.com/13499151/98462830-8822a300-21e9-11eb-956f-465bac1d82cf.png)

## License
Minesweeper.NET is licensed under MIT License, see [LICENSE](https://github.com/kangzhay92/Minesweeper.NET/blob/master/LICENSE) for more information.
