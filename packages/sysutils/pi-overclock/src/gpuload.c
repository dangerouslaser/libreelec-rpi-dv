// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026-present Team LibreELEC (https://libreelec.tv)
//
// GPU load for pi-overclock: renders a costly fragment shader off screen on
// the V3D render node (no display needed, so Kodi keeps running) and checks
// that every frame of the same input reads back identical. A wrong frame
// means the GPU computed something wrongly: exit 2. Runs until SIGTERM/SIGINT
// or for argv[1] seconds; prints one JSON line when it ends.

#define _GNU_SOURCE
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <fcntl.h>
#include <gbm.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define SIZE 1024
#define SEEDS 4

static volatile sig_atomic_t stop;

static void on_signal(int sig)
{
	(void)sig;
	stop = 1;
}

static const char *vs_src =
	"attribute vec2 pos;\n"
	"varying vec2 uv;\n"
	"void main() { uv = pos * 0.5 + 0.5; gl_Position = vec4(pos, 0.0, 1.0); }\n";

static const char *fs_src =
	"precision highp float;\n"
	"uniform float seed;\n"
	"varying vec2 uv;\n"
	"void main() {\n"
	"  vec2 p = uv * 4.0 + vec2(seed, seed * 0.37);\n"
	"  float a = 0.0;\n"
	"  for (int i = 0; i < 48; i++) {\n"
	"    p = vec2(sin(p.x * 1.31 + p.y), cos(p.y * 1.73 - p.x)) + 0.5 * p.yx;\n"
	"    a += p.x * p.y;\n"
	"  }\n"
	"  gl_FragColor = vec4(fract(a), fract(p * 3.0), 1.0);\n"
	"}\n";

static GLuint compile(GLenum type, const char *src)
{
	GLuint s = glCreateShader(type);
	GLint ok = 0;
	char log[1024];

	glShaderSource(s, 1, &src, NULL);
	glCompileShader(s);
	glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		glGetShaderInfoLog(s, sizeof(log), NULL, log);
		fprintf(stderr, "gpuload: shader: %s\n", log);
		exit(1);
	}
	return s;
}

static uint64_t fnv1a(const uint8_t *p, size_t n)
{
	uint64_t h = 0xcbf29ce484222325ull;

	while (n--) {
		h ^= *p++;
		h *= 0x100000001b3ull;
	}
	return h;
}

static double now(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec + ts.tv_nsec / 1e9;
}

static EGLDisplay open_display(int *fd_out)
{
	PFNEGLGETPLATFORMDISPLAYEXTPROC get_display =
		(PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");

	for (int i = 128; i < 136 && get_display; i++) {
		char path[64];
		int fd;
		struct gbm_device *gbm;
		EGLDisplay dpy;

		snprintf(path, sizeof(path), "/dev/dri/renderD%d", i);
		fd = open(path, O_RDWR | O_CLOEXEC);
		if (fd < 0)
			continue;
		gbm = gbm_create_device(fd);
		if (gbm) {
			dpy = get_display(EGL_PLATFORM_GBM_KHR, gbm, NULL);
			if (dpy != EGL_NO_DISPLAY && eglInitialize(dpy, NULL, NULL)) {
				*fd_out = fd;
				return dpy;
			}
			gbm_device_destroy(gbm);
		}
		close(fd);
	}
	return EGL_NO_DISPLAY;
}

int main(int argc, char **argv)
{
	double limit = argc > 1 ? atof(argv[1]) : 0;
	static const EGLint config_attrs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE
	};
	static const EGLint ctx_attrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	static const GLfloat quad[] = { -1, -1, 1, -1, -1, 1, 1, 1 };
	uint64_t reference[SEEDS] = { 0 };
	unsigned long frames = 0, mismatches = 0;
	EGLConfig config;
	EGLint n = 0;
	EGLContext ctx;
	GLuint prog, tex, fbo;
	GLint seed_loc;
	uint8_t *pixels;
	double start;
	int fd = -1;
	EGLDisplay dpy;
	const char *renderer;

	signal(SIGTERM, on_signal);
	signal(SIGINT, on_signal);

	dpy = open_display(&fd);
	if (dpy == EGL_NO_DISPLAY) {
		fprintf(stderr, "gpuload: no EGL display on a render node\n");
		return 1;
	}
	eglBindAPI(EGL_OPENGL_ES_API);
	if (!eglChooseConfig(dpy, config_attrs, &config, 1, &n) || n < 1) {
		fprintf(stderr, "gpuload: no EGL config\n");
		return 1;
	}
	ctx = eglCreateContext(dpy, config, EGL_NO_CONTEXT, ctx_attrs);
	if (ctx == EGL_NO_CONTEXT || !eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
		fprintf(stderr, "gpuload: no surfaceless GLES context\n");
		return 1;
	}
	renderer = (const char *)glGetString(GL_RENDERER);

	prog = glCreateProgram();
	glAttachShader(prog, compile(GL_VERTEX_SHADER, vs_src));
	glAttachShader(prog, compile(GL_FRAGMENT_SHADER, fs_src));
	glBindAttribLocation(prog, 0, "pos");
	glLinkProgram(prog);
	glUseProgram(prog);
	seed_loc = glGetUniformLocation(prog, "seed");

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SIZE, SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "gpuload: framebuffer incomplete\n");
		return 1;
	}
	glViewport(0, 0, SIZE, SIZE);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, quad);
	glEnableVertexAttribArray(0);

	pixels = malloc(SIZE * SIZE * 4);
	if (!pixels)
		return 1;

	start = now();
	while (!stop && (limit <= 0 || now() - start < limit)) {
		int s = frames % SEEDS;
		uint64_t h;

		glUniform1f(seed_loc, (float)s * 1.25f);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glReadPixels(0, 0, SIZE, SIZE, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		if (glGetError() != GL_NO_ERROR) {
			fprintf(stderr, "gpuload: GL error\n");
			return 1;
		}
		h = fnv1a(pixels, SIZE * SIZE * 4);
		if (frames < SEEDS)
			reference[s] = h;
		else if (h != reference[s])
			mismatches++;
		frames++;
	}

	printf("{\"renderer\": \"%s\", \"frames\": %lu, \"mismatches\": %lu, \"seconds\": %.1f}\n",
	       renderer ? renderer : "", frames, mismatches, now() - start);
	fflush(stdout);
	return mismatches ? 2 : 0;
}
