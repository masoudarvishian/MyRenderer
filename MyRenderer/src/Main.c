#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"
#include "matrix.h"
#include <math.h>
#include "light.h"
#include "upng.h"

#define MAX_TRIANGLES_PER_MESH 10000
triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
int num_triangles_to_render = 0;

vec3_t camera_position = { 0, 0, 0 };

mat4_t proj_matrix;
bool is_running = false;
int previous_frame_time;

void setup(void) {
	rendering_mode = render_texture;

	color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);
	z_buffer = (float*)malloc(sizeof(float) * window_width * window_height);

	color_buffer_texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGBA32,
		SDL_TEXTUREACCESS_STREAMING,
		window_width,
		window_height
	);

	float fov = M_PI / 3.0; // equal to 60 degrees
	float aspect = (float)window_height / (float)window_width;
	float znear = 0.1;
	float zfar = 100.0;
	proj_matrix = mat4_make_perspective(fov, aspect, znear, zfar);

	//load_cube_mesh_data();
	load_obj_file_data("./assets/drone.obj");

	load_png_texture_data("./assets/drone.png");
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
			else if (event.key.keysym.sym == SDLK_5)
				rendering_mode = render_texture;
			else if (event.key.keysym.sym == SDLK_6)
				rendering_mode = render_texture | wireframe;
		}
	}
}

void update(void) {

	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
		SDL_Delay(time_to_wait);

	previous_frame_time = SDL_GetTicks();

	// initialize the counter of triangles to render for the current frame
	num_triangles_to_render = 0;

	mesh.rotation.x += 0.02;
	mesh.rotation.y += 0.02;
	mesh.rotation.z += 0.02;
	mesh.translation.z = 5.0;

	mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
	mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
	mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
	mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
	mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

	// Loop all triangle faces of our mesh
	int num_faces = array_length(mesh.faces);
	for (int i = 0; i < num_faces; i++) {
		face_t mesh_face = mesh.faces[i];

		vec3_t face_vertices[3];
		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		vec4_t transformed_vertices[3];

		// Loop all three vertices of this current face and apply transformations
		for (int j = 0; j < 3; j++) {
			vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

			// create a world matrix, combining scaling, rotation and translation
			mat4_t world_matrix = mat4_identity();

			// multiply all metrices and laod the world matrix
			world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
			world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

			// multiply the world matrix by the original vector
			transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);
			
			transformed_vertices[j] = transformed_vertex;
		}

		vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]); /*    A    */
		vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]); /*   / \   */
		vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]); /*  B---C  */

		vec3_t vector_ab = vec3_sub(vector_b, vector_a);
		vec3_t vector_ac = vec3_sub(vector_c, vector_a);
		vec3_normalize(&vector_ab);
		vec3_normalize(&vector_ac);

		vec3_t normal = vec3_cross(vector_ab, vector_ac);
		vec3_normalize(&normal);

		vec3_t camera_ray = vec3_sub(camera_position, vector_a);
		vec3_normalize(&camera_ray);

		float dot_normal_camera = vec3_dot(normal, camera_ray);

		// check backface culling
		if (backface_culling) {
			if (dot_normal_camera < 0.0)
				continue;
		}

		vec4_t projected_points[3];
		for (int j = 0; j < 3; j++) {
			// Project the current vertex
			projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);

			// scale
			projected_points[j].x *= (window_width / 2.0);
			projected_points[j].y *= (window_height / 2.0);

			// invert y axis
			projected_points[j].y *= -1;

			// translate the projected points to the middle of the screen
			projected_points[j].x += (window_width / 2.0);
			projected_points[j].y += (window_height / 2.0);
		}

		float light_intensity_factor = vec3_dot(normal, light.direction) * -1;
		uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);

		triangle_t projected_triangle = {
			.points = {
				{ projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w },
				{ projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w },
				{ projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w }
			},
			.texcoords = {
				{ mesh_face.a_uv.u, mesh_face.a_uv.v },
				{ mesh_face.b_uv.u, mesh_face.b_uv.v },
				{ mesh_face.c_uv.u, mesh_face.c_uv.v }
			},
			.color = triangle_color
		};

		// Save the projected triangle in the array of triangles to render
		if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH) {
			triangles_to_render[num_triangles_to_render] = projected_triangle;
			num_triangles_to_render++;
		}
	}
}

void render(void) {
	draw_grid();
	
	// Loop all projected triangles and render them
	for (int i = 0; i < num_triangles_to_render; i++) {
		triangle_t triangle = triangles_to_render[i];

		if ((rendering_mode & red_dot) == red_dot) {
			draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000);
			draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000);
			draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000);
		}
	   
		if ((rendering_mode & filled_triangle) == filled_triangle) {
			draw_filled_triangle(
				triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w,
				triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w,
				triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w,
				triangle.color
			);
		}

		if ((rendering_mode & render_texture) == render_texture) {
			draw_textured_triangle(
				triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v, // vertex A
				triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v, // vertex B
				triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v, // vertex C
				mesh_texture
			);
		}
		
		if ((rendering_mode & wireframe) == wireframe) {
			draw_triangle(
				triangle.points[0].x, triangle.points[0].y, // vertex A
				triangle.points[1].x, triangle.points[1].y, // vertex B
				triangle.points[2].x, triangle.points[2].y, // vertex C
				0xFFFFFFFF
			);
		}
	}

	render_color_buffer();
	clear_color_buffer(0xFF000000);
	clear_z_buffer();
	SDL_RenderPresent(renderer);
}

void free_resources(void) {
	array_free(mesh.vertices);
	array_free(mesh.faces);
	free(color_buffer);
	free(z_buffer);
	upng_free(png_texture);
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