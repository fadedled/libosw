/*
 * input_common.h
 */


#ifndef __LIBOSW_INPUT_COMMON_H__
#define __LIBOSW_INPUT_COMMON_H__


#include "libosw/input.h"

#define OSW_MAX_EVENT_BUFFER_SIZE        (1024)
#define OSW_KEYBOARD_POLLING             1
#define OSW_MOUSE_POLLING                2

typedef struct InputState_t {
	u32 polling;
	u32 kb_head;
	u32 kb_num;
	OSWMouse mouse;
} InputState;

extern InputState input_state;

void __osw_InputInit(void);
u32 __osw_KeyboardAddEvent(OSWKeyEvent *kev);
void __osw_MouseUpdateState(s32 new_x, s32 new_y);

#endif /*__LIBOSW_VIDEO_COMMON_H__*/
