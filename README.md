# Squirrel On The Run 
Help the cute little squirrel named "Puschel" to collect all acorns  
to survive the winter with his friend the rainbow lizard "Echsi".  
It's just for fun and educational purpose. Feel free to modify, improve, and use it - also the assets :)
 
<img src="https://private-user-images.githubusercontent.com/16195016/360895859-a9b7103f-eb72-4e3a-a711-02c5237ab638.png?jwt=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3MjQ0MDk1ODIsIm5iZiI6MTcyNDQwOTI4MiwicGF0aCI6Ii8xNjE5NTAxNi8zNjA4OTU4NTktYTliNzEwM2YtZWI3Mi00ZTNhLWE3MTEtMDJjNTIzN2FiNjM4LnBuZz9YLUFtei1BbGdvcml0aG09QVdTNC1ITUFDLVNIQTI1NiZYLUFtei1DcmVkZW50aWFsPUFLSUFWQ09EWUxTQTUzUFFLNFpBJTJGMjAyNDA4MjMlMkZ1cy1lYXN0LTElMkZzMyUyRmF3czRfcmVxdWVzdCZYLUFtei1EYXRlPTIwMjQwODIzVDEwMzQ0MlomWC1BbXotRXhwaXJlcz0zMDAmWC1BbXotU2lnbmF0dXJlPWU0NDZiZGZjZjI5M2RmM2Y0ZTg0ZGQ2ZDAwNGU0ODc2MDhlMWQ3YzMxMzcxNTZiNTI3ZDgxNDRlY2U2OTEwYmUmWC1BbXotU2lnbmVkSGVhZGVycz1ob3N0JmFjdG9yX2lkPTAma2V5X2lkPTAmcmVwb19pZD0wIn0.roXn-Yol6uEW7Vi2gj2OdPo3MBtcJ8dMsi34JqGs3c4" width="512">

 
## Installation
Just do for standard compilation (tested with gcc 11.4.0):

````sh
    # generates the application
    g++ -o SquirrelOnTheRun SquirrelOnTheRun.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17 -Wno-narrowing
````

Run the application with:

````sh
    ./SquirrelOnTheRun
````

## Usage
Simply use the mouse to hover on the board.  
Use the left mouse button to place the tiles.  
Avoid the birds and collect the acorns for the next level.  
ESC : quit


## References, License, Credit
The program acknowledge the libraries and refer to the appropriate licenses
* [olcPixelGameEngine](https://github.com/OneLoneCoder/olcPixelGameEngine) by [Javidx9](https://github.com/OneLoneCoder)
* [Hex](https://www.redblobgames.com/grids/hexagons/codegen/output/lib.cpp) by [redblobgames](https://www.redblobgames.com)
* Other pieces of codes are released under [Creative Commons Zero v1.0 Universal](https://github.com/Bondoki/SquirrelOnTheRun/blob/main/LICENSE) for free usage.
* Assets are all released under public domain [Creative Commons Zero v1.0 Universal](https://github.com/Bondoki/SquirrelOnTheRun/blob/main/LICENSE) for free usage, check the assets/README.md for details.  

## Acknowledgment
Inspired by the [Coding Quickie: Isometric Tiles](https://www.youtube.com/watch?v=ukkbNKTgf5U) of [Javidx9](https://github.com/OneLoneCoder) gave the idea for this.  
The code for the implementing hex tiles is heavly based on the the very good [hexagonal grid tutorial](https://www.redblobgames.com/grids/hexagons) by [redblobgames](https://www.redblobgames.com).  
This game was done in August 2024 for the [OLC Code Jam](https://itch.io/jam/olc-codejam-2024).  

