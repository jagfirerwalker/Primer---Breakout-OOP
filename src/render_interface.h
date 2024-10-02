#pragma once // Ensures this header is only included once

#include "assets.h"
#include "shader_header.h"
#include "breaknotes_lib.h"

// #############################################################################
//                           Renderer Constants
// #############################################################################

int RENDER_OPTION_FLIP_X = BIT(0);
int RENDER_OPTION_FLIP_Y = BIT(1);

// #############################################################################
//                           Renderer Structs
// #############################################################################
struct OrthographicCamera2D
{
  float zoom = 1.0f;
  Vec2 dimensions;
  Vec2 position;
};

struct RenderData
{
  OrthographicCamera2D gameCamera;      // Camera used to render the game
  OrthographicCamera2D uiCamera;        // Camera used to render the UI

  Array<Transform, 1000> transforms;     // Array of transforms to render
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

  int xPos = (float)screenPos.x / (float)input->screenSize.x * camera.dimensions.x;

  // Offset using dimensions and position
  xPos += -camera.dimensions.x / 2.0f + camera.position.x;

  int yPos = (float)screenPos.y / (float)input->screenSize.y * camera.dimensions.y;

  // Offset using dimensions and position
  yPos += camera.dimensions.y / 2.0f + camera.position.y;

  return {xPos, yPos};
}

// #############################################################################
//                           Renderer Functions
// #############################################################################

void draw_quad(Transform  transform)
{
  renderData->transforms.add(transform);
}

void draw_quad(Vec2 pos, Vec2 size)
{
  Transform transform = {};
  transform.pos = pos - size / 2.0f;
  transform.size = size;
  transform.atlasOffset = {0, 0};
  transform.spriteSize = {1, 1};

  renderData->transforms.add(transform);
}

void draw_sprite(SpriteID spriteID, Vec2 pos)
{
  Sprite sprite = get_sprite(spriteID);


  Transform transform = {};
  transform.pos = pos - vec_2(sprite.spriteSize) / 2.0f;
  transform.size = vec_2(sprite.spriteSize);
  transform.atlasOffset = sprite.atlasOffset;
  transform.spriteSize = sprite.spriteSize;

  renderData->transforms.add(transform);
}

void draw_sprite(SpriteID spriteID, IVec2 pos)
{
  draw_sprite(spriteID, vec_2(pos));
}