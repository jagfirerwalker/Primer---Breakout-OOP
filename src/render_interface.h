#pragma once // Ensures this header is only included once

#define ENGINE

#include "assets.h"
#include "shader_header.h"
#include "breaknotes_lib.h"
#include "shader_header.h"

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
  Array<Transform, 1000> uiTransforms;   // Array of transforms to render for the UI
};

// #############################################################################
//                           Renderer Globals
// #############################################################################
static RenderData* renderData; // Global render data instance

// #############################################################################
//                           Renderer Utility
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

int get_material_idx(Material material = {})
{
  // convert from SRGB to linear color space, to be used in the shader
  material.color.r = powf(material.color.r, 2.2f);
  material.color.g = powf(material.color.g, 2.2f);
  material.color.b = powf(material.color.b, 2.2f);
  material.color.a = powf(material.color.a, 2.2f);

  for (int materialIdx = 0; materialIdx < renderData->materials.count; materialIdx++)
  {
    if(renderData->materials[materialIdx] == material)
    {
      return materialIdx;
    }
  }

  return renderData->materials.add(material);
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

void draw_sprite(SpriteID spriteID, Vec2 pos, DrawData drawData = {})
{
  Sprite sprite = get_sprite(spriteID);

  Transform transform = {};
  transform.materialIdx = get_material_idx(drawData.material);
  transform.pos = pos - vec_2(sprite.spriteSize) / 2.0f;
  transform.size = vec_2(sprite.spriteSize);
  transform.atlasOffset = sprite.atlasOffset;
  transform.spriteSize = sprite.spriteSize;
  transform.renderOptions = drawData.renderOptions;

  renderData->transforms.add(transform);
}

void draw_sprite(SpriteID spriteID, IVec2 pos, DrawData drawData = {})
{
  draw_sprite(spriteID, vec_2(pos), drawData);
}

// #############################################################################
//                     Render Interface UI Font Rendering
// #############################################################################

void draw_ui_text(char* text, Vec2 pos, TextData textData = {})
{
  SM_ASSERT(text, "Text is null");
  if(!text)
  {
    return;
  }

  Vec2 origin = pos;
  while(char c = *(text++))
  {
    if(c == '\n')
    {
      pos.x = origin.x;
      pos.y += renderData->fontHeight * textData.fontSize;
      continue;
    }

    Glyph glyph = renderData->glyphs[c];
    Transform transform = {};
    transform.materialIdx = get_material_idx(textData.material);
    transform.pos.x = pos.x + glyph.offset.x * textData.fontSize;
    transform.pos.y = pos.y + glyph.offset.y * textData.fontSize;
    transform.atlasOffset = glyph.textureCoords;
    transform.spriteSize = glyph.size;
    transform.size = vec_2(glyph.size) * textData.fontSize;
    transform.renderOptions = textData.renderOptions | RENDERING_OPTION_FONT;

    renderData->uiTransforms.add(transform);

    // Advance the Glyph
    pos.x += glyph.advance.x * textData.fontSize;
  }
}

template <typename... Args>
void draw_format_ui_text(char* format, Vec2 pos, Args... args)
{
  char* text = format_text(format, args...);
  draw_ui_text(text, pos);
}