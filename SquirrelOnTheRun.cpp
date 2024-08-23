/*
 Created on in the week 19-23. August 2024
 for the OLC Code JAM 2024: https://itch.io/jam/olc-codejam-2024 (Theme: RUN)
 Last modified on 23. August 2024
 Author: bondoki
 
 Purpose: Rendering of the hexagon tiles in various layout to emphasize pseudo 2.5 view.
 This program provides routine for selecting and creation of hex tiles
 heavly based on the tutorial http://www.redblobgames.com/grids/hexagons/base
 Inspired by the "Coding Quickie: Isometric Tiles" example of
 Javidx9 ( https://www.youtube.com/watch?v=ukkbNKTgf5U ) gave the idea for this.
 It's just for fun and educational purpose. Feel free to modify and use it :)
 
 Note1: the hex tiles are indexed in the axial coordinate (q,r,s) system with
 boundary condition: q+r+s=0 . Solely the third coordinate is given by s=-q-r 
 and the selected tile has the coordinate (q,r).
 
    ___
   / q \
  /     \
 |       |
  \s   r/
   \___/ 
 
 Note2: The tiles have been original equally size hexagon and scaled in the next
 by 50% in height direction.
 
 Note3: For a more isometric 2.5 view you should first rotate your hexagon by 45�
 and scale it by 50% in height direction.

 Note4: This is way ugly ugly code and drafted together for the
 OLC Code JAM 2024: https://itch.io/jam/olc-codejam-2024 (Theme: RUN) 
 
 License: This piece of code is licensed to Creative Commons Zero v1.0 Universal according to
 https://github.com/Bondoki/SquirrelOnTheRun/blob/main/LICENSE
 also attributing Javidx9 for the unique and easy-to-use olc::PixelGameEngine and the underlying OLC-3 license see
 https://github.com/OneLoneCoder/olcPixelGameEngine/wiki/Licencing
 For more information about Javidx9 and OneLoneCoder please see https://github.com/OneLoneCoder
 The assets used in this project are released under public domain; see the ReadMe in assets-folder
*/

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <unordered_set>
#include <set>
#include <list>
#include <unordered_map>
#include <deque> // random pile of tiles
#include <algorithm> // std::shuffle
#include <random> // std::default_random_engine
#include <iterator>
#include <queue> //std::priority_queue
#include <functional> //std::greater
#include <chrono>


#include "Hex.h"

/*
 Sample code from https://www.redblobgames.com/pathfinding/a-star/
 Copyright 2014 Red Blob Games <redblobgames@gmail.com>
 
 Feel free to use this code in your own projects, including commercial projects
 License: Apache v2.0 <http://www.apache.org/licenses/LICENSE-2.0.html>
*/
template<typename T, typename priority_t>
struct PriorityQueue {
  typedef std::pair<priority_t, T> PQElement;
  std::priority_queue<PQElement, std::vector<PQElement>, std::greater<PQElement> > elements;

  inline bool empty() const {
     return elements.empty();
  }

  inline void put(T item, priority_t priority) {
    elements.emplace(priority, item);
  }

  T get() {
    T best_item = elements.top().second;
    elements.pop();
    return best_item;
  }
};
/*
 END Disclaimer
*/

class SquirrelOnTheRun : public olc::PixelGameEngine
{
public:
  SquirrelOnTheRun()
  {
    sAppName = "SquirrelOnTheRun";
  }
  
private:
  // Total passed time
  float fTotalElapsedTime;
  
  // Number of tiles in world
  olc::vi2d vWorldSize = { 2, 2 };
  
  // Size of single tile graphic
  olc::vi2d vTileSize = { 72, 46 };
  
  // Size of single tile graphic and offset to align them
  olc::vi2d vTileSizeOffset = { 0, 10 };
  
  // Where to place tile (0,0) on screen (in screen pixel steps)
  olc::vi2d vOriginScreen = { 72*3, 46*3 };
  
  // Sprite that holds all imagery
  olc::Renderable  sprIsom;
  
  // List to order for drawing
  std::list<Hex> hexlist;
  
  // HexTile to Value
  using HexMap=std::unordered_map<Hex, int,decltype(hexhash), decltype(hexequal)>;
  HexMap heights{HexMap(10,hexhash,hexequal)};
  
  // Star-Sprite and shadow
  //olc::Renderable  sprStar;

  // Squirrel-Sprite and shadow
  olc::Renderable sprSquirrel;

  // Acorn-Sprite
  olc::Renderable sprAcorn;

  // Shadow-Sprite
  olc::Renderable sprShadow;

  // EnemyBird-Sprite
  olc::Renderable sprBird;

  // Drey-Sprite
  olc::Renderable sprDrey;
  olc::Renderable sprDreyAnim;

  // ExitCross-Sprite
  olc::Renderable sprExitCross;

  // Title screen
  olc::Renderable sprScreenTitle;

  // Help screen
  olc::Renderable sprScreenHelp;

  // Credits screen
  olc::Renderable sprScreenCredits;

  // Tag Saved
  olc::Renderable sprTagSaved;

  // Tag Loosing
  olc::Renderable sprTagLoosing;

  // Tag Saved
  olc::Renderable sprTagRun;



  // Position and Value of Acorn
  HexMap heightsAcorn{HexMap(10,hexhash,hexequal)};
  
  // Board-style name
  std::string sBoardName;
  
  // Animated elements sprite
  //olc::Renderable sprWater;
  //olc::Renderable sprAir;
  olc::Renderable sprFire;
  //olc::Renderable sprEarth;

  enum class PILETILETYPE
    {
        SELECTION = 0,
        SAND = 1,
        MEADOW = 2,
        WOOD = 3,
        MOUNTAIN = 4,
        WARP_IN = 5,
        EXIT_DREY = 6,
        WARP_IN_BIRD = 7,
        BLOCK = 8,
        GRID = 9,
        // etc
    };

  enum class STATE
  {
    STATE_TITLE,
    STATE_CREDIT, 
    STATE_HELP,
    STATE_LEVEL_RESET,
    STATE_LEVEL_INCEASE,
    STATE_GAME_INIT,
    STATE_GAME_PILE_DRAW_TILES,
    STATE_GAME_PILE_SELECTION,
    STATE_GAME_PLAYFIELD_SELECTION,
    STATE_GAME_PLAYFIELD_ADDING,
    STATE_GAME_BIRD_MOVE_RANDOM,
    STATE_GAME_BIRD_MOVE_FOLLOWING,
    STATE_GAME_BIRD_MOVE_HIDING,
    STATE_GAME_SQUIRREL_CHECK_PATH,
    STATE_GAME_SQUIRREL_CHECK_CATCHED_BY_BIRD,
    STATE_GAME_SQUIRREL_CHECK_OPEN_DREY,
    STATE_GAME_LOOSE_NOPATH,
    STATE_GAME_LOOSE_NOMOVE,
    STATE_GAME_LOOSE_NOTILES,
    STATE_GAME_LOOSE_CATCHED_BY_BIRD,
    STATE_GAME_WIN,
  } nState, nNextState;

  std::deque<PILETILETYPE> deqPileOfTiles; 

  // Position of Squirrel
  Hex posSquirrel;

  // Position of Exit
  Hex posDrey;

  // Position of Exit
  std::vector<Hex> posBird;


  // Selection of randomly drawn tiles
  olc::vi2d vSelectedPileTile;
  int vSelectedPileTileType = 0;
  olc::vi2d vSelectedPlayfieldTile;
  int vSelectedPlayfieldType = 0;

  // Position of hovering tile in Playfield
  Hex posHoveringTile;

  // Hex Nullposition
  Hex posNull = Hex(100,100,-200);

  // number of collected acorn
  int numAcornCollected = 0;

  // level
  uint level = 1;
  bool isDreyOpen = false;
  bool isSquirrelAtDrey = false;

  bool isSquirrelCatchedByBird = false;
  bool isSquirrelNotMovable = false;

