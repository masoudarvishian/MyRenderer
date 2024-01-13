#include "triangle.h"

void vec2_swap(int* a, int* b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

void fill_flat_bottom_trinalge(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {

}

void fill_flat_top_trinalge(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {

}

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
	// we need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
	if (y0 > y1) {
		vec2_swap(&y0, &y1);
	}
	if (y1 > y2) {
		vec2_swap(&y1, &y2);
	}
	// double check
	if (y0 > y1) {
		vec2_swap(&y0, &y1);
	}

	// Calculate the new vertex (Mx, My) using triangle similarity
	int My = y1;
	int Mx = ((float)((x2 - x0) * (y1 - y0)) / (float)(y2 - y0)) + x0;

	// Draw flat-bottom triangle
	fill_flat_bottom_trinalge(x0, y0, x1, y1, Mx, My, color);

	// Draw flat-top triangle
	fill_flat_top_trinalge(x1, y1, Mx, My, x2, y2, color);
}

