#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

extern uint32_t* color_buffer;
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern int window_width;
extern int window_height;
extern SDL_Texture* color_buffer_texture;

bool initialize_window(void);
void render_color_buffer(void);
void clear_color_buffer(uint32_t color);
void draw_grid(void);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_pixel(int x, int y, uint32_t color);
void destroy_window(void);

#endif
