//SDL2 flashing random color example
//Should work on iOS/Android/Mac/Windows/Linux

#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "image.h"
#include "log.h"
#include "texture.h"
#include "program.h"

#include <stdlib.h> //rand()

static bool quitting = false;
static float r = 0.0f;
static SDL_Window *window = NULL;
static SDL_GLContext gl_context;
static SDL_Renderer *renderer = NULL;

// [0, 255]
float GrayToLuma(float g)
{
    return 0.859f * g + 16.0f;
}

// [0, 255]
float LumaToGray(float y)
{
    return (y - 16.0f) / 0.859f;
}

// [0, 255]
float linearToSRGB(float i)
{
    float l = i / 255.0f;
    float s;
    if (l <= 0.0031308f)
    {
        s = l * 12.92f;
    }
    else
    {
        s = 1.055f * glm::pow(l, 1.0f/2.4f) - 0.055f;
    }
    return s * 255.0f;
}

void processImage(Image& img)
{
    // convert from RGB to YUV

    // BT.709
    for (uint32_t y = 0; y < img.height; y++)
    {
        for (uint32_t x = 0; x < img.width; x++)
        {
            uint8_t* p = img.data.data() + y * img.width * 3 + x * 3;

            float R = *p;
            float G = *(p + 1);
            float B = *(p + 2);

            float Y = 16.0f + 0.0620071f * B + 0.614231f * G + 0.182586f * R;
            float U = 128.0f + 0.439216f * B - 0.338572f * G - 0.100644f * R;
            float V = 128.0f - 0.0402735f * B - 0.398942f * G + 0.439216f * R;

            *p = (uint8_t)glm::clamp(Y, 0.0f, 255.0f);
            *(p + 1) = (uint8_t)glm::clamp(U, 0.0f, 255.0f);
            *(p + 2) = (uint8_t)glm::clamp(V, 0.0f, 255.0f);
        }
    }

    /*
    // apply 2.2 gamma to each pixel.
    for (auto& i : img.data)
    {
        i = (uint8_t)glm::clamp(linearToSRGB(i), 0.0f, 255.0f);
    }
    */
}

void dumpTable()
{
    Log::printf("static uint8_t table[256] =\n{\n");
    for (int i = 0; i < 32; i++)
    {
        Log::printf("    ");
        for (int j = 0; j < 8; j++)
        {
            int ii = i * 8 + j;
            uint8_t v = (uint8_t)glm::clamp(linearToSRGB((float)ii), 0.0f, 255.0f);
            Log::printf("%d, ", v);
        }
        Log::printf("\n");
    }
    Log::printf("};\n");
}

int SDLCALL watch(void *userdata, SDL_Event* event)
{
    if (event->type == SDL_APP_WILLENTERBACKGROUND) {
        quitting = true;
    }

    return 1;
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS) != 0)
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    int width = 512;
    int height = 512;
    window = SDL_CreateWindow("sdl2stub", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);

    gl_context = SDL_GL_CreateContext(window);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
    {
        SDL_Log("Failed to SDL Renderer: %s", SDL_GetError());
		return -1;
	}

    SDL_AddEventWatch(watch, NULL);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        SDL_Log("Error: %s\n", glewGetErrorString(err));
    }

    Image img;
    if (!img.Load("texture/T_VideoCallThumbnailYellow.png"))
    {
        Log::printf("failed to load img\n");
    }

    processImage(img);

    img.Save("texture/T_VideoCallThumbnailYellow_YUV.png");

    Texture::Params texParams = {FilterType::LinearMipmapLinear, FilterType::Linear, WrapType::ClampToEdge, WrapType::ClampToEdge};
    static Texture* imgTexture = new Texture(img, texParams);

    // pre-multiplied alpha blending
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    Program* imgProgram = new Program();
    imgProgram->Load("shader/fullbright_texture_vert.glsl", "shader/fullbright_texture_frag.glsl");

    while (!quitting)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quitting = true;
            }
            if (event.type == SDL_KEYDOWN)
            {
                int sym = event.key.keysym.sym;
                //int mod = event.key.keysym.mod;
                // esc quits
                if (sym == SDLK_ESCAPE)
                {
                    quitting = true;
                }
            }
        }

        SDL_GL_MakeCurrent(window, gl_context);
        r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

        glClearColor(r, 0.4f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projMat = glm::ortho(0.0f, (float)width, 0.0f, (float)height, -10.0f, 10.0f);

        imgProgram->Apply();
        imgProgram->SetUniform("modelViewProjMat", projMat);
        imgProgram->SetUniform("color", glm::vec4(1.0f));

        // use texture unit 0 for colorTexture
        imgTexture->Apply(0);
        imgProgram->SetUniform("colorTexture", 0);

        glm::vec2 xyLowerLeft(0.0f, (height - width) / 2.0f);
        glm::vec2 xyUpperRight((float)width, (height + width) / 2.0f);
        glm::vec2 uvLowerLeft(0.0f, 0.0f);
        glm::vec2 uvUpperRight(1.0f, 1.0f);

        glm::vec3 positions[] = {glm::vec3(xyLowerLeft, 0.0f), glm::vec3(xyUpperRight.x, xyLowerLeft.y, 0.0f),
                                 glm::vec3(xyUpperRight, 0.0f), glm::vec3(xyLowerLeft.x, xyUpperRight.y, 0.0f)};
        imgProgram->SetAttrib("position", positions);

        glm::vec2 uvs[] = {uvLowerLeft, glm::vec2(uvUpperRight.x, uvLowerLeft.y),
                           uvUpperRight, glm::vec2(uvLowerLeft.x, uvUpperRight.y)};
        imgProgram->SetAttrib("uv", uvs);

        const size_t NUM_INDICES = 6;
        uint16_t indices[NUM_INDICES] = {0, 1, 2, 0, 2, 3};
        glDrawElements(GL_TRIANGLES, NUM_INDICES, GL_UNSIGNED_SHORT, indices);

        SDL_GL_SwapWindow(window);
    }

    SDL_DelEventWatch(watch, NULL);
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
