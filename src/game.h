#pragma once

#include "input.h"
#include "breaknotes_lib.h"
#include "render_interface.h"
#include <string>
#include <sstream>

// #############################################################################
//                           Game Globals
// #############################################################################

// How many times should the game update per second
constexpr int UPDATES_PER_SECOND = 120;
// How long should the game wait before updating again
constexpr double UPDATE_DELAY = 1.0 / UPDATES_PER_SECOND;
constexpr int WORLD_WIDTH = 320;
constexpr int WORLD_HEIGHT = 180;
constexpr int TILESIZE = 8;
constexpr IVec2 WORLD_GRID = {WORLD_WIDTH / TILESIZE, WORLD_HEIGHT / TILESIZE}; // create grid of tiles 320/8 = 40, 180/8 = 22

// #############################################################################
//                           Game Structs
// #############################################################################
enum GameInputType
{
  MOVE_LEFT,
  MOVE_RIGHT,
  MOVE_UP,
  MOVE_DOWN,
  JUMP,

  MOUSE_LEFT,
  MOUSE_RIGHT,

  GAME_INPUT_COUNT
};

struct KeyMapping
{
  Array<KeyCodeID, 3> keys;
};

struct Tile
{
  int neighbourMask;
  bool isVisible;
};

struct Player
{
  IVec2 pos;
  IVec2 prevPos;
};

struct GameState
{
  float updateTimer;
  bool initialized = false;
  Player player;

  Array<IVec2, 21> tileCoords;
  Tile worldGrid[WORLD_GRID.x][WORLD_GRID.y];
  
  KeyMapping keyMappings[GAME_INPUT_COUNT];

  float fpsUpdateTimer;
  int frameCount;
  float currentFps;
};


// #############################################################################
//                           Game Globals
// #############################################################################
static GameState* gameState;

// #############################################################################
//                           Game Functions (Exposed)
// #############################################################################
extern "C"
{
  EXPORT_FN void update_game(GameState* gameStateIn, RenderData* renderDataIn, Input* inputIn, float dt);
}