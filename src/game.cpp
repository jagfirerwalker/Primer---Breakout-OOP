#include "game.h"
#include "assets.h"


// #############################################################################
//                           Game Constants
// #############################################################################

// #############################################################################
//                           Game Structs
// #############################################################################

// #############################################################################
//                           Game Functions
// #############################################################################

// Check if a specific game input was just pressed
bool just_pressed(GameInputType type)
{
  KeyMapping mapping = gameState->keyMappings[type];
  for(int idx = 0; idx < mapping.keys.count; idx++)
  {
    if(input->keys[mapping.keys[idx]].justPressed)
    {
      return true;
    }
  }

  return false;
}

// Check if a specific game input is currently being held down
bool is_down(GameInputType type)
{
  KeyMapping mapping = gameState->keyMappings[type];
  for(int idx = 0; idx < mapping.keys.count; idx++)
  {
    if(input->keys[mapping.keys[idx]].isDown)
    {
      return true;
    }
  }
  return false;
}

// Get a pointer to a tile at the specified grid coordinates
Tile* get_tile(int x, int y)
{
  Tile* tile = nullptr;
    // Check if the coordinates are within the world grid boundaries
  if(x >= 0 && x < WORLD_GRID.x && y >= 0 && y < WORLD_GRID.y)
  {
    tile = &gameState->worldGrid[x][y];
  }

  return tile;
}

// Get a pointer to a tile at the specified world position
Tile* get_tile(IVec2 worldPos) // convert world position to tile position
{
  // Convert world coordinates to grid coordinates by dividing by TILESIZE
  int x = worldPos.x / TILESIZE;
  int y = worldPos.y / TILESIZE;

  return get_tile(x, y);
}

// Main simulation function, called at a fixed time step
void simulate()
{
  // Update Player
  {
    // Store the previous position for interpolation
    gameState->player.prevPos = gameState->player.pos;

    // Update player position based on input
    if(is_down(MOVE_LEFT))
    {
      gameState->player.pos.x -= 1;
    }
    if(is_down(MOVE_RIGHT))
    {
      gameState->player.pos.x += 1;
    }
    if(is_down(MOVE_UP))
    {
      gameState->player.pos.y -= 1;
    }
    if(is_down(MOVE_DOWN))
    {
      gameState->player.pos.y += 1;
    }
  }

  bool updateTiles = false;
  // Handle left mouse button (make tiles visible)
  if(is_down(MOUSE_LEFT))
  {
    IVec2 worldPos = screen_to_world(input->mousePos);
    IVec2 mousePosWorld = input->mousePosWorld;
    Tile* tile = get_tile(worldPos);

    if(tile)
    {
      tile->isVisible = true;
      updateTiles = true;
    }
  }

  // Handle right mouse button (make tiles invisible)
  if(is_down(MOUSE_RIGHT))
  {
    IVec2 worldPos = screen_to_world(input->mousePos);
    IVec2 mousePosWorld = input->mousePosWorld;
    Tile* tile = get_tile(worldPos);
    if(tile)
    {
      tile->isVisible = false;
      updateTiles = true;
    }
  }

  if(updateTiles)
    // Update tile neighbor masks if any tiles changed visibility
  {
    // Neighbouring Tiles        Top    Left      Right       Bottom  
    int neighbourOffsets[24] = { 0,-1,  -1, 0,     1, 0,       0, 1,   
    //                          Topleft Topright Bottomleft Bottomright
                                -1,-1,   1,-1,    -1, 1,       1, 1,
    //                           Top2   Left2     Right2      Bottom2
                                 0,-2,  -2, 0,     2, 0,       0, 2};

    // Topleft     = BIT(4) = 16
    // Toplright   = BIT(5) = 32
    // Bottomleft  = BIT(6) = 64
    // Bottomright = BIT(7) = 128

    for(int y = 0; y < WORLD_GRID.y; y++)
    {
      for(int x = 0; x < WORLD_GRID.x; x++)
      {
        Tile* tile = get_tile(x, y);
        if(!tile->isVisible) // if tile is not visible, continue
        {
          continue;
        }

        tile->neighbourMask = 0;
        int neighbourCount = 0;
        int extendedNeighbourCount = 0;
        int emptyNeighbourSlot = 0;

        // Look at the surrounding  12 surrounding 
        for(int n = 0; n < 12; n++)
        {
          Tile* neighbour = get_tile(x + neighbourOffsets[n * 2], y + neighbourOffsets[n * 2 + 1]);

          // No neighbour means the edge of the world
          if(!neighbour || neighbour->isVisible)
          {
            tile->neighbourMask |= BIT(n);
            if(n < 8) // Counting direct neighbors
            {
              neighbourCount++;
            }
            else // Counting neighbors 1 Tile away
            {
              extendedNeighbourCount++;
            }
          }
          else if(n < 8)
          {
            emptyNeighbourSlot = n;
          }
        }

        // Determine the final neighbor mask based on surrounding tiles
        if(neighbourCount == 7 && emptyNeighbourSlot >= 4) // We have a corner
        {
          tile->neighbourMask = 16 + (emptyNeighbourSlot - 4);
        }
        else if(neighbourCount == 8 && extendedNeighbourCount == 4)
        {
          tile->neighbourMask = 20;
        }
        else
        {
          tile->neighbourMask = tile->neighbourMask & 0b1111;
        }
      }
    }
  }
}

