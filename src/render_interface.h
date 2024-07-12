#pragma once // Ensures this header is only included once

#include "assets.h"
#include "breaknotes_lib.h"

// #############################################################################
//                           Renderer Constants
// #############################################################################
constexpr int MAX_TRANSFORMS = 1000; // Maximum number of transforms allowed

// #############################################################################
//                           Renderer Structs
// #############################################################################
struct OrthographicCamera2D
{
  float zoom = 1.0f;
  Vec2 dimensions;
  Vec2 position;
};


struct Transform 
{
  Vec2 pos;          // Position of the sprite
  Vec2 size;         // Size of the sprite
  IVec2 atlasOffset; // Offset in the texture atlas
  IVec2 spriteSize;  // Size of the sprite in the texture atlas
};

struct RenderData
{
  OrthographicCamera2D gameCamera;      // Camera used to render the game
  OrthographicCamera2D uiCamera;        // Camera used to render the UI
  int transformCount;                    // Number of transforms currently in use
  Transform transforms[MAX_TRANSFORMS];  // Array of transforms
};

// #############################################################################
//                           Renderer Globals
// #############################################################################
static RenderData* renderData; // Global render data instance

// #############################################################################
//                           Renderer Untility
// #############################################################################

IVec2 screen_to_world(IVec2 screenPos)
{
  OrthographicCamera2D camera = renderData->gameCamera;

  int xPos = screenPos.x / input->screenSize.x * (int)camera.dimensions.x;

  // Offset using dimensions and position
  xPos += -camera.dimensions.x / 2.0f + camera.position.x;

  int yPos = screenPos.y / input->screenSize.y * (int)camera.dimensions.y;

  // Offset using dimensions and position
  yPos += -camera.dimensions.y / 2.0f + camera.position.y;

  return {xPos, yPos};
}

// #############################################################################
//                           Renderer Functions
// #############################################################################


void draw_sprite(SpriteID spriteID, Vec2 pos)
{
  Sprite sprite = get_sprite(spriteID);

  Transform transform = {};
  transform.pos = pos - vec_2(sprite.spriteSize) / 2.0f;
  transform.size = vec_2(sprite.spriteSize);
  transform.atlasOffset = sprite.atlasOffset;
  transform.spriteSize = sprite.spriteSize;

  renderData->transforms[renderData->transformCount++] = transform;
}

void draw_sprite(SpriteID spriteID, IVec2 pos)
{
  draw_sprite(spriteID, vec_2(pos));
}