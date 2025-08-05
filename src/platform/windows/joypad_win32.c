

#include <stdio.h>
#include <libosw/osw.h>
#include "../../joypad_common.h"
#include <windows.h>
#include <xinput.h>

#define MAX_WIN_JOYPADS			4		//Max joypads for Windows (up to 4)
#define AXIS_CODE_MAX			8

/*Joystick state for a max of 8 joypads*/
static struct Joypad_Device_tag {
	/* Platform dependant state */
	int				fd;				//File descriptor
	u64				id;				//device ID
} joy_dev[MAX_WIN_JOYPADS];


//joypad assigned to the keyboard
u32 kb_joy = 0;


static void __osw_JoyClose(u32 joy_id)
{
	u32 k;
	joy_state[joy_id].active = 0;
	joy_state[joy_id].btn = 0;
	joy_state[joy_id].stick[JOY_STICK_LX] = 0;
	joy_state[joy_id].stick[JOY_STICK_LY] = 0;
	joy_state[joy_id].stick[JOY_STICK_RX] = 0;
	joy_state[joy_id].stick[JOY_STICK_RY] = 0;

	/*Find next joypad for keyboard */
	joy_state[kb_joy].active = 0u;
	for (k = 0; k < MAX_WIN_JOYPADS; ++k) {
		if (joy_dev[k].fd == -1) {
			kb_joy = k;
			joy_state[kb_joy].active = 1u;
			return;
		}
	}
}


void __osw_JoypadSearchAndOpen(void)
{
	u32 i;
	XINPUT_STATE state = {0};
	for (i = 0; i < MAX_WIN_JOYPADS; ++i) {
		if (XInputGetState(i, &state) == ERROR_SUCCESS) {
			joy_state[i].active = 1;
		}
	}
	//TODO: do keyboard search
	/*Set the keyboard buttons for the remaning controller*/
}


u32  __osw_JoypadInit(void)
{
	//Get all currently conected gamepads
	__osw_JoypadSearchAndOpen();

	//Set the keyboard as joypad0
	kb_joy = 0;
	joy_state[0].active = 1u;

	return OSW_OK;
}


void __osw_JoypadExit(void)
{
	u32 i;
	for (i = 0; i < MAX_WIN_JOYPADS; ++i) {
		__osw_JoyClose(i);
	}
}


void __osw_JoypadAttrSet(u32 attr, u32 val)
{

}

const u32 xinput_gpad_btn_converter[16] = {
	JOY_UP,		/* XINPUT_GAMEPAD_DPAD_UP */
	JOY_DOWN,	/* XINPUT_GAMEPAD_DPAD_DOWN */
	JOY_LEFT,	/* XINPUT_GAMEPAD_DPAD_LEFT */
	JOY_RIGHT,	/* XINPUT_GAMEPAD_DPAD_RIGHT */
	JOY_STR,	/* XINPUT_GAMEPAD_START */
	JOY_SEL,	/* XINPUT_GAMEPAD_BACK */
	0x0, 		/* XINPUT_GAMEPAD_LEFT_THUMB */
	0x0,		/* XINPUT_GAMEPAD_RIGHT_THUMB */
	JOY_L,		/* XINPUT_GAMEPAD_LEFT_SHOULDER */
	JOY_R,		/* XINPUT_GAMEPAD_RIGHT_SHOULDER */
	JOY_EAST,	/* XINPUT_GAMEPAD_A */
	JOY_SOUTH,	/* XINPUT_GAMEPAD_B */
	JOY_WEST,	/* XINPUT_GAMEPAD_X */
	JOY_NORTH	/* XINPUT_GAMEPAD_Y */
};

void __osw_JoypadPoll(void)
{
	XINPUT_STATE state = {0};
	u32 i, k;
	/* Update the prev values per active joypad */
	for (i = 0; i < MAX_WIN_JOYPADS; ++i) {
		if (joy_state[i].active) {
			if (XInputGetState(i, &state) == ERROR_SUCCESS) {
				XINPUT_GAMEPAD xgamepad = state.Gamepad;
				u32 btn = 0;
				joy_state[i].btn_prev = joy_state[i].btn;
				joy_state[i].stick_prev[0] = joy_state[i].stick[0];
				joy_state[i].stick_prev[1] = joy_state[i].stick[1];
				joy_state[i].stick_prev[2] = joy_state[i].stick[2];
				joy_state[i].stick_prev[3] = joy_state[i].stick[3];

				for (k = 0; k < 16; ++k) {
					btn |= xinput_gpad_btn_converter[k] * (xgamepad.wButtons >> k) & 1;
				}
				btn |= (xgamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) * JOY_TL;
				btn |= (xgamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) * JOY_TR;
				joy_state[i].btn = btn;
				joy_state[i].stick[JOY_STICK_LX] = (s8)(((xgamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE |
				xgamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) * xgamepad.sThumbLX) >> 8);
				joy_state[i].stick[JOY_STICK_LY] = (s8)(((xgamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE |
				xgamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) * xgamepad.sThumbLY) >> 8);
				joy_state[i].stick[JOY_STICK_RX] = (s8)(((xgamepad.sThumbRX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE |
				xgamepad.sThumbRX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) * xgamepad.sThumbRX) >> 8);
				joy_state[i].stick[JOY_STICK_RY] = (s8)(((xgamepad.sThumbRY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE |
				xgamepad.sThumbRY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) * xgamepad.sThumbRY) >> 8);
			}
			else {
				/*Deactivate Joypad if there are errors while reading */
				__osw_JoyClose(i);
			}
		}
	}

	/*Set the keyboard buttons for the remaning controller*/
	/* if (joy_dev[kb_joy].fd == -1) {
		joy_state[kb_joy].btn = kb_btn;
	} */
}