  std::minstd_rand0 generator;  // minstd_rand0 is a standard linear_congruential_engine

  
public:
  bool OnUserCreate() override
  {
    fTotalElapsedTime=0.0f;
    /* initialize random seed: */
    srand (time(NULL));
    
    // Load sprites used in demonstration
    sprIsom.Load("assets/hextiles5.png"); // ok
    
    // sprStar.Load("assets/star.png");

    sprSquirrel.Load("assets/HeroSquirrelTiles.png"); // ok

    sprAcorn.Load("assets/Acorn.png"); // ok

    sprShadow.Load("assets/Shadow.png"); // ok

    sprBird.Load("assets/EnemyBird.png"); // ok

    sprDrey.Load("assets/Drey_72x46.png"); // ok

    sprDreyAnim.Load("assets/DreyEnd.png"); // ok

    sprExitCross.Load("assets/ExitCross.png"); // ok

    sprScreenTitle.Load("assets/ScreenTitle.png"); // ok

    sprScreenHelp.Load("assets/ScreenHelp.png"); // ok

    sprScreenCredits.Load("assets/ScreenCredits.png"); // ok

    sprTagSaved.Load("assets/TagSquirrelSaved.png"); // ok

    sprTagLoosing.Load("assets/TagLoosing.png"); // ok

    sprTagRun.Load("assets/TagSquirrelRun.png");// ok

    
    //sprWater.Load("assets/hex_water_anim.png");
    //sprAir.Load("assets/hex_air_anim.png");
    sprFire.Load("assets/hex_fire_anim.png"); // ok
    //sprEarth.Load("assets/hex_earth_anim.png");
    
    // we start with hexagonal board
    sBoardName="";

    // for the birds
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    generator.seed(seed);  // minstd_rand0 is a standard linear_congruential_engine

    
    nState = STATE::STATE_TITLE;// STATE::STATE_LEVEL_RESET;

    return true;
  }
  
  // create the board layout and sort for drawing
  void createBoard()
  {
    //clear the list and the map
    hexlist.clear();
    heights.clear();
    
    int dist = 4;
    {
       //Hexagon - Playfield
        sBoardName="Playfield";
        dist = 5;
        for (int q = -dist; q <= dist; q++)
        {
          int r1 = std::max(-dist, -q - dist);
          int r2 = std::min(dist, -q + dist);
          for (int r = r1; r <= r2; r++)
          {
            if((hex_distance(Hex(q, r, -q-r),Hex(0,0,0))) == 5 ) // rock boundary
            { 
              hexlist.push_back(Hex(q, r, -q-r));
              heights[Hex(q, r, -q-r)] = (int) PILETILETYPE::MOUNTAIN;//(hex_distance(Hex(q, r, -q-r),Hex(0,0,0))+4)%9+1;
            }
          }
        }

        hexlist.push_back(Hex(0,0,0));
        heights[Hex(0,0,0)] = (int) PILETILETYPE::WARP_IN_BIRD;

        // for (int r = -2; r <= 4; r++)
        // {
        //   hexlist.push_back(Hex(-2,r,2-r));
        //   heights[Hex(-2,r,2-r)] = (int) PILETILETYPE::WOOD;
        // }

        // remove boundary tile and provide entry and exit point
        {
          heights.erase(Hex(-5,0,5));
          hexlist.remove(Hex(-5,0,5));

          heights.erase(Hex(5,0,-5));
          hexlist.remove(Hex(5,0,-5));

          hexlist.push_back(Hex(-5,0,5));
          heights[Hex(-5,0,5)] = (int) PILETILETYPE::WARP_IN; //entry of squirrel

          hexlist.push_back(Hex(5,0,-5));
          heights[Hex(5,0,-5)] = (int) PILETILETYPE::EXIT_DREY; //exit

          // and sort again
          hexlist.sort(hexcomp_q);
          hexlist.sort(hexcomp_r);
        }

    }
    
    // center tile 0,0 to screen center
    vOriginScreen.x= ScreenWidth()/2-vTileSize.x/2;
    vOriginScreen.y= ScreenHeight()/2-vTileSize.y/2;
    
    // sort the list to draw from top to bottom
    hexlist.sort(hexcomp_q);
    hexlist.sort(hexcomp_r);
    
  }
  
