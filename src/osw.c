



#include <libosw/osw.h>
#include <stdlib.h>

#include "libosw/video.h"
#include "opengl/gfx_ogl.h"
#include "video_common.h"
#include "input_common.h"
#include "joypad_common.h"



static WSExitFunc osw_exit_func = NULL;

static u32 is_inited = 0;

/* Inits osw */
u32 OSW_Init(const char* title, u32 w, u32 h, u32 flags)
{
	if (is_inited) {
		return -1;
	}
	u32 err = OSW_OK;

	err = __osw_VideoInit(flags);
	if (err != OSW_OK) {
		return err;
	}
	OSW_VideoSetFilter(OSW_VIDEO_FILTER_SHARP);
	OSW_VideoSetSize(w, h);
	OSW_VideoTitleSet(title);
	OSW_VideoHintFPS(60);

	__osw_InputInit();
	err = __osw_JoypadInit();
	if (err != OSW_OK) {
		goto err_joypad;
		return err;
	}
	/* Init the corresponding graphics API */
	if (!(vstate.flags & OSW_FLAG_USE_OPENGL)) {
		ogl_Init();
	}
	is_inited = 1;
	return err;

err_joypad:
	__osw_VideoExit();
	return err;
}

/* Exits program */
void OSW_Exit(u32 exit_code)
{
	if (!is_inited) {
		return;
	}

	if (osw_exit_func) {
		if (osw_exit_func(exit_code) == OSW_EXIT_CANCEL) {
			return;
		}
	}

	__osw_JoypadExit();
	__osw_VideoExit();
	exit(exit_code);
}

/* Sets function to be called when trying to exit Katsu */
void OSW_ExitFuncSet(WSExitFunc func)
{
	osw_exit_func = func;
}

#define JOY_SEARCH_ALARM		60
u32 joy_search_timer = 0;

/* Polls joypads and window events */
void OSW_Poll(void)
{
	/* TODO: This should not be automatic... */
	if (joy_search_timer == JOY_SEARCH_ALARM) {
		__osw_JoypadSearchAndOpen();
		joy_search_timer = 0;
	}
	__osw_JoypadPoll();
	__osw_VideoPoll();
	joy_search_timer++;
}


