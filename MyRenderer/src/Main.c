#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"

triangle_t* triangles_to_render = NULL;

vec3_t camera_position = { 0, 0, 0 };

float fov_factor = 640;
bool is_running = false;
int previous_frame_time;

void setup(void) {
	rendering_mode = wireframe | red_dot;
	color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);
	color_buffer_texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		window_width,
		window_height
	);
	load_obj_file_data("./assets/cube.obj");
}

void process_input(void) {
	SDL_Event event;
	SDL_PollEvent(&event);
	switch (event.type) {
	case SDL_QUIT:
		is_running = false;
		break;
	case SDL_KEYDOWN: {
			if (event.key.keysym.sym == SDLK_ESCAPE)
				is_running = false;

			if (event.key.keysym.sym == SDLK_c)
				backface_culling = true;

			if (event.key.keysym.sym == SDLK_d)
				backface_culling = false;
			
			// change rendering mode
			if (event.key.keysym.sym == SDLK_1)
				rendering_mode = wireframe | red_dot;
			else if (event.key.keysym.sym == SDLK_2)
				rendering_mode = wireframe;
			else if (event.key.keysym.sym == SDLK_3)
				rendering_mode = filled_triangle;
			else if (event.key.keysym.sym == SDLK_4)
				rendering_mode = filled_triangle | wireframe;
		}
	}
}

vec2_t project(vec3_t point) {
	vec2_t projected_point = {
		.x = (fov_factor * point.x) / point.z,
		.y = (fov_factor * point.y) / point.z
	};
	return projected_point;
}

void update(void) {

	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
		SDL_Delay(time_to_wait);

	previous_frame_time = SDL_GetTicks();

	triangles_to_render = NULL;

	mesh.rotation.x += 0.01;
	mesh.rotation.y += 0.01;
	mesh.rotation.z += 0.01;

	// Loop all triangle faces of our mesh
	int num_faces = array_length(mesh.faces);
	for (int i = 0; i < num_faces; i++) {
		face_t mesh_face = mesh.faces[i];

		vec3_t face_vertices[3];
		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		vec3_t transformed_vertices[3];

		// Loop all three vertices of this current face and apply transformations
		for (int j = 0; j < 3; j++) {
			vec3_t transformed_vertex = face_vertices[j];

			transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
			transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
			transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

			// Translate the vertex away from the camera
			transformed_vertex.z += 5;
			transformed_vertices[j] = transformed_vertex;
		}

		// check backface culling
		if (backface_culling) {
			vec3_t vector_a = transformed_vertices[0]; /*    A    */
			vec3_t vector_b = transformed_vertices[1]; /*   / \   */
			vec3_t vector_c = transformed_vertices[2]; /*  B---C  */

			vec3_t vector_ab = vec3_sub(vector_b, vector_a);
			vec3_t vector_ac = vec3_sub(vector_c, vector_a);
			vec3_normalize(&vector_ab);
			vec3_normalize(&vector_ac);

			vec3_t normal = vec3_cross(vector_ab, vector_ac);
			vec3_normalize(&normal);

			vec3_t camera_ray = vec3_sub(camera_position, vector_a);

			float dot_normal_camera = vec3_dot(normal, camera_ray);

			if (dot_normal_camera < 0.0)
				continue;
		}

		triangle_t projected_triangle;
		for (int j = 0; j < 3; j++) {
			// Project the current vertex
			vec2_t projected_point = project(transformed_vertices[j]);

			// Scale and translate the projected points to the middle of the screen
			projected_point.x += (window_width / 2);
			projected_point.y += (window_height / 2);

			projected_triangle.points[j] = projected_point;
		}
		// Save the projected triangle in the array of triangles to render
		array_push(triangles_to_render, projected_triangle);
	}
}

void render(void) {
	draw_grid();
	
	// Loop all projected triangles and render them
	int num_triangles = array_length(triangles_to_render);
	for (int i = 0; i < num_triangles; i++) {
		triangle_t triangle = triangles_to_render[i];

		if ((rendering_mode & red_dot) == red_dot) {
			draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000);
			draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000);
			draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000);
		}
	   
		if ((rendering_mode & filled_triangle) == filled_triangle) {
			draw_filled_triangle(
				triangle.points[0].x, triangle.points[0].y,
				triangle.points[1].x, triangle.points[1].y,
				triangle.points[2].x, triangle.points[2].y,
				0xFFFFFFFF
			);
		}
		
		if ((rendering_mode & wireframe) == wireframe) {
			draw_triangle(triangle, 0xFF00FF00);
		}
	}

	array_free(triangles_to_render);
	render_color_buffer();
	clear_color_buffer(0xFF000000);
	SDL_RenderPresent(renderer);
}

void free_resources(void) {
	array_free(mesh.vertices);
	array_free(mesh.faces);
	free(color_buffer);
}

int main(int argc, char* args[]) {
	is_running = initialize_window();
	setup();
	while (is_running) {
		process_input();
		update();
		render();
	}
	destroy_window();
	free_resources();
	return 0;
}