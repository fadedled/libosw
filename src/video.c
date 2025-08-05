
#include <libosw/osw.h>
#include "video_common.h"
#include "opengl/gfx_ogl.h"
#include <stddef.h>

VideoState vstate;



void OSW_VideoSetFilter(ViFilter filter)
{
	if (filter < OSW_VIDEO_MAX_FILTER) {
		vstate.filter_mode = filter;
	}
}


void OSW_VideoSetSize(u32 w, u32 h)
{
	vstate.frame_w = w;
	vstate.frame_h = h;
	__osw_VideoAttrSet(OSW_VIDEO_ATTR_FRAME, NULL);
}


void OSW_VideoTitleSet(const char* title)
{
	__osw_VideoAttrSet(OSW_VIDEO_ATTR_TITLE, (void*) title);
}


void __osw_UpdateOutputDims(void)
{
	u32 out_w = 0, out_h = 0;
	f32 aspect = (f32) vstate.output_w / (f32) vstate.output_h;
	f32 frame_aspect = (f32) vstate.frame_w / (f32) vstate.frame_h;
	if (aspect <= frame_aspect) {
		out_h = vstate.frame_h;
		out_w = out_h * aspect;
	} else {
		out_w = vstate.frame_w;
		out_h = out_w / aspect;
	}

	vstate.frame_output_x = (vstate.frame_w - out_w) / 2;
	vstate.frame_output_y = (vstate.frame_h - out_h) / 2;
	vstate.frame_output_w = out_w;
	vstate.frame_output_h = out_h;
}

void OSW_VideoHintFPS(u32 fps)
{
	if (fps > 0) {
		vstate.target_us = 1000000000L / ((u64) fps);
	}
}

/* Draws the screen */
void OSW_VideoDrawBuffer(const void *fb, u32 w, u32 h)
{
	if (!(vstate.flags & OSW_FLAG_USE_OPENGL)) {
		ogl_Draw(fb, w, h);
	}
}

void OSW_VideoSwapBuffers(void)
{
	__osw_VideoSwapBuffers();
}