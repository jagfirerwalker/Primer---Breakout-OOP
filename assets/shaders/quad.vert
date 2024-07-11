#version 430 core

// Structs
struct Transform
{
    vec2 pos;
    vec2 size;
    ivec2 atlasOffset;
    ivec2 spriteSize;
};

// Input
layout (std430, binding = 0) buffer TransformSBO
{
  Transform transforms[];
};

uniform vec2 screenSize;
uniform mat4 orthoProjection;
 
// Output
layout (location = 0) out vec2 textureCoordsOut;

void main()
{
    Transform transform = transforms[gl_InstanceID];
    vec2 vertices[6] =
    {
        transform.pos,
        vec2(transform.pos + vec2(0.0, transform.size.y)),
        vec2(transform.pos + vec2(transform.size.x, 0.0)),
        vec2(transform.pos + vec2(transform.size.x, 0.0)),
        vec2(transform.pos + vec2(0.0, transform.size.y)),
        transform.pos + transform.size
    };

    float left = transform.atlasOffset.x;
    float top = transform.atlasOffset.y;
    float right = transform.atlasOffset.x + transform.spriteSize.x;
    float bottom = transform.atlasOffset.y + transform.spriteSize.y;

    vec2 textureCoords[6] =
    {
        vec2(left, top),
        vec2(left, bottom),
        vec2(right, top),
        vec2(right, top),
        vec2(left, bottom),
        vec2(right, bottom)
    };

    // Sets the position of the vertex in clip space.
    // Normalize Position
    {
      vec2 vertexPos = vertices[gl_VertexID];
      // vertexPos.y = -vertexPos.y + screenSize.y; // needs to be inverted due to OpenGL's coordinate system
      // vertexPos = 2.0 * (vertexPos / screenSize) - 1.0; // normalize to [-1, 1]
      gl_Position = orthoProjection * vec4(vertexPos, 0.0, 1.0);
    }

    textureCoordsOut = textureCoords[gl_VertexID];
}