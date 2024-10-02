// This line prevents the header file from being included multiple times
#pragma once

// Including a custom header file that likely contains some utility functions or definitions
#include "breaknotes_lib.h"

// #############################################################################
//                           Assets Constants
// #############################################################################

// #############################################################################
//                           Assets Structs
// #############################################################################

// An enumeration (a set of named constants) for different sprite types
// Currently, there's only one sprite type (DICE) and a COUNT to keep track of total sprites

enum SpriteID
{
  SPRITE_WHITE,
  SPRITE_DICE,

  SPRITE_COUNT
};

// A structure to hold information about a sprite
// It contains the sprite's position in the texture atlas and its size
struct Sprite
{
  IVec2 atlasOffset;
  IVec2 spriteSize;
  int framecCount = 1;
};

// #############################################################################
//                           Assets Functions
// #############################################################################

// A function that returns a Sprite structure based on the given SpriteID
Sprite get_sprite(SpriteID spriteID)
{
  Sprite sprite = {};
  sprite.framecCount = 1;

  switch(spriteID)
  {
    case SPRITE_WHITE:
    {
      sprite.atlasOffset = {0, 0};
      sprite.spriteSize = {1, 1};

      break;
    }

    case SPRITE_DICE:
    {
      sprite.atlasOffset = {16, 0};
      sprite.spriteSize = {16, 16};

      break;
    }
  }

  return sprite;
}