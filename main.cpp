/*
 * Example program for Sampled Values (SV) subscriber
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

#include <cstdio>
#include <cstring>
#include "hal_thread.h"
#include "publisher.hpp"

static bool running = true;

/** Changes the style of the nk_button into a greyed on, returns the old style */
nk_style_button greyed_out_button(nk_context *ctx) {
  struct nk_style_button button;
  button = ctx->style.button;
  ctx->style.button.normal = nk_style_item_color(nk_rgb(40,40,40));
  ctx->style.button.hover = nk_style_item_color(nk_rgb(40,40,40));
  ctx->style.button.active = nk_style_item_color(nk_rgb(40,40,40));
  ctx->style.button.border_color = nk_rgb(60,60,60);
  ctx->style.button.text_background = nk_rgb(60,60,60);
  ctx->style.button.text_normal = nk_rgb(60,60,60);
  ctx->style.button.text_hover = nk_rgb(60,60,60);
  ctx->style.button.text_active = nk_rgb(60,60,60);
  return button;
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
    /* Default channel and values */
    Channel* channel1 = publisher.add_channel("longnamechanneltotesttheguiandotherstuffaswell");
    channel1->create_float_value();
    channel1->create_float_value();

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
          if (nk_menu_begin_label(ctx, "CHANNELS", NK_TEXT_LEFT, nk_vec2(120, 200))) {
              nk_layout_row_dynamic(ctx, 30, 1);
              if (nk_menu_item_label(ctx, "REMOVE ALL", NK_TEXT_LEFT)) {
                publisher.channels.clear();
              }
              if (nk_menu_item_label(ctx, "CLEAR ALL", NK_TEXT_LEFT)) {
                // TODO:
              }
              nk_menu_end(ctx);
          }
          if (nk_menu_begin_label(ctx, "SERVER", NK_TEXT_LEFT, nk_vec2(120, 200))) {
              nk_layout_row_dynamic(ctx, 30, 1);
              if (nk_menu_item_label(ctx, "START", NK_TEXT_LEFT)) {
                if (!publisher.setup_completed) {
                  publisher.complete_setup();
                }
                publisher.running = true;
              }
              if (nk_menu_item_label(ctx, "STOP", NK_TEXT_LEFT)) {
                /* FIXME: Resets complete state upon stopping the broadcast */
                publisher = Publisher{interface};
              }
              nk_menu_end(ctx);
          }
          nk_layout_row_end(ctx);
          nk_menubar_end(ctx);

          nk_layout_row_dynamic(ctx, 35, 1);
          if(nk_group_begin(ctx, "Status", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row_dynamic(ctx, 25, 2);
            nk_label(ctx, "SERVER:", NK_TEXT_LEFT);
            if (publisher.running) {
              nk_label_colored(ctx, "RUNNING", NK_TEXT_CENTERED, nk_rgb(0, 255, 0));
            } else {
              nk_label_colored(ctx, "STOPPED", NK_TEXT_CENTERED, nk_rgb(255, 0, 0));
            }
            nk_group_end(ctx);
          }
          /* Padding */
          nk_layout_row_dynamic(ctx, 10, 2);
          nk_label(ctx, "", NK_TEXT_LEFT);

          /* Channel panes */
          static float values[Publisher::MAX_NUM_CHANNELS][Channel::MAX_NUM_VALUES];
          for (size_t i = 0; i < publisher.channels.size(); i++) {
            Channel &channel = publisher.channels[i];

            std::string chan_name{"CHANNEL #" + std::to_string(i + 1)};
            nk_layout_row_dynamic(ctx, 25, 2);
            nk_label(ctx, chan_name.c_str(), NK_TEXT_LEFT);

            /* Only able to remove the last value */
            if (i == publisher.channels.size() - 1) {
              /* Greyed out buttons during broadcasting */
              if (!publisher.running) {
                if (nk_button_label(ctx, "Remove channel")) {
                  publisher.channels.pop_back();
                  /* If we removed the last channel we are done here */
                  if (publisher.channels.size() == 0) {
                    continue;
                  }
                }
              } else {
                nk_style_button button = greyed_out_button(ctx);
                nk_button_label(ctx, "Remove channel");
                ctx->style.button = button;
              }
            }

            nk_layout_row_dynamic(ctx, 25, 2);
            nk_label(ctx, "Name: ", NK_TEXT_RIGHT);
            nk_label(ctx, channel.name.c_str(), NK_TEXT_LEFT);

            for (size_t j = 0; j < channel.values.size(); j++) {
              nk_layout_row_dynamic(ctx, 25, 2);
              /* FIXME: Only working with float values for now */
              nk_property_float(ctx, "Value:", 0.0f, &values[i][j], 100.0f, 0.1f, 1.0f);
              /* Only enable data to be set when setup is completed */
              if (publisher.setup_completed) {
                channel.set_value(channel.values[j], values[i][j]);
              }
              /* Only able to remove the last value */
              if (j == channel.values.size() - 1) {
                /* Greyed out buttons during broadcasting */
                if (!publisher.running) {
                  if (nk_button_label(ctx, "Remove value")) {
                    channel.values.pop_back();
                  }
                } else {
                  nk_style_button button = greyed_out_button(ctx);
                  nk_button_label(ctx, "Remove value");
                  ctx->style.button = button;
                }
              }
            }

            /* Padding */
            nk_layout_row_dynamic(ctx, 5, 2);
            nk_label(ctx, "", NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 25, 2);
            nk_label(ctx, "", NK_TEXT_LEFT);
            /* Greyed out buttons during broadcasting */
            if (!publisher.running) {
              if (nk_button_label(ctx, "New value")) {
                /* FIXME: Only working with float values for now */
                channel.create_float_value();
              }
            } else {
              nk_style_button button = greyed_out_button(ctx);
              nk_button_label(ctx, "New value");
              ctx->style.button = button;
            }
            /* Padding */
            nk_layout_row_dynamic(ctx, 5, 2);
            nk_label(ctx, "", NK_TEXT_LEFT);
          }

          nk_layout_row_dynamic(ctx, 30, 1);
          nk_label(ctx, "", NK_TEXT_LEFT);

          nk_layout_row_dynamic(ctx, 25, 2);
          nk_label(ctx, "Name:", NK_TEXT_RIGHT);
          static char chan_name[64];
          static int chan_name_lng = 0;
          nk_edit_string(ctx, NK_EDIT_FIELD, chan_name, &chan_name_lng, 64, nk_filter_default);
          nk_layout_row_dynamic(ctx, 25, 1);
          /* Greyed out buttons during broadcasting */
          if (!publisher.running) {
            if (nk_button_label(ctx, "New channel")) {
              publisher.add_channel(std::string(chan_name));
              chan_name_lng = 0; // Reset string in edit field
              std::memset(chan_name, '\0', sizeof(chan_name));
            }
          } else {
            nk_style_button button = greyed_out_button(ctx);
            nk_button_label(ctx, "New channel");
            ctx->style.button = button;
          }
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
      publisher.broadcast();
      Thread_sleep(50);
    }
cleanup:
    return 0;
}
