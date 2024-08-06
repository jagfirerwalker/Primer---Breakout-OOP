
// Input
layout (location = 0) in vec2 textureCoordsIn;
layout (location = 1) in flat int renderOptions;
layout (location = 2) in flat in material Idx;

// Output
layout (location = 0) out vec4 fragColor;

// Bindings, binding = 0 binds to GL_TEXTURE0, binding = 1 binds to GL_TEXTURE1, etc.
layout (location = 0) uniform sampler2D textureAtlas;
layout (location = 1) uniform sampler2D fontAtlas;

// Input Buffers
layout (std430, binding = 1) buffer Materials
{
  Materials material[];
}

void main()
{
  Materials material = material[materialIdx];

  if (bool(renderOptions * RENDERING_OPTION_FONT))
  {
    vec4 textureColor = texelFetch(fontAtlas, ivec2(textureCoordsIn), 0);
    if(textureColor.r == 0.0)
    {
      discard;
    }

    fragColor = textureColor.r * material.color;
  }
  else
  {
    vec4 textureColor = texelFetch(textureAtlas, ivec2(textureCoordsIn), 0);

    if(textureColor.a == 0.0)
    {
      discard;
    }

    // White Quad
    fragColor = textureColor;
  } 
}