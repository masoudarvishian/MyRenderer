#include "display.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
uint32_t* color_buffer = NULL;
int window_width = 800;
int window_height = 600;
SDL_Texture* color_buffer_texture = NULL;

bool initialize_window(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Failed to init SDL!\n");
		return false;
	}

	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);
	window_width = display_mode.w;
	window_height = display_mode.h;

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

	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

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

void draw_grid(void) {
	for (int y = 0; y < window_height; y++) {
		for (int x = 0; x < window_width; x++) {
			if (y % 10 == 0 || x % 10 == 0)
				color_buffer[(window_width * y) + x] = 0xFF222222;
		}
	}
	/*for (int y = 0; y < window_height; y+=10) {
		for (int x = 0; x < window_width; x+=10) {
			color_buffer[(window_width * y) + x] = 0xFF333333;
		}
	}*/
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
	for (int row = y; row < height; row++) {
		for (int col = x; col < width; col++) {
			color_buffer[(window_width * row) + col] = color;
		}
	}
}

void destroy_window(void) {
	free(color_buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void draw_pixel(int x, int y, uint32_t color) {
	if (x < window_width && y < window_height)
		color_buffer[window_width * y + x] = color;
}