  bool OnUserUpdate(float fElapsedTime) override
  {
    
    fTotalElapsedTime += fElapsedTime;
    Clear(olc::Pixel(155,180,0));
    
    if (GetKey(olc::Key::R).bPressed)
      nNextState = STATE::STATE_LEVEL_RESET;
    
    // Get Mouse in world
    olc::vi2d vMouse = { GetMouseX(), GetMouseY() };
    
    // this piece of converts screen pixels to axial tile coordinates
    double qfrac = 4.0 / 3.0 * ((1.0*vMouse.x-vOriginScreen.x - 0.5*(vTileSize.x-vTileSizeOffset.x)) / (vTileSize.x-vTileSizeOffset.x));
    double rfrac = -2.0 / 3.0 * ((1.0*vMouse.x-vOriginScreen.x - 0.5*(vTileSize.x-vTileSizeOffset.x)) / (vTileSize.x-vTileSizeOffset.x)) + 2.0*1.7320508075688/3.0 * ((1.0*vMouse.y-vOriginScreen.y-0.5*(vTileSize.y-vTileSizeOffset.y)) / (vTileSize.y-vTileSizeOffset.y));
    double sfrac = -qfrac-rfrac;
    int qi = int(std::round(qfrac));
    int ri = int(std::round(rfrac));
    int si = int(std::round(sfrac));
    double q_diff = std::fabs(qi - qfrac);
    double r_diff = std::fabs(ri - rfrac);
    double s_diff = std::fabs(si - sfrac);
    
    if (q_diff > r_diff && q_diff > s_diff)
    {
      qi = -ri - si;
    }
    else
      if (r_diff > s_diff)
      {
        ri = -qi - si;
      }
      else
      {
        si = -qi - ri;
      }
    
    olc::vi2d vSelected = 
      {         
        qi, ri
      };

    // Labmda function to convert "world" (offset-array) coordinate into screen space
    auto ToScreen = [&](int x, int y)
    {
      return olc::vi2d{
          0.5 * (vTileSize.x - vTileSizeOffset.x) * 3.0 / 2.0 * (x) + vOriginScreen.x,
          0.5 * ((vTileSize.y - vTileSizeOffset.y) * 1.7320508075688 * ((y) + 0.5 * ((x) & 1))) + vOriginScreen.y};
    };

    // Labmda function to convert "world" (axial) coordinate into screen space
    auto AxialToScreen = [&](int q, int r)
    {
      return olc::vi2d{
          0.5 * (vTileSize.x - vTileSizeOffset.x) * 3.0 / 2.0 * (q) + vOriginScreen.x,
          0.5 * ((vTileSize.y - vTileSizeOffset.y) * (1.7320508075688 / 2.0 * (q) + 1.7320508075688 * r)) + vOriginScreen.y};
    };

    // Gameplay
    switch (nState)
    {
    
    case STATE::STATE_TITLE:

      nNextState = STATE::STATE_TITLE;

      if (GetMouse(0).bPressed)
      {
        // check mouse position to change to different screens
        // Get Mouse in world
        olc::vi2d vMouse = { GetMouseX(), GetMouseY() };

        // Game screen
        if(GetMouseX() >= 440 && GetMouseX() <= 775 && GetMouseY() >= 150 && GetMouseY() <= 245)
        {
          nNextState = STATE::STATE_LEVEL_RESET;
        }

        // Help screen
        if(GetMouseX() >= 440 && GetMouseX() <= 775 && GetMouseY() >= 290 && GetMouseY() <= 385)
        {
          nNextState = STATE::STATE_HELP;
        }

        // Credit screen
        if(GetMouseX() >= 440 && GetMouseX() <= 775 && GetMouseY() >= 430 && GetMouseY() <= 520)
        {
          nNextState = STATE::STATE_CREDIT;
        }

        // EXIT Cross
        if(GetMouseX() >= 735 && GetMouseX() <= 775 && GetMouseY() >= 20 && GetMouseY() <= 60)
        {
          nNextState = STATE::STATE_TITLE;
          return false; // end application
        }

      }

      if(GetKey(olc::Key::ESCAPE).bPressed)
      {
        return false; // exit application
      }
    break;

    case STATE::STATE_HELP:

      nNextState = STATE::STATE_HELP;

      if (GetMouse(0).bPressed)
      {
        // check mouse position to change to different screens
        // Get Mouse in world
        olc::vi2d vMouse = { GetMouseX(), GetMouseY() };

        // EXIT Cross
        if(GetMouseX() >= 735 && GetMouseX() <= 775 && GetMouseY() >= 20 && GetMouseY() <= 60)
        {
          nNextState = STATE::STATE_TITLE;
        }
      }

      if(GetKey(olc::Key::ESCAPE).bPressed)
      {
        nNextState = STATE::STATE_TITLE;
      }
    break;

    case STATE::STATE_CREDIT:

      nNextState = STATE::STATE_CREDIT;

      if (GetMouse(0).bPressed)
      {
        // check mouse position to change to different screens
        // Get Mouse in world
        olc::vi2d vMouse = { GetMouseX(), GetMouseY() };

        // EXIT Cross
        if(GetMouseX() >= 735 && GetMouseX() <= 775 && GetMouseY() >= 20 && GetMouseY() <= 60)
        {
          nNextState = STATE::STATE_TITLE;
        }
      }

      if(GetKey(olc::Key::ESCAPE).bPressed)
      {
        nNextState = STATE::STATE_TITLE;
      }

    break;

    case STATE::STATE_LEVEL_RESET:
      level = 1;
      numAcornCollected = 0;
      isDreyOpen = false;
      isSquirrelAtDrey = false;
      isSquirrelCatchedByBird = false;
      isSquirrelNotMovable = false;
      nNextState = STATE::STATE_GAME_INIT;
    break;

    case STATE::STATE_LEVEL_INCEASE:
      level++;
      numAcornCollected = 0;
      isDreyOpen = false;
      isSquirrelAtDrey = false;
      isSquirrelCatchedByBird = false;
      isSquirrelNotMovable = false;
      nNextState = STATE::STATE_GAME_INIT;
    break;

    case STATE::STATE_GAME_INIT:

      // increase level counter

      // create the pile of tiles - in total 3*12
      // and shuffle
      deqPileOfTiles.clear();
      for (int i = 0; i < 12; i++) // there can only be 61 tiles placed on the field
      {
        deqPileOfTiles.push_back(PILETILETYPE::SAND);
      }
      for (int i = 0; i < 16; i++) // there can only be 61 tiles placed on the field
      {
        deqPileOfTiles.push_back(PILETILETYPE::MEADOW);
      }
      for (int i = 0; i < 20; i++) // there can only be 61 tiles placed on the field
      {
        deqPileOfTiles.push_back(PILETILETYPE::WOOD);
      }
      std::shuffle(deqPileOfTiles.begin(), deqPileOfTiles.end(), std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count()));

      /*
      std::cout << "shuffled elements:";
      for (PILETILETYPE &x : deqPileOfTiles)
        std::cout << ' ' << (int)x;
      std::cout << '\n';
      */

      createBoard();

      //createDrawTilesFromPile();

      posSquirrel.setAxial(-5, 0, 5);
      posDrey.setAxial(5, 0, -5);

      posBird.clear();
      for(int i = 0; i < level; i++)
        posBird.push_back(Hex(0,0,0));
        //posBird.setAxial(0,0,0);

      posHoveringTile = posNull;

      vSelectedPileTileType = 0;

      // set acorn
      heightsAcorn.clear();

      heightsAcorn[Hex(-4,1,3)] = 1;
      heightsAcorn[Hex(-4,3,1)] = 1;
      heightsAcorn[Hex(4,-1,-3)] = 1;
      heightsAcorn[Hex(4,-3,-1)] = 1;

      heightsAcorn[Hex(-2,4,-2)] = 2;
      heightsAcorn[Hex(-2,-2,4)] = 2;
      heightsAcorn[Hex(2,-4,2)] = 2;
      heightsAcorn[Hex(2,2,-4)] = 2;

      heightsAcorn[Hex(0,4,-4)] = 3;
      heightsAcorn[Hex(0,-4,4)] = 3;
      // end-set acorn

      // reset acorn
      numAcornCollected = 0;
      
      nNextState = STATE::STATE_GAME_PILE_DRAW_TILES;//STATE::STATE_GAME_PILE_SELECTION;

      break;

    case STATE::STATE_GAME_PILE_DRAW_TILES:

      if (deqPileOfTiles.size() == 0)
      {
        // no tiles left
        isSquirrelNotMovable = true;
        nNextState = STATE::STATE_GAME_LOOSE_NOTILES;
        break;
      }
      // check if Pile has some tiles left
      if (deqPileOfTiles.size() > 0)
      {
        hexlist.push_back(Hex(-2, -6, 8));
        heights[Hex(-2, -6, 8)] = (int)deqPileOfTiles.at(0);
      }

      if (deqPileOfTiles.size() > 1)
      {
        hexlist.push_back(Hex(0, -7, 7));
        heights[Hex(0, -7, 7)] = (int)deqPileOfTiles.at(1);
      }

      if (deqPileOfTiles.size() > 2)
      {
        hexlist.push_back(Hex(2, -8, 6));
        heights[Hex(2, -8, 6)] = (int)deqPileOfTiles.at(2);
      }

      /*std::cout << "shuffled elements:";
      for (PILETILETYPE &x : deqPileOfTiles)
        std::cout << ' ' << (int)x;
      std::cout << '\n';
      */
      
      nNextState = STATE::STATE_GAME_PILE_SELECTION;

      break;

    case STATE::STATE_GAME_PILE_SELECTION:

      if(GetKey(olc::Key::ESCAPE).bPressed)
      {
        nNextState = STATE::STATE_TITLE;
        break;
      }

      // Handle mouse left click to switch tile
      if (GetMouse(0).bPressed)
      {
        // Exit-Cross -Sprite clicked?
        {
          // check mouse position to change to different screens
          // Get Mouse in world
          olc::vi2d vMouse = {GetMouseX(), GetMouseY()};

          // EXIT Cross
          if (GetMouseX() >= 735 && GetMouseX() <= 775 && GetMouseY() >= 20 && GetMouseY() <= 60)
          {
            nNextState = STATE::STATE_TITLE;
            break;
          }
        }

        // Guard array boundary
        auto got = heights.find(Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y));

        // does it exist?
        if (got != heights.end())
        {
          // check if tile is most left selection
          if (Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y) == Hex(-2, -6, 8))
          {
            int value = heights.at(Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y));

            vSelectedPileTileType = value;
            vSelectedPileTile = olc::vi2d(vSelected.x, vSelected.y);

            //std::cout << (int)vSelectedPileTileType << std::endl;

            //nNextState = STATE::STATE_GAME_PLAYFIELD_SELECTION;
          }

