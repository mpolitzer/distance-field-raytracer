#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>

#define TZ_GFX_WINDOW_INLINE 1
#include "tz/tz/gfx/window.h"

#define TZ_GFX_SHADER_INLINE 1
#include "tz/tz/gfx/shader.h"

#define TZ_GFX_CAM3_INLINE 1
#include "tz/tz/gfx/cam3.h"

/* -------------------------------------------------------------------------- */

#define RELPATH "dist/share/shaders/"
#define VS RELPATH "main.vs"
#define FS RELPATH "main.fs"

#define N 128
float _d[N][N][N];

struct app_info {
	tz_window win;
	tz_shader s;

	// distance field
	GLuint vao, tex;

	tz_mat4 proj, i_pvm;
	tz_cam3 cam;
};
struct app_info _app;

static GLuint create_df_texture(int w, int h, int d, float *p);
static GLuint create_df_canvas();
static void   mk_sphere(int w, int h, int d, float *p);
static void   build_shaders(tz_shader *s);
static void   print_help(const char *app_name);
static void   abort_if(bool cond, const char *fmt, ...);

/* -------------------------------------------------------------------------- */

#define key(ev, k) ((ev).key.keysym.sym == (k) && (ev).key.repeat == 0)
int main(int argc, const char *argv[])
{
	abort_if(SDL_Init(SDL_INIT_EVERYTHING) != 0,
			"SDL:%s\n", SDL_GetError());
	abort_if(tz_window_create(&_app.win, 800, 800, false) == false,
			"SDL:%s\n", SDL_GetError());

	mk_sphere(N, N, N, (float *)(void *)_d);
	_app.vao = create_df_canvas();
	_app.tex = create_df_texture(N, N, N, (float *)(void *)_d);

	_app.cam.rho   = 3;
	_app.cam.phi   = M_PI/2;
	_app.cam.theta = 0;
	tz_mat4_perspective(&_app.proj, 45, 800/(float)800, 0.1, 100);
	tz_mat4_mkidentity(&_app.proj);

	build_shaders(&_app.s);
	print_help(argv[0]);
	while (1) {
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_QUIT: {
				goto cleanup;
			}
			case SDL_KEYDOWN: {
				if (key(ev, SDLK_r)) { build_shaders(&_app.s); }
				if (key(ev, SDLK_h)) { print_help(argv[0]); }

				if (key(ev, SDLK_d)) { _app.cam.x -= .1; }
				if (key(ev, SDLK_a)) { _app.cam.x += .1; }
				if (key(ev, SDLK_s)) { _app.cam.z -= .1; }
				if (key(ev, SDLK_w)) { _app.cam.z += .1; }
				if (key(ev, SDLK_v)) { _app.cam.y -= .1; }
				if (key(ev, SDLK_f)) { _app.cam.y += .1; }
				break;
			}
			case SDL_MOUSEMOTION: {
				int x, y;
				if (SDL_GetMouseState(&x, &y)) {
					_app.cam.theta += M_PI*ev.motion.xrel/1280.0;
					_app.cam.phi   -= M_PI*ev.motion.yrel/720.0;
				}
				break;
			}
			case SDL_MOUSEWHEEL: {
				float inc = SDL_GetModState() ? 0.1 : 1;
				_app.cam.rho += inc*ev.wheel.y;
				printf("slice=%f\n", _app.cam.rho);
				break;
			}}
		}

		tz_mat4 modelview, pvm, i_pvm;
		tz_cam3_mkview(&_app.cam, &modelview);
		tz_mat4_mul(&pvm, &_app.proj, &modelview);
		tz_mat4_inverse(&i_pvm, &pvm);
		tz_mat4_transpose(&pvm, &pvm);

		glUniformMatrix4fv(0, 1, GL_TRUE, pvm.f);
		glUniformMatrix4fv(1, 1, GL_TRUE, i_pvm.f);

		glUniform1f(9, _app.cam.rho);

		glBindVertexArray(_app.vao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		tz_window_flip(&_app.win);
		SDL_Delay(1); // don't hot the CPU
	}

cleanup:
	return EXIT_SUCCESS;
}

static GLuint create_df_texture(int w, int h, int d, float *p)
{
	GLuint id;
	glGenTextures(1, &id);

	glActiveTexture(GL_TEXTURE0+0);
	glBindTexture(GL_TEXTURE_3D, id);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexSubImage3D(GL_TEXTURE_3D,
	//		0, 0, 0, 0,
	//		w, h, d, GL_RED,
	//		GL_FLOAT, p);
	glTexImage3D(GL_TEXTURE_3D, 0,
			GL_RED,
			w, h, d, 0,
			GL_RED,
			GL_FLOAT,
			p);
	return id;
}

static GLuint create_df_canvas()
{
	GLuint vao, vbo;
	const GLfloat strip[] = {
		-1, -1,
		-1,  1,
		 1, -1,
		 1,  1,
	};
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof strip, strip,  GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	return vao;
}

static void mk_sphere(int w, int h, int d, float *p)
{
	for (int k=0; k<d; ++k) {
		for (int j=0; j<h; ++j) {
			for (int i=0; i<w; ++i) {
				int index = k*N*N + j*N + i;
				float x = i/(float)N - .5,
				      y = j/(float)N - .5,
				      z = k/(float)N - .5;
				float r=.2f;
				p[index] = sqrt(x*x + y*y + z*z)-r;
			}
		}
	}
}

void print_sphere()
{
	for (int k=0; k<N; ++k) {
		for (int j=0; j<N; ++j) {
			for (int i=0; i<N; ++i) {
				if (_d[k][j][i] > .2) { putchar('#'); continue; }
				if (_d[k][j][i] > .1) { putchar('*'); continue; }
				if (_d[k][j][i] >  0) { putchar('.'); continue; }
				putchar(' ');
			}
			putchar('\n');
		}
	}
}

static void abort_if(bool cond, const char *fmt, ...)
{
	if (cond) {
		va_list ap;
		va_start(ap, fmt);
		vfprintf(stdout, fmt, ap);
		abort();
	}
}

static GLchar *readfile(const char *name);
static void build_shaders(tz_shader *s) {
	GLchar *file_contents;

	tz_shader_destroy(s);
	tz_shader_init(s);
	tz_shader_new(s);
	if ((file_contents = readfile(VS))) {
		tz_shader_compile(s, file_contents, GL_VERTEX_SHADER);
		free(file_contents);
	}
	else SDL_assert("shader " VS " not found");

	if ((file_contents = readfile(FS))) {
		tz_shader_compile(s, file_contents, GL_FRAGMENT_SHADER);
		free(file_contents);
	}
	else SDL_assert("shader " FS " not found");

	tz_shader_link(s);
	tz_shader_bind(s);
	printf("loaded:\n\t" VS "\n\t" FS "\n");
}


static GLchar *readfile(const char *name)
{
	char *data;
	size_t len;
	SDL_RWops *rw = SDL_RWFromFile(name, "rb");
	SDL_assert(rw && "failed to read file.");

	len = SDL_RWseek(rw, 0, SEEK_END);
	data = malloc(len+1);
	data[len] = '\0';

	SDL_RWseek(rw, 0, SEEK_SET);
	SDL_RWread(rw, data, len, 1);
	SDL_RWclose(rw);
	return data;
}

static void print_help(const char *app_name)
{
	printf(
			"%s usage: (With app running):\n"
			"\th - help (print this text)\n"
			"\tr - reload shaders\n"
			,
			app_name
	      );
}