// #############################################################################
//                           Game Functions(exposed)
// #############################################################################
// Main game update function, called every frame
EXPORT_FN void update_game(GameState* gameStateIn, RenderData* renderDataIn, Input* inputIn, float dt)
{
  // Update global pointers if they've changed
  if(renderData != renderDataIn)
  {
    gameState = gameStateIn;
    renderData = renderDataIn;
    input = inputIn;
  }
  // One-time initialization
  if(!gameState->initialized)
  {
    renderData->gameCamera.dimensions = {WORLD_WIDTH, WORLD_HEIGHT};
    gameState->initialized = true;

    // Initialize Tileset
    {
      IVec2 tilesPosition = {48, 0};

      for(int y = 0; y < 5; y++)
      {
        for(int x = 0; x < 4; x++)
        {
          gameState->tileCoords.add({tilesPosition.x + x * TILESIZE, tilesPosition.y + y * TILESIZE}); // take tile position add x and y * tilesize, to get the end position of the tile.
        }
      }

    // Black inside
    gameState->tileCoords.add({tilesPosition.x, tilesPosition.y + 5 * TILESIZE});
    }

    // Key Mappings
    {
      gameState->keyMappings[MOVE_UP].keys.add(KEY_W);
      gameState->keyMappings[MOVE_UP].keys.add(KEY_UP);
      gameState->keyMappings[MOVE_LEFT].keys.add(KEY_A);
      gameState->keyMappings[MOVE_LEFT].keys.add(KEY_LEFT);
      gameState->keyMappings[MOVE_DOWN].keys.add(KEY_S);
      gameState->keyMappings[MOVE_DOWN].keys.add(KEY_DOWN);
      gameState->keyMappings[MOVE_RIGHT].keys.add(KEY_D);
      gameState->keyMappings[MOVE_RIGHT].keys.add(KEY_RIGHT);
      gameState->keyMappings[MOUSE_LEFT].keys.add(KEY_MOUSE_LEFT);
      gameState->keyMappings[MOUSE_RIGHT].keys.add(KEY_MOUSE_RIGHT);
    }

    renderData->gameCamera.position.x = 160;
    renderData->gameCamera.position.y = -90;
  }

  // Fixed Update Loop
  {
    gameState->updateTimer += dt;
    while(gameState->updateTimer >= UPDATE_DELAY)
    {
      gameState->updateTimer -= UPDATE_DELAY;
      simulate(); // draw tiles and update player

      // Relative Mouse here, because more frames than simulation
      input->relMouse = input->mousePos - input->prevMousePos;
      input->prevMousePos = input->mousePos;

      // Clear the transition count for every key
      {
        for (int keyCode = 0; keyCode < KEY_COUNT; keyCode++)
        {
          input->keys[keyCode].justPressed = false;
          input->keys[keyCode].justReleased = false;
          input->keys[keyCode].halfTransitionCount = 0;
        }
      }
    }
  }

  // Calculate interpolation factor for smooth rendering between fixed updates
  float interpolatedDT = (float)(gameState->updateTimer / UPDATE_DELAY);

  // Draw Player
  {
    Player& player = gameState->player;
    IVec2 playerPos = lerp(player.prevPos, player.pos, interpolatedDT);
    draw_sprite(SPRITE_DICE, playerPos);
  }

  // Drawing Tileset
  {
    for (int y = 0; y < WORLD_GRID.y; y++)
    {
      for (int x = 0; x < WORLD_GRID.x; x++)
      {
        Tile* tile = get_tile(x, y);
        if (!tile->isVisible)
        {
          continue;
        }

        // Draw Tile
        Transform transform = {};
        // Draw the Tile around the center
        transform.pos = {x * (float) TILESIZE, y * (float) TILESIZE};
        transform.size = {TILESIZE, TILESIZE};
        transform.spriteSize = {TILESIZE, TILESIZE};
        // Select the appropriate tile sprite based on its neighbor mask
        transform.atlasOffset = gameState->tileCoords[tile->neighbourMask];
        draw_quad(transform);
      }
    }
  } 
}