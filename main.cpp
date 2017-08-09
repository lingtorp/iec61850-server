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
    struct nk_context* ctx;

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
          nk_layout_row_begin(ctx, NK_STATIC, 25, 2);
          nk_layout_row_push(ctx, 45);
          if (nk_menu_begin_label(ctx, "PROPERTIES", NK_TEXT_LEFT, nk_vec2(120, 200))) {
              nk_layout_row_dynamic(ctx, 30, 1);
              nk_menu_item_label(ctx, "REMOVE ALL", NK_TEXT_LEFT);
              nk_menu_item_label(ctx, "CLEAR ALL", NK_TEXT_LEFT);
              nk_menu_end(ctx);
          }
          if (nk_menu_begin_label(ctx, "SERVER", NK_TEXT_LEFT, nk_vec2(120, 200))) {
              nk_layout_row_dynamic(ctx, 30, 1);
              if (nk_menu_item_label(ctx, "START", NK_TEXT_LEFT)) {
                // server.start(interface);
              }
              if (nk_menu_item_label(ctx, "STOP", NK_TEXT_LEFT)) {
                // server.stop();
              }
              nk_menu_end(ctx);
          }
          nk_layout_row_end(ctx);
          nk_menubar_end(ctx);

          nk_layout_row_dynamic(ctx, 25, 2);
          nk_label(ctx, "SERVER:", NK_TEXT_LEFT);
          nk_label(ctx, "RUNNING", NK_TEXT_CENTERED);

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
      nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);}

      SDL_GL_SwapWindow(win);

      /* Sampled values server */
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
