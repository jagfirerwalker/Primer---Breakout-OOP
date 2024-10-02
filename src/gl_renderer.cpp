#include "gl_renderer.h"
#include "render_interface.h"

// To Load PNG Files
#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"

// #############################################################################
//                           OpenGL Constants
// #############################################################################
const char* TEXTURE_PATH = "assets/textures/TEXTURE_ATLAS.png";


// #############################################################################
//                           OpenGL Structs
// #############################################################################
struct GLContext
{
  GLuint programID;
  GLuint textureID;
  GLuint transformSBOID;
  GLuint screenSizeID;
  GLuint orthoProjectionID;

  long long textureTimestamp;
  long long shaderTimestamp;
};

// #############################################################################
//                           OpenGL Globals
// #############################################################################
static GLContext glContext;

// #############################################################################
//                           OpenGL Functions
// #############################################################################
static void APIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                         GLsizei length, const GLchar* message, const void* user)
{
    const char* sourceStr;
    switch(source)
    {
        case GL_DEBUG_SOURCE_API: sourceStr = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceStr = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: sourceStr = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION: sourceStr = "Application"; break;
        case GL_DEBUG_SOURCE_OTHER: sourceStr = "Other"; break;
        default: sourceStr = "Unknown"; break;
    }

    const char* typeStr;
    switch(type)
    {
        case GL_DEBUG_TYPE_ERROR: typeStr = "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr = "Undefined Behavior"; break;
        case GL_DEBUG_TYPE_PORTABILITY: typeStr = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE: typeStr = "Performance"; break;
        case GL_DEBUG_TYPE_MARKER: typeStr = "Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP: typeStr = "Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP: typeStr = "Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER: typeStr = "Other"; break;
        default: typeStr = "Unknown"; break;
    }

    const char* severityStr;
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_HIGH: severityStr = "High"; break;
        case GL_DEBUG_SEVERITY_MEDIUM: severityStr = "Medium"; break;
        case GL_DEBUG_SEVERITY_LOW: severityStr = "Low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Notification"; break;
        default: severityStr = "Unknown"; break;
    }

    printf("OpenGL Debug - Source: %s, Type: %s, ID: %u, Severity: %s\n", sourceStr, typeStr, id, severityStr);
    printf("Message: %s\n", message);

    if(severity == GL_DEBUG_SEVERITY_HIGH)
    {
        SM_ASSERT(false, "Critical OpenGL error");
    }
}

GLuint gl_create_shader(int shaderType, char* shaderPath, BumpAllocator* transientStorage)
{
  int fileSize = 0;
  char* shaderHeader = read_file("src/shader_header.h", &fileSize, transientStorage);
  char* shaderSource = read_file(shaderPath, &fileSize, transientStorage);

  if(!shaderHeader)
  {
    SM_ASSERT(false, "Failed to load shader_header.h");
    return 0;
  }

  if(!shaderSource)
  {
    // SM_ASSERT(false, "Failed to load shader: %s",shaderPath);
    SM_ERROR("Failed to load shader: %s. Error: %s", shaderPath, strerror(errno));
    return 0;
  }


  char* shaderSources[] =
  {
    "#version 430 core\n",
    shaderHeader,
    shaderSource
  };


  GLuint shaderID = glCreateShader(shaderType);
  glShaderSource(shaderID, ArraySize(shaderSources), shaderSources, 0);
  glCompileShader(shaderID);

  GLint compileStatus;
  glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compileStatus);
  if (compileStatus != GL_TRUE)
  {
      GLint logLength;
      glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
      char* log = (char*)malloc(logLength);
      glGetShaderInfoLog(shaderID, logLength, NULL, log);
      
      printf("Shader compilation failed for %s:\n%s\n", shaderPath, log);
      
      free(log);
      glDeleteShader(shaderID);
      return 0;
  }

  return shaderID;
}


