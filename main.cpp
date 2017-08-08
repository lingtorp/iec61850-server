/*
 *
 * Example program for Sampled Values (SV) subscriber
 *
 */
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include "../../nuklear.h"
#include "nuklear_sdl_gl3.h"

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 600

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

#define APP_NAME "IEC61850-Server"

#include <signal.h>
#include <stdio.h>
#include "hal_thread.h"
#include "publisher.hpp"

static bool running = true;

void sigint_handler(int signalId) {
    running = 0;
}

int main(int argc, char** argv) {
    /* Platform */
    SDL_Window* win;
    struct nk_color background;
    int win_width, win_height;

    /* GUI */
    struct nk_context *ctx;

    /* SDL setup */
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    win = SDL_CreateWindow(APP_NAME,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_GL_CreateContext(win);
    SDL_GetWindowSize(win, &win_width, &win_height);

    /* OpenGL setup */
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glewExperimental = 1;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to setup GLEW\n");
        exit(1);
    }

    ctx = nk_sdl_init(win);

    /* Load Fonts: loads the default font */
    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();

    /* Sampled values server setup*/
    std::string interface = "ens33";
    printf("Using interface %s\n", interface.c_str());

    Publisher publisher{interface};

    Channel* channel1 = publisher.add_channel("svpub1");

    Value val1 = channel1->create_float_value();
    Value val2 = channel1->create_float_value();

    publisher.setup_complete();

    /*
    SampledValuesPublisher svPublisher = SampledValuesPublisher_create(NULL, interface);

    SV_ASDU asdu1 = SampledValuesPublisher_addASDU(svPublisher, "svpub1", NULL, 1);

    int float1 = SV_ASDU_addFLOAT(asdu1);
    int float2 = SV_ASDU_addFLOAT(asdu1);

    SV_ASDU asdu2 = SampledValuesPublisher_addASDU(svPublisher, "svpub2", NULL, 1);

    int float3 = SV_ASDU_addFLOAT(asdu2);
    int float4 = SV_ASDU_addFLOAT(asdu2);

    SampledValuesPublisher_setupComplete(svPublisher);
    */

    float fVal1 = 1234.5678f;
    float fVal2 = 0.1234f;

    while(running) {
      /* Input */
      SDL_Event evt;
      nk_input_begin(ctx);
      while (SDL_PollEvent(&evt)) {
          if (evt.type == SDL_QUIT) goto cleanup;
          nk_sdl_handle_event(&evt);
      }
      nk_input_end(ctx);

      /* GUI */
      if (nk_begin(ctx, APP_NAME, nk_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT),
          NK_WINDOW_BORDER|NK_WINDOW_TITLE)) {
          nk_menubar_begin(ctx);
          nk_layout_row_begin(ctx, NK_STATIC, 25, 1);
          nk_layout_row_push(ctx, 45);
          if (nk_menu_begin_label(ctx, "PROPERTIES", NK_TEXT_LEFT, nk_vec2(120, 200))) {
              nk_layout_row_dynamic(ctx, 30, 1);
              nk_menu_item_label(ctx, "REMOVE ALL", NK_TEXT_LEFT);
              nk_menu_item_label(ctx, "CLEAR ALL", NK_TEXT_LEFT);
              nk_menu_end(ctx);
          }
          nk_layout_row_end(ctx);
          nk_menubar_end(ctx);

          /* Property pane */
          nk_layout_row_dynamic(ctx, 25, 2);
          nk_label(ctx, "Property #1", NK_TEXT_LEFT);
          nk_button_label(ctx, "Remove");

          nk_layout_row_dynamic(ctx, 25, 2);
          nk_label(ctx, "Name: ", NK_TEXT_LEFT);
          static char text[32];
          static int text_len = 0;
          nk_edit_string(ctx, NK_EDIT_SIMPLE, text, &text_len, sizeof(text), nk_filter_default);

          nk_layout_row_dynamic(ctx, 25, 1);
          static float value = 0.0f;
          nk_property_float(ctx, "Value:", 0.0f, &value, 100.0f, 0.1f, 1.0f);

          nk_layout_row_dynamic(ctx, 50, 1);
          nk_button_label(ctx, "+");
      }
      nk_end(ctx);

      /* Draw */
      {float bg[4];
      nk_color_fv(bg, background);
      SDL_GetWindowSize(win, &win_width, &win_height);
      glViewport(0, 0, win_width, win_height);
      glClear(GL_COLOR_BUFFER_BIT);
      glClearColor(bg[0], bg[1], bg[2], bg[3]);
      /* IMPORTANT: `nk_sdl_render` modifies some global OpenGL state
       * with blending, scissor, face culling, depth test and viewport and
       * defaults everything back into a default state.
       * Make sure to either a.) save and restore or b.) reset your own state after
       * rendering the UI. */
      nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);}

      SDL_GL_SwapWindow(win);

      /* Sampled values server */
      /*
      SV_ASDU_setFLOAT(asdu1, float1, fVal1);
      SV_ASDU_setFLOAT(asdu1, float2, fVal2);

      SV_ASDU_increaseSmpCnt(asdu1);
      SV_ASDU_increaseSmpCnt(asdu2);

      fVal1 += 1.1f;
      fVal2 += 0.1f;

      SampledValuesPublisher_publish(svPublisher);
      */
      channel1->set_value(val1, fVal1);
      channel1->set_value(val2, fVal2);

      fVal1 += 1.1f;
      fVal2 += 0.1f;

      publisher.broadcast();

      Thread_sleep(50);
    }
cleanup:
    return 0;
}