          // check if tile is middle selection
          if (Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y) == Hex(0, -7, 7))
          {
            int value = heights.at(Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y));

            vSelectedPileTileType = value;
            vSelectedPileTile = olc::vi2d(vSelected.x, vSelected.y);

            //std::cout << (int)vSelectedPileTileType << std::endl;
            //nNextState = STATE::STATE_GAME_PLAYFIELD_SELECTION;
          }

          // check if tile is right selection
          if (Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y) == Hex(2, -8, 6))
          {
            int value = heights.at(Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y));

            vSelectedPileTileType = value;
            vSelectedPileTile = olc::vi2d(vSelected.x, vSelected.y);

            //std::cout << (int)vSelectedPileTileType << std::endl;
            //nNextState = STATE::STATE_GAME_PLAYFIELD_SELECTION;
          }
        }
      }

      // break; no break as player may reselect choice

    case STATE::STATE_GAME_PLAYFIELD_SELECTION:

      if (vSelectedPileTileType != 0) // hexequal(Hex(vSelected.x,vSelected.y,-vSelected.x-vSelected.y), Hex(Hex(q,r,s))) )
      {
        // in playfield: max distance < 5
        if (hex_distance(Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y), Hex(0, 0, 0)) < 5)
        {
          olc::vi2d vWorld = AxialToScreen(vSelected.x, vSelected.y);
          int moveXSelected = 0.0;
          int moveYSelected = 5 * std::sin(fTotalElapsedTime * 5);

          // place tile only in vicinity of squirrel
          if (hex_distance(posSquirrel, Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y)) == 1)
          {
            // check if position is empty or already added
            if (heights.find(Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y)) == heights.end())
            {
              if (!hexequal(Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y), posHoveringTile))
              {
                hexlist.push_back(Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y));
                heights[Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y)] = (int)vSelectedPileTileType;

                heights.erase(posHoveringTile);
                hexlist.remove(posHoveringTile);

                posHoveringTile = Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y);

                // and sort again
                hexlist.sort(hexcomp_q);
                hexlist.sort(hexcomp_r);
              }
            }
          }
          else
          {
            if (heights.find(posHoveringTile) != heights.end())
            {
              heights.erase(posHoveringTile);
              hexlist.remove(posHoveringTile);

              posHoveringTile = posNull;

              // and sort again
              hexlist.sort(hexcomp_q);
              hexlist.sort(hexcomp_r);
            }
          }
        }
        else
        {
          // check if hovering tile is in default position and delete if necessary
          if (!hexequal(posNull, posHoveringTile))
          {
            // hexlist.push_back(Hex(vSelected.x,vSelected.y,-vSelected.x-vSelected.y));
            // heights[Hex(vSelected.x,vSelected.y,-vSelected.x-vSelected.y)] = (int) vSelectedPileTileType;

            heights.erase(posHoveringTile);
            hexlist.remove(posHoveringTile);

            posHoveringTile = posNull;

            // and sort again
            hexlist.sort(hexcomp_q);
            hexlist.sort(hexcomp_r);
          }
        }
      }

      // Handle mouse left click to add tile if possible
      if (GetMouse(0).bPressed)
      {
        // Guard array boundary
        auto got = heights.find(Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y));

        // does it exist? As also rendered possible location aare also in the list, so add the tile to the playfield
        if (got != heights.end())
        {
          // something can be added on the playfield
          if (hexequal(Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y), posHoveringTile))
          {
            posSquirrel = posHoveringTile;
            posHoveringTile = posNull;

            // reset the Pile and delete drawn Tiles
            heights.erase(Hex(-2, -6, 8));
            hexlist.remove(Hex(-2, -6, 8));

            heights.erase(Hex(0, -7, 7));
            hexlist.remove(Hex(0, -7, 7));

            heights.erase(Hex(2, -8, 6));
            hexlist.remove(Hex(2, -8, 6));

            // remove left tile
            if(vSelectedPileTile == olc::vi2d(-2,-6))
            {
              deqPileOfTiles.erase (deqPileOfTiles.begin()+0);
            }

            // remove middle
            if(vSelectedPileTile == olc::vi2d(0,-7))
            {
              deqPileOfTiles.erase (deqPileOfTiles.begin()+1);
            }

            //remove right
            if(vSelectedPileTile == olc::vi2d(2,-8))
            {
              deqPileOfTiles.erase (deqPileOfTiles.begin()+2);
            }

            // vSelectedPileTileType = 0;

            // check if Hex-Field has an acorn
            {
              auto gotAcorn = heightsAcorn.find(posSquirrel);
              if (gotAcorn != heightsAcorn.end())
              {
                  // increase counter
                  numAcornCollected += gotAcorn->second;

                  // delete tile
                  heightsAcorn.erase(gotAcorn->first);
              }

            }


            
            if (hexequal(posSquirrel, Hex(4, 0, -4)) && isDreyOpen == true)
            {
              // Squirrel reached the drey - or more precisely is next to the drey
              //std::cout << " Squirrel at the drey! Well done! The rainbow Lizard is happy :)" << std::endl;
              isSquirrelAtDrey = true;
              vSelectedPileTileType = 0;
              nNextState = STATE::STATE_GAME_WIN;
              break;
            }

            // check if at least one tile next to the squirrel is empty
            // otherwise lose the game
            {
              int dist = 1;
              bool hasEmptyVicinity = false;
              for (int q = -dist; q <= dist; q++)
              {
                int r1 = std::max(-dist, -q - dist);
                int r2 = std::min(dist, -q + dist);
                for (int r = r1; r <= r2; r++)
                {
                  if ((hex_distance(hex_add(Hex(q, r, -q - r), posSquirrel), posSquirrel)) == 1) // rock boundary
                  {
                    // vicinity tile is not in the list - at least one tile can be added to escape the squirrel
                    if (heights.find(hex_add(Hex(q, r, -q - r), posSquirrel)) == heights.end())
                      hasEmptyVicinity = true;
                  }
                }
              }

              if (hasEmptyVicinity == false)
              {
                // Exit with "loosing" state
                //std::cout << "Ohhh, noooo! The Squirrel can not move anymore! Try again :)" << std::endl;
                isSquirrelNotMovable = true;
                nNextState = STATE::STATE_GAME_LOOSE_NOMOVE;
                break;
              }
            }

            //nNextState = STATE::STATE_GAME_PILE_DRAW_TILES;
            if(vSelectedPileTileType == (int) PILETILETYPE::SAND)
            {
              //FOLLOW 
              //std::cout << "follow" << std::endl;
              vSelectedPileTileType = 0;
              nNextState = STATE::STATE_GAME_BIRD_MOVE_FOLLOWING;
            }

            if(vSelectedPileTileType == (int) PILETILETYPE::MEADOW)
            {
              //HIDE
              //std::cout << "hide" << std::endl;
              vSelectedPileTileType = 0;
              nNextState = STATE::STATE_GAME_BIRD_MOVE_RANDOM;
            }

            if(vSelectedPileTileType == (int) PILETILETYPE::WOOD)
            {
              //BLOCK
              //std::cout << "block" << std::endl;
              vSelectedPileTileType = 0;
              nNextState = STATE::STATE_GAME_BIRD_MOVE_RANDOM;
            }


          }
        }
      }

      // loop will reenter in STATE::STATE_GAME_PILE_SELECTION if nothing happens

    break;

    case STATE::STATE_GAME_PLAYFIELD_ADDING:
      //
      break;

    case STATE::STATE_GAME_BIRD_MOVE_RANDOM:
      
      for(int i = 0; i < posBird.size(); i++)
      {
      // get a (q,r) pair with -1 <= q,r <= 1
      //unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

      //std::minstd_rand0 generator (seed);  // minstd_rand0 is a standard linear_congruential_engine

      // create new position within boundary <= 5

      Hex tmpBird = posBird.at(i);// posBird;

      // this is not working as Hex(1,1,-2) is no neighbour
      // do
      // {
      //   // generator() -> between 0, numeric_limits<UIntType>::max()
      //   int q = generator()%3-1;// gives value between -1,0,1
      //   int r = generator()%3-1;// gives value between -1,0,1
      //   std::cout << q << " , " << r << std::endl;
      //   tmpBird = hex_add(posBird, Hex(q,r,-q-r));
      // } while (!(hex_distance(tmpBird, Hex(0, 0, 0)) < 5));

      // Six neigbors + stay on position
      std::vector<Hex> neighborDirectionHex = {Hex(+1, 0, -1), Hex(+1, -1, 0), Hex(0, -1, +1), 
      Hex(-1, 0, +1), Hex(-1, +1, 0), Hex(0, +1, -1), Hex(0, 0, 0)};

      do
      {
        // generator() -> between 0, numeric_limits<UIntType>::max()
        tmpBird = hex_add(posBird.at(i), neighborDirectionHex.at(generator()%7));

      } while ((hex_distance(tmpBird, Hex(0, 0, 0)) >= 5) || (heights.find(tmpBird) != heights.end() ? heights.find(tmpBird)->second == (int) PILETILETYPE::WOOD : false));
    
      
      //std::cout << "Bird old: ( " << posBird.get_q() << " , " << posBird.get_r() << " ) --> ";

      posBird[i] = tmpBird;

      //std::cout << "Bird new: ( " << posBird.get_q() << " , " << posBird.get_r() << " ) " << std::endl;

      }

      nNextState = STATE::STATE_GAME_SQUIRREL_CHECK_PATH;//STATE::STATE_GAME_PILE_DRAW_TILES;
      break;

    case STATE::STATE_GAME_BIRD_MOVE_HIDING:
    nNextState = STATE::STATE_GAME_SQUIRREL_CHECK_PATH;//STATE::STATE_GAME_PILE_DRAW_TILES;
    break;

    case STATE::STATE_GAME_BIRD_MOVE_FOLLOWING:

    for(int i = 0; i < posBird.size(); i++)
    {
      auto AStarSearch = [&](Hex TileStart, Hex TileEnd)
      {
        using HexHexUnorderedMap=std::unordered_map<Hex, Hex,decltype(hexhash), decltype(hexequal)>;
        HexHexUnorderedMap came_from{HexHexUnorderedMap(10,hexhash,hexequal)};
        
        HexMap cost_so_far{HexMap(10,hexhash,hexequal)};
        
        came_from.clear();
        cost_so_far.clear();

        PriorityQueue<Hex, int> frontier;
        frontier.put(TileStart, 0);

        came_from[TileStart] = TileStart;
        cost_so_far[TileStart] = 0;

        while (!frontier.empty())
        {
          Hex current = frontier.get();
          if (current == TileEnd)
          {
            break;
          }

          // get a vector of tile for the 'current' tile 
          // also check if distance from center is < 5
          std::vector<Hex> neighbors;
          neighbors.clear();

          {
            int dist = 1;
            for (int q = -dist; q <= dist; q++)
            {
              int r1 = std::max(-dist, -q - dist);
              int r2 = std::min(dist, -q + dist);
              for (int r = r1; r <= r2; r++)
              {
                if ((hex_distance(hex_add(Hex(q, r, -q - r), current), current)) == 1) // vicinity of current
                {
                  if(hex_distance(hex_add(Hex(q, r, -q - r), current), Hex(0, 0, 0)) < 5) // within 5 tiles
                    neighbors.push_back(hex_add(Hex(q, r, -q - r), current));
                }
              }
            }

            // print neighbors
            // std::cout << " neighbors at " << "( " << current.get_q() << " , " << current.get_r() << " ); " << std::endl;
            // for (auto it = neighbors.begin(); it != neighbors.end(); ++it)
            // {
            //   std::cout << "( " << it->get_q() << " , " << it->get_r() << " ); ";
            // }
            // std::cout << std::endl;
          }

          for (Hex next : neighbors)
          { 
            // calculate cost
            // if tile not present simply 1
            // if tile is SAND simply 1
            // if tile is MEADOW simply 1
            // if tile is WOOD high value 10

            int factor = 1;

            auto got = heights.find(next);
            // does it exist?
            if (got != heights.end())
            {
              // check if tile is most left selection
              if (heights.at(next) == (int)PILETILETYPE::WOOD)
              {
                factor = 10;
              }
            }

            int new_cost = cost_so_far[current] + factor;// by default each tile distance is one
            if (cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far[next])
            {
              cost_so_far[next] = new_cost;
              int priority = new_cost ;//+ hex_distance(next, TileEnd);
              frontier.put(next, priority);
              came_from[next] = current;
            }
          }
        }

        //return cost_so_far; // gives tile and cost (distance)
        // for full path Hex->Hex use: return came_from;
        return came_from;
      };

      auto reconstruct_path = [&](Hex TileStart, Hex TileEnd, std::unordered_map<Hex, Hex,decltype(hexhash), decltype(hexequal)> came_from) 
      {
        std::vector<Hex> path;
        Hex current = TileEnd;
        if (came_from.find(TileEnd) == came_from.end())
        {
          return path; // no path can be found
        }
        while(!hexequal(current, TileStart))// (current != TileStart)
        {
          path.push_back(current);
          current = came_from[current];
        }
        path.push_back(TileStart); // optional
        std::reverse(path.begin(), path.end());
        return path;
      };

      auto came_from = AStarSearch(posBird.at(i), posSquirrel);
      auto path = reconstruct_path(posBird.at(i), posSquirrel, came_from);
      // print path
      /* std::cout << "path" << std::endl;
      for (auto it = path.begin(); it != path.end(); ++it)
          {
              std::cout << "( " << it->get_q() << " , " << it->get_r() << " ),  ";
              // std::cout << "( " << it->first.get_q() << " , " << it->first.get_r() << " ) => "  << it->second << std::endl;
              //std::cout << "( " << it->first.get_q() << " , " << it->first.get_r() << " ) => "  << "( " << it->second.get_q() << " , " << it->second.get_r() << " ) " << std::endl;
          } */
      //std::cout << std::endl;

      // replace bird position with path

      //std::cout << "Bird old: ( " << posBird.get_q() << " , " << posBird.get_r() << " ) --> ";

      if(path.size()>=2) // sanity check
        posBird[i] = path.at(1);

     // std::cout << "Bird new: ( " << posBird.get_q() << " , " << posBird.get_r() << " ) " << std::endl;
    }
      nNextState = STATE::STATE_GAME_SQUIRREL_CHECK_PATH;
      break;

    case STATE::STATE_GAME_SQUIRREL_CHECK_PATH:
    {
      auto AStarSearch = [&](Hex TileStart, Hex TileEnd)
      {
        using HexHexUnorderedMap=std::unordered_map<Hex, Hex,decltype(hexhash), decltype(hexequal)>;
        HexHexUnorderedMap came_from{HexHexUnorderedMap(10,hexhash,hexequal)};
        
        HexMap cost_so_far{HexMap(10,hexhash,hexequal)};
        
        came_from.clear();
        cost_so_far.clear();

        PriorityQueue<Hex, int> frontier;
        frontier.put(TileStart, 0);

        came_from[TileStart] = TileStart;
        cost_so_far[TileStart] = 0;

        while (!frontier.empty())
        {
          Hex current = frontier.get();
          if (current == TileEnd)
          {
            break;
          }

          // get a vector of tile for the 'current' tile 
          // also check if distance from center is < 5
          std::vector<Hex> neighbors;
          neighbors.clear();

          {
            int dist = 1;
            for (int q = -dist; q <= dist; q++)
            {
              int r1 = std::max(-dist, -q - dist);
              int r2 = std::min(dist, -q + dist);
              for (int r = r1; r <= r2; r++)
              {
                if ((hex_distance(hex_add(Hex(q, r, -q - r), current), current)) == 1) // vicinity of current
                {
                  if(hex_distance(hex_add(Hex(q, r, -q - r), current), Hex(0, 0, 0)) < 5) // within 5 tiles
                    if(heights.find(hex_add(Hex(q, r, -q - r), current)) == heights.end())  // does it exist? We need open path for the squirrel
                      neighbors.push_back(hex_add(Hex(q, r, -q - r), current));
                }

                if ((hexequal(hex_add(Hex(q, r, -q - r), current), posDrey))) // add the drey
                {
                  neighbors.push_back(hex_add(Hex(q, r, -q - r), current));
                }
              }
            }

            // print neighbors
            // std::cout << " neighbors at " << "( " << current.get_q() << " , " << current.get_r() << " ); " <<  std::endl;
            // for (auto it = neighbors.begin(); it != neighbors.end(); ++it)
            // {
            //   std::cout << "( " << it->get_q() << " , " << it->get_r() << " ); ";
            // }
            // std::cout << std::endl;
          }

          for (Hex next : neighbors)
          { 
            // calculate cost
            // if tile not present simply 1
            // if tile is SAND simply 1
            // if tile is MEADOW simply 1
            // if tile is WOOD high value 10

            int factor = 1;

            //auto got = heights.find(next);

            // does it exist? We need open path for the squirrel
            //if (got == heights.end())
            {

              int new_cost = cost_so_far[current] + factor; // by default each tile distance is one
              if (cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far[next])
              {
                cost_so_far[next] = new_cost;
                int priority = new_cost; //+ hex_distance(next, TileEnd);
                frontier.put(next, priority);
                came_from[next] = current;
              }
            }
          }
        }

        //return cost_so_far; // gives tile and cost (distance)
        // for full path Hex->Hex use: return came_from;
        return came_from;
      };

      auto reconstruct_path = [&](Hex TileStart, Hex TileEnd, std::unordered_map<Hex, Hex,decltype(hexhash), decltype(hexequal)> came_from) 
      {
        std::vector<Hex> path;
        Hex current = TileEnd;
        if (came_from.find(TileEnd) == came_from.end())
        {
          return path; // no path can be found
        }
        while(!hexequal(current, TileStart))// (current != TileStart)
        {
          path.push_back(current);
          current = came_from[current];
        }
        path.push_back(TileStart); // optional
        std::reverse(path.begin(), path.end());
        return path;
      };

      auto came_from = AStarSearch(posSquirrel, posDrey);
      auto path = reconstruct_path(posSquirrel, posDrey, came_from);
      // print path
      // std::cout << "path squirrel" << std::endl;
      // for (auto it = path.begin(); it != path.end(); ++it)
      //     {
      //         std::cout << "( " << it->get_q() << " , " << it->get_r() << " ),  ";
      //         // std::cout << "( " << it->first.get_q() << " , " << it->first.get_r() << " ) => "  << it->second << std::endl;
      //         //std::cout << "( " << it->first.get_q() << " , " << it->first.get_r() << " ) => "  << "( " << it->second.get_q() << " , " << it->second.get_r() << " ) " << std::endl;
      //     }
      // std::cout << std::endl;

      
      if(path.size()==0) // sanity check
        {
          // no path :/
          isSquirrelNotMovable = true;
          vSelectedPileTileType = 0;
          nNextState = STATE::STATE_GAME_LOOSE_NOPATH; 
          break;
        }
      

    }

      nNextState = STATE::STATE_GAME_SQUIRREL_CHECK_CATCHED_BY_BIRD;//STATE::STATE_GAME_PILE_DRAW_TILES;
    break;

    case STATE::STATE_GAME_SQUIRREL_CHECK_CATCHED_BY_BIRD:

      nNextState = STATE::STATE_GAME_SQUIRREL_CHECK_OPEN_DREY;

      for (int i = 0; i < posBird.size(); i++)
      {
        if (hexequal(posSquirrel, posBird.at(i)) == true)
        {
          isSquirrelCatchedByBird = true;
          nNextState = STATE::STATE_GAME_LOOSE_CATCHED_BY_BIRD;
          break;
        }
      }

    break;

    case STATE::STATE_GAME_SQUIRREL_CHECK_OPEN_DREY:

      if((numAcornCollected >= (level+9)))
      {
        isDreyOpen = true;
        //DrawString(8, ScreenHeight()-20, "All acorns collected. RUN to the drey...", olc::BLACK);
      }

      nNextState = STATE::STATE_GAME_PILE_DRAW_TILES;
    break;


    case STATE::STATE_GAME_WIN:
      //std::cout << " Squirrel at the drey! Well done! The rainbow Lizard is happy :) Press Key R" << std::endl;
      DrawString(8, ScreenHeight()-30, "The Squirrel and the Rainbow Lizard are happy together :)", olc::BLACK);
      DrawString(8, ScreenHeight()-20, "Press left mouse button.", olc::BLACK);
      if (GetMouse(0).bPressed)
      {
        nNextState = STATE::STATE_LEVEL_INCEASE;
      }
    break;

    case STATE::STATE_GAME_LOOSE_NOPATH:
      //std::cout << "Ohhh, noooo! The Squirrel can not move anymore! Try again :) Press Key R" << std::endl;
      DrawString(8, ScreenHeight()-30, "Ohhh, noooo! No path to the drey! Try again :)", olc::BLACK);
      DrawString(8, ScreenHeight()-20, "Press Key R or left mouse click.", olc::BLACK);
      if (GetMouse(0).bPressed)
      {
        nNextState = STATE::STATE_LEVEL_RESET;
      }
    break;

    case STATE::STATE_GAME_LOOSE_NOMOVE:
      //std::cout << "Ohhh, noooo! The Squirrel can not move anymore! Try again :) Press Key R" << std::endl;
      DrawString(8, ScreenHeight()-30, "Ohhh, noooo! The Squirrel can not move anymore! Try again :)", olc::BLACK);
      DrawString(8, ScreenHeight()-20, "Press Key R or left mouse click.", olc::BLACK);
      if (GetMouse(0).bPressed)
      {
        nNextState = STATE::STATE_LEVEL_RESET;
      }
    break;

    case  STATE::STATE_GAME_LOOSE_NOTILES:
      // 
      DrawString(8, ScreenHeight()-30, "Ohhh, noooo! No Tiles left! Try again :)", olc::BLACK);
      DrawString(8, ScreenHeight()-20, "Press Key R or left mouse click.", olc::BLACK);
      DrawDecal({585, 485}, sprTagLoosing.Decal(), {1.0f, 1.0f}, olc::WHITE);
      if (GetMouse(0).bPressed)
      {
        nNextState = STATE::STATE_LEVEL_RESET;
      }
    break;

    case STATE::STATE_GAME_LOOSE_CATCHED_BY_BIRD:
      
      DrawString(8, ScreenHeight()-30, "Ohhh, noooo! The SQUIRREL was catched by the bird :/ ", olc::BLACK);
      DrawString(8, ScreenHeight()-20, "Press Key R or left mouse click.", olc::BLACK);
      if (GetMouse(0).bPressed)
      {
        nNextState = STATE::STATE_LEVEL_RESET;
      }
    break;

    default:
      break;
    }

    // move forward with state
    nState = nNextState;

    // GAME DRAWING
    if (nState != STATE::STATE_TITLE || nState != STATE::STATE_HELP || nState != STATE::STATE_CREDIT)
    {
      // draw grid
      {
        int dist = 5;
        for (int q = -dist; q <= dist; q++)
        {
          int r1 = std::max(-dist, -q - dist);
          int r2 = std::min(dist, -q + dist);
          for (int r = r1; r <= r2; r++)
          {
            if ((hex_distance(Hex(q, r, -q - r), Hex(0, 0, 0))) < 5) // rock boundary
            {
              olc::vi2d vWorld = AxialToScreen(q, r);
              olc::Pixel tinting = olc::WHITE; // hex_distance(Hex(q,r,-q-r), posBird) <= 1 ? (hex_distance(Hex(q,r,-q-r), posBird) == 0 ? olc::RED : olc::Pixel(255,100,100)) : olc::WHITE;

              for (int i = 0; i < posBird.size(); i++)
              {
                if (hex_distance(Hex(q, r, -q - r), posBird.at(i)) <= 1)
                {
                  if (hex_distance(Hex(q, r, -q - r), posBird.at(i)) == 0)
                    tinting = olc::RED;
                  else
                    tinting = olc::Pixel(255, 100, 100);
                }
              }

              DrawPartialDecal({vWorld.x, vWorld.y}, sprIsom.Decal(), {9 * vTileSize.x, 0 * vTileSize.y}, {vTileSize.x, vTileSize.y}, {(1.0F), (1.0F)}, tinting);
            }
          }
        }
      }

      // draw tiles
      // includes 3 randomly drawn tiles of the pile
      for (auto it = hexlist.cbegin(); it != hexlist.cend(); ++it)
      {
        int q = (*it).get_q();
        int r = (*it).get_r();
        int s = (*it).get_s();

        auto got = heights.find(Hex(q, r, s));

        if (got != heights.end())
        {
          olc::vi2d vWorld = AxialToScreen(q, r);

          int moveXSelected = 0;
          int moveYSelected = 0;

          // highlight selected hexagon
          if (hexequal(Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y), Hex(q, r, s)))
          {
            moveXSelected = 0.0;
            moveYSelected = 5 * std::sin(fTotalElapsedTime * 5);
          }

          // olc::Pixel tinting = hex_distance(Hex(q,r,s), posBird) <= 1 ? (hex_distance(Hex(q,r,s), posBird) == 0 ? olc::RED : olc::Pixel(255,190,190)) : olc::WHITE;

          olc::Pixel tinting = olc::WHITE; // hex_distance(Hex(q,r,-q-r), posBird) <= 1 ? (hex_distance(Hex(q,r,-q-r), posBird) == 0 ? olc::RED : olc::Pixel(255,100,100)) : olc::WHITE;

          for (int i = 0; i < posBird.size(); i++)
          {
            if (hex_distance(Hex(q, r, -q - r), posBird.at(i)) <= 1)
            {
              if (hex_distance(Hex(q, r, -q - r), posBird.at(i)) == 0)
                tinting = olc::RED;
              else
                tinting = olc::Pixel(255, 100, 100);
            }
          }

          switch (got->second)
          {
          case 0:
            // Selected Tile
            DrawPartialDecal({vWorld.x + moveXSelected, vWorld.y + moveYSelected}, sprIsom.Decal(), {0 * vTileSize.x, 0}, {vTileSize.x, vTileSize.y}, {(1.0F), (1.0F)}, tinting);
            break;
          case 1:
            // Visible Tile
            DrawPartialDecal({vWorld.x + moveXSelected, vWorld.y + moveYSelected}, sprIsom.Decal(), {1 * vTileSize.x, 0}, {vTileSize.x, vTileSize.y}, {(1.0F), (1.0F)}, tinting);
            break;
          case 2:
            DrawPartialDecal({vWorld.x + moveXSelected, vWorld.y + moveYSelected}, sprIsom.Decal(), {2 * vTileSize.x, 0}, {vTileSize.x, vTileSize.y}, {(1.0F), (1.0F)}, tinting);
            break;
          case 3:
            DrawPartialDecal({vWorld.x + moveXSelected, vWorld.y + moveYSelected}, sprIsom.Decal(), {3 * vTileSize.x, 0}, {vTileSize.x, vTileSize.y}, {(1.0F), (1.0F)}, tinting);
            break;
          case 4:
            DrawPartialDecal({vWorld.x + moveXSelected, vWorld.y + moveYSelected}, sprIsom.Decal(), {4 * vTileSize.x, 0}, {vTileSize.x, vTileSize.y}, {(1.0F), (1.0F)}, tinting);
            break;

          case 5:
            DrawPartialDecal({vWorld.x + moveXSelected, vWorld.y + moveYSelected}, sprIsom.Decal(), {5 * vTileSize.x, 0 * vTileSize.y}, {vTileSize.x, vTileSize.y});
            break;
          case 6:
            if (isDreyOpen == false)
            {
              DrawPartialDecal({vWorld.x + moveXSelected, vWorld.y + moveYSelected}, sprIsom.Decal(), {6 * vTileSize.x, 0 * vTileSize.y}, {vTileSize.x, vTileSize.y});
            }
            else
            {
              DrawPartialDecal({vWorld.x /*+moveXSelected*/, vWorld.y /*+moveYSelected*/}, sprIsom.Decal(), {10 * vTileSize.x, 0 * vTileSize.y}, {vTileSize.x, vTileSize.y});
              // DrawPartialDecal({vWorld.x+moveXSelected, vWorld.y+moveYSelected}, sprDrey.Decal(), {0, 0}, {32, 32});
            }
            /*if ( hexequal(Hex(vSelected.x,vSelected.y,-vSelected.x-vSelected.y), Hex(Hex(q,r,s))) )
            {
              DrawPartialDecal({vWorld.x+moveXSelected, vWorld.y+moveYSelected}, sprEarth.Decal(), {int(fmodf(fTotalElapsedTime*9,18.0f)) * vTileSize.x, 0 * vTileSize.y}, {vTileSize.x, vTileSize.y});
            }
            else
            {
              DrawPartialDecal({vWorld.x+moveXSelected, vWorld.y+moveYSelected}, sprIsom.Decal(), {6 * vTileSize.x, 0 * vTileSize.y}, {vTileSize.x, vTileSize.y});
            }*/
            break;
          case 7:
            if (hexequal(Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y), Hex(Hex(q, r, s))))
            {
              DrawPartialDecal({vWorld.x + moveXSelected, vWorld.y + moveYSelected}, sprFire.Decal(), {int(fmodf(fTotalElapsedTime * 10, 7.0f)) * vTileSize.x, 0 * vTileSize.y}, {vTileSize.x, vTileSize.y}, {(1.0F), (1.0F)}, tinting);
            }
            else
            {
              DrawPartialDecal({vWorld.x + moveXSelected, vWorld.y + moveYSelected}, sprIsom.Decal(), {7 * vTileSize.x, 0 * vTileSize.y}, {vTileSize.x, vTileSize.y}, {(1.0F), (1.0F)}, tinting);
            }
            break;
          case 8:
            /*if (hexequal(Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y), Hex(Hex(q, r, s))))
            {
              DrawPartialDecal({vWorld.x + moveXSelected, vWorld.y + moveYSelected}, sprWater.Decal(), {int(fmodf(fTotalElapsedTime * 9, 18.0f)) * vTileSize.x, 0 * vTileSize.y}, {vTileSize.x, vTileSize.y});
            }
            else*/
            {
              DrawPartialDecal({vWorld.x + moveXSelected, vWorld.y + moveYSelected}, sprIsom.Decal(), {8 * vTileSize.x, 0 * vTileSize.y}, {vTileSize.x, vTileSize.y});
            }
            break;

          case 9:
            /*if (hexequal(Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y), Hex(Hex(q, r, s))))
            {
              DrawPartialDecal({vWorld.x + moveXSelected, vWorld.y + moveYSelected}, sprAir.Decal(), {int(fmodf(fTotalElapsedTime * 9, 6.0f)) * vTileSize.x, 0 * vTileSize.y}, {vTileSize.x, vTileSize.y});
            }
            else*/
            {
              DrawPartialDecal({vWorld.x + moveXSelected, vWorld.y + moveYSelected}, sprIsom.Decal(), {9 * vTileSize.x, 0 * vTileSize.y}, {vTileSize.x, vTileSize.y});
            }
            break;
          }
        }
      }

      // Draw selected "yellow" frame
      {

        if (vSelectedPileTileType != 0) // hexequal(Hex(vSelected.x,vSelected.y,-vSelected.x-vSelected.y), Hex(Hex(q,r,s))) )
        {
          olc::vi2d vWorld = AxialToScreen(vSelectedPileTile.x, vSelectedPileTile.y);

          // move if cursor is on top of tile
          if (vSelected == vSelectedPileTile)
            DrawPartialDecal({vWorld.x, vWorld.y + 5 * std::sin(fTotalElapsedTime * 5)}, sprIsom.Decal(), {0 * vTileSize.x, 0 * vTileSize.y}, {vTileSize.x, vTileSize.y});
          else
            DrawPartialDecal({vWorld.x, vWorld.y + 0}, sprIsom.Decal(), {0 * vTileSize.x, 0 * vTileSize.y}, {vTileSize.x, vTileSize.y});
        }
      }

      // Draw Acorn
      {
        for (auto it = heightsAcorn.begin(); it != heightsAcorn.end(); ++it)
        {
          int moveXSelected = 0;
          int moveYSelected = 0;
          // highlight selected hexagon
          if (hexequal(Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y), it->first))
          {
            moveXSelected = 0.0;
            moveYSelected = 5 * std::sin(fTotalElapsedTime * 5);
          }

          olc::vi2d vWorld = AxialToScreen(it->first.get_q(), it->first.get_r());
          // Shadow
          DrawPartialDecal({vWorld.x + vTileSize.x / 6 - 3 * std::sin(fTotalElapsedTime * 5), vWorld.y - 0.0 * vTileSize.y + moveYSelected}, sprShadow.Decal(), {0 * 46, 0}, {46, 44}, {1.0f + 0.15 * std::sin(fTotalElapsedTime * 5), 1.0f}, olc::WHITE);
          // Acorn
          DrawPartialDecal({vWorld.x + vTileSize.x / 4, vWorld.y + 5 * std::sin(fTotalElapsedTime * 5) - 0.40 * vTileSize.y}, sprAcorn.Decal(), {(it->second - 1) * 32, 0}, {32, 32}, {1.0f, 1.0f}, olc::WHITE);
        }
      }

      // Draw Bird
      for (int i = 0; i < posBird.size(); i++)
      {
        int moveXSelected = 0;
        int moveYSelected = 0;

        // highlight selected hexagon
        if (hexequal(Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y), posBird.at(i)))
        {
          moveXSelected = 0.0;
          moveYSelected = 5 * std::sin(fTotalElapsedTime * 5);
        }
        olc::vi2d vWorld = AxialToScreen(posBird.at(i).get_q(), posBird.at(i).get_r());

        // shadow
        DrawPartialDecal({vWorld.x + vTileSize.x / 6 - 3 * std::sin(fTotalElapsedTime * 5), vWorld.y - 0.0 * vTileSize.y + moveYSelected}, sprShadow.Decal(), {0 * 46, 0}, {46, 44}, {1.0f + 0.15 * std::sin(fTotalElapsedTime * 5), 1.0f}, olc::MAGENTA);
        // Bird
        // DrawPartialDecal({vWorld.x+vTileSize.x/6, vWorld.y+5*std::sin(fTotalElapsedTime*5)-0.40*vTileSize.y}, sprBird.Decal(), {0 * 32, 0}, {32, 32},  { 1.0f,1.0f }, olc::WHITE);
        DrawPartialDecal({vWorld.x + vTileSize.x / 4 + moveXSelected, vWorld.y + moveYSelected - 0.75 * vTileSize.y}, sprBird.Decal(), {int(fmodf(fTotalElapsedTime * 6, 5.0f)) * 32, 0 * 32}, {32, 32});
      }

      // Draw collected acorn
      {
        for (int i = 0; i < (level + 9); i++)
        {
          // minimimum required Acorn
          DrawPartialDecal({ScreenWidth() / 2 - 18 / 2 * 32 - 16 + vTileSize.x / 4 + i * 32, ScreenHeight() - 75 + /*5 * std::sin(fTotalElapsedTime * 5)*/ -0.40 * vTileSize.y}, sprAcorn.Decal(), {3 * 32, 0}, {32, 32}, {1.0f, 1.0f}, olc::GREY);
        }

        for (int i = 0; i < numAcornCollected; i++)
        {
          // Acorn
          DrawPartialDecal({ScreenWidth() / 2 - 18 / 2 * 32 - 16 + vTileSize.x / 4 + i * 32, ScreenHeight() - 75 + /*5 * std::sin(fTotalElapsedTime * 5)*/ -0.40 * vTileSize.y}, sprAcorn.Decal(), {0 * 32, 0}, {32, 32}, {1.0f, 1.0f}, olc::WHITE);
        }

        DrawString(15, ScreenHeight() - 80, "Acorn: " + std::to_string(numAcornCollected) + "/" + std::to_string(level + 9), olc::BLACK);
      }

      // Draw Squirrel and it's shadow
      {
        int moveXSelected = 0;
        int moveYSelected = 0;

        // highlight selected hexagon
        if (hexequal(Hex(vSelected.x, vSelected.y, -vSelected.x - vSelected.y), posSquirrel))
        {
          moveXSelected = 0.0;
          moveYSelected = 5 * std::sin(fTotalElapsedTime * 5);
        }
        olc::vi2d vWorld = AxialToScreen(posSquirrel.get_q(), posSquirrel.get_r());

        // shadow
        if (isSquirrelAtDrey == false)
          DrawPartialDecal({vWorld.x + vTileSize.x / 4 - 3 * std::sin(fTotalElapsedTime * 5), vWorld.y - 0.05 * vTileSize.y + moveYSelected}, sprSquirrel.Decal(), {1 * 36, 0}, {36, 36}, {0.85f + 0.15 * std::sin(fTotalElapsedTime * 5), 1.0f}); //, olc::WHITE);
        // squirrel
        if (isSquirrelCatchedByBird == true)
        {
          DrawPartialDecal({vWorld.x + vTileSize.x / 6, vWorld.y + 5 * std::sin(fTotalElapsedTime * 5) - 0.40 * vTileSize.y}, sprSquirrel.Decal(), {2 * 36, 0}, {36, 36}, {1.0f, 1.0f}, olc::WHITE);
          DrawDecal({585, 485}, sprTagLoosing.Decal(), {1.0f, 1.0f}, olc::WHITE);
        }
        else
        {
          if (isSquirrelNotMovable == true)
          {
            DrawPartialDecal({vWorld.x + vTileSize.x / 6, vWorld.y + 5 * std::sin(fTotalElapsedTime * 5) - 0.40 * vTileSize.y}, sprSquirrel.Decal(), {3 * 36, 0}, {36, 36}, {1.0f, 1.0f}, olc::WHITE);
            DrawDecal({585, 485}, sprTagLoosing.Decal(), {1.0f, 1.0f}, olc::WHITE);
          }
          else
          {
            // winning condition
            if (isDreyOpen == true && isSquirrelAtDrey == true)
            {
              olc::vi2d vWorld = AxialToScreen(posDrey.get_q(), posDrey.get_r());
              DrawPartialDecal({vWorld.x /*+vTileSize.x/6*/, vWorld.y + /*5*std::sin(fTotalElapsedTime*5)*/ -0.40 * vTileSize.y}, sprDreyAnim.Decal(), {int(fmodf(fTotalElapsedTime * 3, 4.0f)) * 72, 0}, {72, 46}, {1.0f, 1.0f}, olc::WHITE);
            
              DrawDecal({595, 470}, sprTagSaved.Decal(), {1.0f, 1.0f}, olc::WHITE);
            }
            // draw normal squirrel
            else
              DrawPartialDecal({vWorld.x + vTileSize.x / 6, vWorld.y + 5 * std::sin(fTotalElapsedTime * 5) - 0.40 * vTileSize.y}, sprSquirrel.Decal(), {0 * 36, 0}, {36, 36}, {1.0f, 1.0f}, olc::WHITE);
          }
        }

        // draw drey
        if (isDreyOpen == true && isSquirrelAtDrey == false)
        {
          olc::vi2d vWorld = AxialToScreen(posDrey.get_q(), posDrey.get_r());
          DrawPartialDecal({vWorld.x /*+vTileSize.x/6*/, vWorld.y + /*5*std::sin(fTotalElapsedTime*5)*/ -0.40 * vTileSize.y}, sprDrey.Decal(), {0 * 36, 0}, {72, 46}, {1.0f, 1.0f}, olc::WHITE);
        }
      }

      // Draw ExitCross
      {
        DrawDecal({723, 8}, sprExitCross.Decal(), {1.0f, 1.0f}, olc::WHITE);
      }

      // Draw Debug Info
      DrawString(4, 4, "Mouse   : " + std::to_string(vMouse.x) + ", " + std::to_string(vMouse.y), olc::BLACK);
      DrawString(4, 14, "Selected: " + std::to_string(vSelected.x) + ", " + std::to_string(vSelected.y), olc::BLACK);
      DrawString(4, 24, "Style: " + sBoardName, olc::BLACK);
      DrawString(4, 34, "hexlist: " + std::to_string(hexlist.size()), olc::BLACK);
      DrawString(4, 44, "pile: " + std::to_string(deqPileOfTiles.size()), olc::BLACK);
      DrawString(4, 54, "level: " + std::to_string(level), olc::BLACK);
      DrawString(4, 64, "birds: " + std::to_string(posBird.size()), olc::BLACK);

      // DrawString(4, ScreenHeight()-20, "Use left and right mouse button to toggle tiles.", olc::BLACK);
      // DrawString(4, ScreenHeight()-10, "Use number keys 1-8 to toggle layout.", olc::BLACK);

      if (isDreyOpen == true && isSquirrelAtDrey == false)
        {
          DrawString(8, ScreenHeight() - 20, "All acorns collected. HURRY! RUN, RUN, RUN to the drey...", olc::BLACK);
          DrawDecal({575, 465}, sprTagRun.Decal(), {1.0f, 1.0f}, olc::WHITE);
        }
        
        
    }

    // Title screen
    if(nState == STATE::STATE_TITLE)
    {
      DrawDecal({0, 0}, sprScreenTitle.Decal(), {1.0f, 1.0f}, olc::WHITE);
    }

    // Help screen
    if(nState == STATE::STATE_HELP)
    {
      DrawDecal({0, 0}, sprScreenHelp.Decal(), {1.0f, 1.0f}, olc::WHITE);
    }

    // Credit screen
    if(nState == STATE::STATE_CREDIT)
    {
      DrawDecal({0, 0}, sprScreenCredits.Decal(), {1.0f, 1.0f}, olc::WHITE);
    }

    // if(nState == STATE::STATE_TITLE)
    // {
    //   // Graceful exit if user is in full screen mode
    //   return !GetKey(olc::Key::ESCAPE).bPressed;
    // }

    return true;
  }
};


int main()
{
  SquirrelOnTheRun SotR;
  if (SotR.Construct(800, 600, 1, 1, false, true)) //restrict to window mode and cropped framerate 
    SotR.Start();
  return 0;
}
