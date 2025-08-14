

#include <libosw/osw.h>
#include <stdio.h>
#include <stdlib.h>
#include "gl.h"
#include "../video_common.h"

#include "libosw/video.h"
#include "shaders/main.inc"


struct VideoData_t {
	f32 frame_w;
	f32 frame_h;
	f32 outdims_w;
	f32 outdims_h;
} video_data;

u32 prog_final;

u32 main_vao;
u32 main_vbo;
u32 video_data_ubo;

u32 tex_mainfb;



static u32 __osw_CreateSimpleGLProgram(const char *sh_src)
{

#ifdef OSW_DEBUG
	s32 status;
	GLsizei log_length = 0;
	GLchar message[1024];
#endif /* OSW_DEBUG */

	/* Compile Vertex Shader */
	u32 vsh_obj = glCreateShader(GL_VERTEX_SHADER);
	const char *vsh_source[2] = {"#version 410 core\n#define VERTEX_SHADER\n", sh_src};
	glShaderSource(vsh_obj, 2, vsh_source, NULL);
	glCompileShader(vsh_obj);

#ifdef OSW_DEBUG
	glGetShaderiv(vsh_obj, GL_COMPILE_STATUS, &status);
	if (!status) {
		glGetShaderInfoLog(vsh_obj, 1024, &log_length, message);
		printf("SHADER ERROR in Vertex Shader:\n%s\n", message);
		exit(0);
	}
#endif /* OSW_DEBUG */

	/* Compile Fragment Shader */
	u32 fsh_obj = glCreateShader(GL_FRAGMENT_SHADER);
	const char *fsh_source[2] = {"#version 410 core\n#define FRAGMENT_SHADER\n", sh_src};
	glShaderSource(fsh_obj, 2, fsh_source, NULL);
	glCompileShader(fsh_obj);

#ifdef OSW_DEBUG
	glGetShaderiv(fsh_obj, GL_COMPILE_STATUS, &status);
	if (!status) {
		glGetShaderInfoLog(fsh_obj, 1024, &log_length, message);
		printf("SHADER ERROR in Fragment Shader:\n%s\n", message);
		exit(0);
	}
#endif /* OSW_DEBUG */

	/* Link program */
	u32 program = glCreateProgram();
	glAttachShader(program, vsh_obj);
	glAttachShader(program, fsh_obj);
	glLinkProgram(program);

#ifdef OSW_DEBUG
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status) {
		glGetProgramInfoLog(program, 1024, &log_length, message);
		printf("%s\n", message);
		exit(0);
	}
#endif /* OSW_DEBUG */

	/* Delete shaders */
	glDetachShader(program, vsh_obj);
	glDetachShader(program, fsh_obj);
	glDeleteShader(vsh_obj);
	glDeleteShader(fsh_obj);

	return program;
}



void ogl_Init(void)
{
	/*Set some OpenGL settings*/
	glClearColor(0.0, 0.0, 0.0, 1.0);

	/* XXX: Compile shaders */

	/*Generate main VAO and VBO for sprites*/
	glGenVertexArrays(1, &main_vao);
	glBindVertexArray(main_vao);

	glGenBuffers(1, &main_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, main_vbo);
	glBufferData(GL_ARRAY_BUFFER, 64, NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &video_data_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, video_data_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(video_data), NULL, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribIPointer(0, 4, GL_UNSIGNED_INT, 64, (void*) 0);
	glEnable(GL_TEXTURE_2D);

#ifdef OSW_DEBUG
	printf("Compiling Main Program\n");
#endif
	prog_final = __osw_CreateSimpleGLProgram((const char*)main_glsl);
	/* Set tex units */
	glUseProgram(prog_final);
	glUniform1i(glGetUniformLocation(prog_final, "tex_main"), GL_TEXTURE0);

	/*Texture for main framebuffer*/
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &tex_mainfb);
	glBindTexture(GL_TEXTURE_2D, tex_mainfb);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexStorage2D( GL_TEXTURE_2D, 1, GL_RGBA8, OSW_VIDEO_MAX_WIDTH, OSW_VIDEO_MAX_HEIGHT);

	glBindVertexArray(0);
}


void ogl_Draw(const void *fb, u32 w, u32 h)
{
	vstate.output_w = MIN(OSW_VIDEO_MAX_WIDTH, w);
	vstate.output_h = MIN(OSW_VIDEO_MAX_WIDTH, h);
	__osw_UpdateOutputDims();
	video_data.frame_w = (f32) vstate.frame_w;
	video_data.frame_h = (f32) vstate.frame_h;
	video_data.outdims_w = (f32) vstate.output_w;
	video_data.outdims_h = (f32) vstate.output_h;

	/* Update the buffers */
	glBindVertexArray(main_vao);
	glBindBuffer(GL_UNIFORM_BUFFER, video_data_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(video_data), &video_data);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, video_data_ubo);

	/* Draw final image */
	glViewport(0, 0, vstate.frame_w, vstate.frame_h);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_BLEND);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(prog_final);

	/* Load texture */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_mainfb);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, fb);

	glViewport(vstate.frame_output_x, vstate.frame_output_y, vstate.frame_output_w, vstate.frame_output_h);

	u32 filter_mode = (vstate.frame_w > vstate.output_w * 2  || vstate.frame_h > vstate.output_h * 2 ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_mode);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