bool gl_init(BumpAllocator* transientStorage)
{
  load_gl_functions();

  const char* glVersion = (const char*)glGetString(GL_VERSION);
  const char* glslVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    if (glVersion && glslVersion) {
        printf("OpenGL Version: %s\n", glVersion);
        printf("GLSL Version: %s\n", glslVersion);
    } else {
        printf("Unable to get OpenGL or GLSL version.\n");
    };

  glDebugMessageCallback(&gl_debug_callback, nullptr);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glEnable(GL_DEBUG_OUTPUT);

  GLuint vertShaderID = gl_create_shader(GL_VERTEX_SHADER, 
                                         "assets/shaders/quad.vert", transientStorage);
  GLuint fragShaderID = gl_create_shader(GL_FRAGMENT_SHADER, 
                                         "assets/shaders/quad.frag", transientStorage);
  if(!vertShaderID || !fragShaderID)
  {
    SM_ASSERT(false, "Failed to create Shaders");
    return false;
  }

  long long timestampVert = get_timestamp("assets/shaders/quad.vert");
  long long timestampFrag = get_timestamp("assets/shaders/quad.frag");
  glContext.shaderTimestamp = max(timestampVert, timestampFrag);

  glContext.programID = glCreateProgram();
  glAttachShader(glContext.programID, vertShaderID);
  glAttachShader(glContext.programID, fragShaderID);
  glLinkProgram(glContext.programID);

  glDetachShader(glContext.programID, vertShaderID);
  glDetachShader(glContext.programID, fragShaderID);
  glDeleteShader(vertShaderID);
  glDeleteShader(fragShaderID);

  // This has to be done, otherwise OpenGL will not draw anything
  GLuint VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // Texture Loading using STBI
  {
    int width, height, channels;
    char* data = (char*)stbi_load(TEXTURE_PATH, &width, &height, &channels, 4);
    if(!data)
    {
      SM_ASSERT(false, "Failed to load texture");
      return false;
    }

    glGenTextures(1, &glContext.textureID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, glContext.textureID);

    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // This setting only matters when using the GLSL texture() function
    // When you use texelFetch() this setting has no effect,
    // because texelFetch is designed for this purpose
    // See: https://interactiveimmersive.io/blog/glsl/glsl-data-tricks/
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glContext.textureTimestamp = get_timestamp(TEXTURE_PATH);

    stbi_image_free(data);
  }

  // Transform Storage Buffer
  {
    glGenBuffers(1, &glContext.transformSBOID);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, glContext.transformSBOID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Transform) * renderData->transforms.maxElements,
                 renderData->transforms.elements, GL_DYNAMIC_DRAW);
  }

  // Uniforms
  {
    glContext.screenSizeID = glGetUniformLocation(glContext.programID, "screenSize");
    glContext.orthoProjectionID = glGetUniformLocation(glContext.programID, "orthoProjection");
  }
  
  // sRGB output (even if input texture is non-sRGB -> don't rely on texture used)
  // Your font is not using sRGB, for example (not that it matters there, because no actual color is sampled from it)
  // But this could prevent some future bug when you start mixing different types of textures
  // Of course, you still need to correctly set the image file source format when using glTexImage2D()
  glEnable(GL_FRAMEBUFFER_SRGB);
  glDisable(0x809D); // disable multisampling

  // Depth Tesing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_GREATER);

  // Use Program
  glUseProgram(glContext.programID);

  return true;
}

void gl_render(BumpAllocator* transientStorage)
{
  // Texture Hot Reloading
  {
    long long currentTimestamp = get_timestamp(TEXTURE_PATH);

    if(currentTimestamp > glContext.textureTimestamp)
    {    
      glActiveTexture(GL_TEXTURE0);
      int width, height, nChannels;
      char* data = (char*)stbi_load(TEXTURE_PATH, &width, &height, &nChannels, 4);
      if(data)
      {
        glContext.textureTimestamp = currentTimestamp;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
      }
    }
  }

  // Shader Hot Reloading
  {
    long long timestampVert = get_timestamp("assets/shaders/quad.vert");
    long long timestampFrag = get_timestamp("assets/shaders/quad.frag");
    
    if(timestampVert > glContext.shaderTimestamp ||
       timestampFrag > glContext.shaderTimestamp)
    {
      // Add a small delay to allow file operations to complete
      Sleep(100);

      GLuint vertShaderID = gl_create_shader(GL_VERTEX_SHADER, 
                                              "assets/shaders/quad.vert", transientStorage);
      GLuint fragShaderID = gl_create_shader(GL_FRAGMENT_SHADER, 
                                              "assets/shaders/quad.frag", transientStorage);
      if(!vertShaderID || !fragShaderID)
      {
        SM_ASSERT(false, "Failed to create Shaders");
        return;
      }
      
      // This code creates an OpenGL shader program by:

      // Creating a program object.
      // Attaching vertex and fragment shaders to it.
      // Linking the shaders into a complete program.
      // Detaching and deleting the shaders to free up resources.
      GLuint programID = glCreateProgram();
      glAttachShader(programID, vertShaderID);
      glAttachShader(programID, fragShaderID);
      glLinkProgram(programID);
      glDetachShader(programID, vertShaderID);
      glDetachShader(programID, fragShaderID);
      glDeleteShader(vertShaderID);
      glDeleteShader(fragShaderID);

      // Validate if program works
      {
        int programSuccess;
        char programInfoLog[512];
        glGetProgramiv(programID,GL_LINK_STATUS, &programSuccess);

        if(!programSuccess)
        {
          glGetProgramInfoLog(programID, 512, 0, programInfoLog);
          
          SM_ASSERT(0, "Failed to link program: %s", programInfoLog);
          return;
        }
      }
      glDeleteProgram(glContext.programID);
      glContext.programID = programID;
      glUseProgram(programID);

      glContext.shaderTimestamp = max(timestampVert, timestampFrag);
    }
  }

  glClearColor(119.0f / 255.0f, 33.0f / 255.0f, 111.0f / 255.0f, 1.0f);
  glClearDepth(0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, input->screenSize.x, input->screenSize.y);

  // Copy screen size to the GPU
  {
    struct Vec2
    {
      float x;
      float y;
    };
    Vec2 screenSize = {(float)input->screenSize.x, (float)input->screenSize.y};
    glUniform2fv(glContext.screenSizeID, 1, &screenSize.x);
  }
  
  // Orthographic Projection
  OrthographicCamera2D camera = renderData->gameCamera;
  Mat4 orthoProjection = orthographic_projection(camera.position.x - camera.dimensions.x / 2.0f, 
                                                 camera.position.x + camera.dimensions.x / 2.0f, 
                                                 camera.position.y - camera.dimensions.y / 2.0f, 
                                                 camera.position.y + camera.dimensions.y / 2.0f);
  glUniformMatrix4fv(glContext.orthoProjectionID, 1, GL_FALSE, &orthoProjection.ax);

  // Opaque Objects
  {
    // Copy transforms to the GPU
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Transform) * renderData->transforms.count,
                    renderData->transforms.elements);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, renderData->transforms.count);

    // Reset for next Frame
    renderData->transforms.count = 0;
  }
}