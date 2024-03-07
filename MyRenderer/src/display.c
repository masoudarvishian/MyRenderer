#include "display.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
uint32_t* color_buffer = NULL;
float* z_buffer = NULL;
int window_width = 800;
int window_height = 600;
SDL_Texture* color_buffer_texture = NULL;

uint8_t wireframe = 0x1;
uint8_t red_dot = 0x2;
uint8_t filled_triangle = 0x4;
uint8_t render_texture = 0x8;
uint8_t rendering_mode;
bool backface_culling = true;

bool initialize_window(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Failed to init SDL!\n");
		return false;
	}

	/*SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);
	window_width = display_mode.w;
	window_height = display_mode.h;*/

	window = SDL_CreateWindow(
		NULL,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		window_width,
		window_height,
		SDL_WINDOW_BORDERLESS
	);
	if (!window) {
		fprintf(stderr, "Failed to create window!\n");
		return false;
	}
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		fprintf(stderr, "Failed to create renderer!\n");
		return false;
	}

	//SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

	return true;
}

void render_color_buffer(void) {
	SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer, (int)(window_width * sizeof(uint32_t)));
	SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void clear_color_buffer(uint32_t color) {
	for (int y = 0; y < window_height; y++) {
		for (int x = 0; x < window_width; x++) {
			color_buffer[(window_width * y) + x] = color;
		}
	}
}

void clear_z_buffer(void) {
	for (int y = 0; y < window_height; y++) {
		for (int x = 0; x < window_width; x++) {
			z_buffer[(window_width * y) + x] = 1.0;
		}
	}
}

void draw_grid(void) {
	for (int y = 0; y < window_height; y++) {
		for (int x = 0; x < window_width; x++) {
			if (y % 10 == 0 || x % 10 == 0)
				color_buffer[(window_width * y) + x] = 0xFF222222;
		}
	}
}

void draw_pixel(int x, int y, uint32_t color) {
	if (x >= 0 && x < window_width && y >= 0 && y < window_height)
		color_buffer[window_width * y + x] = color;
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
	for (int row = y; row < y + height; row++) {
		for (int col = x; col < x + width; col++) {
			draw_pixel(col, row, color);
		}
	}
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
	int delta_x = x1 - x0;
	int delta_y = y1 - y0;

	int longest_side_length = abs(delta_x) >= abs(delta_y) ? abs(delta_x) : abs(delta_y);

	float x_inc = delta_x / (float)longest_side_length;
	float y_inc = delta_y / (float)longest_side_length;

	float current_x = x0;
	float current_y = y0;

	for (int i = 0; i <= longest_side_length; i++) {
		draw_pixel(round(current_x), round(current_y), color);
		current_x += x_inc;
		current_y += y_inc;
	}
}

void destroy_window(void) {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

