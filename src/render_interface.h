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
struct Transform 
{
  Vec2 pos;          // Position of the sprite
  Vec2 size;         // Size of the sprite
  IVec2 atlasOffset; // Offset in the texture atlas
  IVec2 spriteSize;  // Size of the sprite in the texture atlas
};

struct RenderData
{
  int transformCount;                    // Number of transforms currently in use
  Transform transforms[MAX_TRANSFORMS];  // Array of transforms
};

// #############################################################################
//                           Renderer Globals
// #############################################################################
static RenderData* renderData; // Global render data instance

// #############################################################################
//                           Renderer Functions
// #############################################################################


void draw_sprite(SpriteID spriteID, Vec2 pos, Vec2 size)
{
  Sprite sprite = get_sprite(spriteID);

  Transform transform = {};
  transform.pos = pos;
  transform.size = size;
  transform.atlasOffset = sprite.atlasOffset;
  transform.spriteSize = sprite.spriteSize;

  renderData->transforms[renderData->transformCount++] = transform;
}