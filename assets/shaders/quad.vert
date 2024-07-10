#version 430 core

void main()
{
  // OpenGL Coordinates
  // -1/ 1                1/ 1
  // -1/-1                1/-1
  vec2 vertices[6] = 
  {
    // Top Left
    vec2(-0.5, 0.5),

    // Bottom Left
    vec2(-0.5, -0.5),

    // Top Right
    vec2(0.5, 0.5),

    // Top Right
    vec2(0.5, 0.5),
    
    // Bottom Left
    vec2(-0.5, -0.5)

    // Bottom Right
    vec2(0.5, -0.5)
  };

  /**
   * Sets the position of the vertex in clip space.
   * 
   * @param vertices - The array of vertices.
   * @param gl_VertexID - The ID of the current vertex.
   */
  gl_Position = vec4(vertices[gl_VertexID], 1.0, 1.0);


}