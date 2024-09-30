#version 430 core

// Input
layout (location = 0) in vec2 textureCoordsIn;
layout (location = 1) in flat int renderOptions;
layout (location = 2) in flat int materialIdx;

// Output
layout (location = 0) out vec4 fragColor;

// Bindings
layout (binding = 0) uniform sampler2D textureAtlas;
layout (binding = 1) uniform sampler2D fontAtlas;

// Input Buffer
layout (std430, binding = 1) buffer Materials
{
  Material materials[];
}
void main()
{
  vec4 textureColor = texelFetch(textureAtlas, ivec2(textureCoordsIn), 0);

  if(textureColor.a == 0.0)
  {
    discard;
  }

  // White Quad
  fragColor = textureColor;
}