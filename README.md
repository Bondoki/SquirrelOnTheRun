# Squirrel On The Run 
Help the cute little squirrel named "Puschel" to collect all acorns  
to survive the winter with his friend the rainbow lizard "Echsi".  
It's just for fun and educational purpose. Feel free to modify, improve, and use it - also the assets :)
 
<img src="https://github.com/user-attachments/assets/4543d3f1-f393-40e0-ae5a-9956c9f4d21d" width="512">

 
## Installation

### Local Installation
Just do for standard compilation (tested with gcc 11.4.0):

````sh
    # generates the application
    g++ -o SquirrelOnTheRun SquirrelOnTheRun.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17 -Wno-narrowing
````

Run the application with:

````sh
    ./SquirrelOnTheRun
````

### Web Browser
Download the [PGEtinker_SquirrelOnTheRun.html](https://github.com/Bondoki/SquirrelOnTheRun/blob/main/PGEtinker_SquirrelOnTheRun.html) and open the file with your web broser.  

e.g.  
````sh
    firefox PGEtinker_SquirrelOnTheRun.html
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
* Big Thanks to the [PGEtinker community](https://pgetinker.com/) giving me the possibility to share the application without installation :)


## Acknowledgment
Inspired by the [Coding Quickie: Isometric Tiles](https://www.youtube.com/watch?v=ukkbNKTgf5U) of [Javidx9](https://github.com/OneLoneCoder) gave the idea for this.  
The code for the implementing hex tiles is heavly based on the the very good [hexagonal grid tutorial](https://www.redblobgames.com/grids/hexagons) by [redblobgames](https://www.redblobgames.com).  
This game was done in August 2024 for the [OLC Code Jam](https://itch.io/jam/olc-codejam-2024).  

