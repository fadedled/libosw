/*
 * joypad_common.h
 */


#ifndef __LIBOSW_JOYPAD_COMMON_H__
#define __LIBOSW_JOYPAD_COMMON_H__

#include "libosw/joypad.h"

typedef struct JoypadState_t {
	u32 active;

	u32 btn;
	u32 btn_prev;

	s8 stick[4];
	s8 stick_prev[4];

} JoypadState;

extern JoypadState joy_state[MAX_JOYPADS];


u32  __osw_JoypadInit(void);
void __osw_JoypadExit(void);
void __osw_JoypadAttrSet(u32 attr, u32 val);
void __osw_JoypadSearchAndOpen(void);
void __osw_JoypadPoll(void);


#endif /*__LIBOSW_JOYPAD_COMMON_H__*/
