#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "triangle.h"

#define FPS 60
#define FRAME_TARGET_TIME (1000/FPS)

extern uint32_t* color_buffer;
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern int window_width;
extern int window_height;
extern SDL_Texture* color_buffer_texture;

extern uint8_t wireframe;
extern uint8_t red_dot;
extern uint8_t filled_triangle;
extern uint8_t rendering_mode;
extern bool backface_culling;

bool initialize_window(void);
void render_color_buffer(void);
void clear_color_buffer(uint32_t color);
void draw_grid(void);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_pixel(int x, int y, uint32_t color);
void destroy_window(void);
void draw_triangle(triangle_t triangle, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);

#endif
