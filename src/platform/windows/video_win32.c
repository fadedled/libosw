
#include <libosw/osw.h>
#include "../../video_common.h"
#include "../../joypad_common.h"
#include "../../input_common.h"
#include <windows.h>
#include <strsafe.h>
/* #include <shellscalingapi.h> */
#include "../../opengl/gl.h"
#include <wingdi.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

/* WGL context creation defines */
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001

typedef HGLRC (WINAPI * WGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);
typedef const char* (WINAPI * WGLGETEXTENSIONSSTRINGARBPROC)(HDC);
typedef BOOL (WINAPI * WGLOSWAPINTERVALEXTPROC)(int);


struct KTVideoX11_t {
	HWND hwin;
	HDC hdc;
	HGLRC glc;

} osw_win;

u32 wkey_mapping[16];

void __osw_winResize(HWND hWnd, int nWidth, int nHeight)
{
	RECT cli_rect, win_rect;
	u32 dpi = GetDpiForWindow(hWnd);
	GetClientRect(hWnd, &cli_rect);
	GetWindowRect(hWnd, &win_rect);
	u32 brd_x = (win_rect.right - win_rect.left) - cli_rect.right;
	u32 brd_y = (win_rect.bottom - win_rect.top) - cli_rect.bottom;
	u32 scale_w = MulDiv((nWidth + brd_x), dpi, USER_DEFAULT_SCREEN_DPI);
	u32 scale_h = MulDiv((nHeight + brd_y), dpi, USER_DEFAULT_SCREEN_DPI);
	MoveWindow(hWnd, win_rect.left, win_rect.top, scale_w, scale_h, TRUE);
}


static LRESULT CALLBACK __osw_winCallback(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
	OSWKeyEvent kev;
	u32 k;
	switch(msg) {
		case WM_SIZE: {
			vstate.frame_w = LOWORD(lparam);
			vstate.frame_h = HIWORD(lparam);
		} break;
		case WM_WINDOWPOSCHANGING: {
			RECT cli_rect, win_rect;
			GetClientRect(osw_win.hwin, &cli_rect);
			vstate.frame_w = cli_rect.right - cli_rect.left;
			vstate.frame_h = cli_rect.bottom - cli_rect.top;
		} break;
		case WM_DESTROY: {
		} break;
		case WM_CLOSE: {
			OSW_Exit(0);
		} break;
		case WM_ACTIVATEAPP: {
		} break;
		case WM_PAINT: {
			//Should swap manually...
		} break;
		case WM_LBUTTONDOWN: input_state.mouse.btn0++; input_state.mouse.btn |= OSW_MOUSE_BTN0; break;
		case WM_RBUTTONDOWN: input_state.mouse.btn1++; input_state.mouse.btn |= OSW_MOUSE_BTN1; break;
		case WM_LBUTTONUP:   input_state.mouse.btn &= ~OSW_MOUSE_BTN0; break;
		case WM_RBUTTONUP:   input_state.mouse.btn &= ~OSW_MOUSE_BTN1; break;
		case WM_MOUSEWHEEL:  input_state.mouse.scroll += (GET_WHEEL_DELTA_WPARAM(wparam) > 0 ? 1 : -1); break;
		case WM_MOUSEMOVE: {
			u32 mouse_x = GET_X_LPARAM(lparam);
			u32 mouse_y = GET_Y_LPARAM(lparam);
			__osw_MouseUpdateState(mouse_x, mouse_y);
		} break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN: {
			/* Obtain scan code and see if there is a mapping */
			u32 keyFlags = HIWORD(lparam);
			u32 keycode = LOBYTE(keyFlags); /* switch ? */
			u32 scancode = LOWORD(wparam); /* switch ? */
			if ((keyFlags & KF_EXTENDED) == KF_EXTENDED) {
				keycode |= 0xE000;
			}
			if (input_state.polling & OSW_KEYBOARD_POLLING) {
				/* If keyboard polling is active report key */
				kev.type = OSW_KEYEV_TYPE_PRESSED;
				kev.keycode = keycode;
				/* kev.mod = mod; */
				kev.sym = scancode;
				__osw_KeyboardAddEvent(&kev);
			} else {
				/* Inactive keyboard polling makes keyboard act as a joypad */
				for (k = 0; k < 14; ++k) {
					if (wkey_mapping[k] == keycode) {
						joy_state[0].btn |= (1 << k);
					}
				}
			}
		} break;
		case WM_KEYUP:
		case WM_SYSKEYUP: {
			/* Obtain scan code and see if there is a mapping */
			u32 keyFlags = HIWORD(lparam);
			u32 keycode = LOBYTE(keyFlags); /* switch ? */
			u32 scancode = LOWORD(wparam); /* switch ? */
			if ((keyFlags & KF_EXTENDED) == KF_EXTENDED) {
				keycode |= 0xE000;
			}
			if (input_state.polling & OSW_KEYBOARD_POLLING) {
				/* If keyboard polling is active report key */
				kev.type = OSW_KEYEV_TYPE_RELEASED;
				kev.keycode = keycode;
				/* kev.mod = mod; */
				kev.sym = scancode;
				__osw_KeyboardAddEvent(&kev);
			} else {
				/* Inactive keyboard polling makes keyboard act as a joypad */
				for (k = 0; k < 14; ++k) {
					if (wkey_mapping[k] == keycode) {
						joy_state[0].btn &= ~(1 << k);
					}
				}
			}
		} break;
		case WM_DPICHANGED: {
			__osw_winResize(osw_win.hwin, vstate.frame_w, vstate.frame_h);
		} break;
		default:
			return DefWindowProc(win, msg, wparam, lparam);
	}

	return 0;
}



u32  __osw_VideoInit(u32 flags)
{
	/* Open a window */
	HGLRC glc_old;
	const LPCTSTR CLASS_NAME = TEXT("osw_winClass");
	WNDCLASS wclass = {0};
	wclass.style = CS_OWNDC;
	wclass.lpfnWndProc = __osw_winCallback;
	wclass.hInstance = GetModuleHandle(0);
	wclass.lpszClassName = CLASS_NAME;
	wclass.hCursor = LoadCursor(NULL, IDC_ARROW);

	if (!RegisterClass(&wclass)) {
		return 1;
	}

	osw_win.hwin = CreateWindow(CLASS_NAME, "",
							   WS_OVERLAPPEDWINDOW | WS_VISIBLE,
							   CW_USEDEFAULT, CW_USEDEFAULT,
							   vstate.frame_w, vstate.frame_h,
							   NULL, NULL, wclass.hInstance, NULL);
	if (!osw_win.hwin){
		return 1;
	}
	__osw_winResize(osw_win.hwin, vstate.frame_w, vstate.frame_h);
	osw_win.hdc = GetDC(osw_win.hwin);
	/* SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE); */

	/* Create OpenGL context */
	PIXELFORMATDESCRIPTOR pfd = {0};
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW
				| PFD_SUPPORT_OPENGL
				| PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;
	SetPixelFormat(osw_win.hdc, ChoosePixelFormat(osw_win.hdc, &pfd), &pfd);
	if (!(glc_old = wglCreateContext(osw_win.hdc))) {
		return 1;
	}
	wglMakeCurrent(osw_win.hdc, glc_old);

	/* Get OpenGL 4.3 */
	WGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
	WGLGETEXTENSIONSSTRINGARBPROC  wglGetExtensionsStringARB = NULL;
	wglCreateContextAttribsARB = (WGLCREATECONTEXTATTRIBSARBPROC)
				wglGetProcAddress("wglCreateContextAttribsARB");
	wglGetExtensionsStringARB = (WGLGETEXTENSIONSSTRINGARBPROC)
				wglGetProcAddress("wglGetExtensionsStringARB");

	const char* wglExts = wglGetExtensionsStringARB(osw_win.hdc);
	if (strstr(wglExts, "WGL_ARB_create_context") && wglCreateContextAttribsARB) {
		int context_attribs[7] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 1,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0};
		/* Create OpenGL context and make current */
		wglDeleteContext(glc_old);
		osw_win.glc = wglCreateContextAttribsARB(osw_win.hdc, 0, context_attribs);
		if (!osw_win.glc) {
			return 1;
		}
		wglMakeCurrent(osw_win.hdc, osw_win.glc);
	}
	else {
		return 1;
	}
	/* Set Vsync, it won't cry if we can't get it */
	if (strstr(wglExts, "WGL_EXT_oswap_control ")) {
		WGLOSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
		wglSwapIntervalEXT = (WGLOSWAPINTERVALEXTPROC)
						wglGetProcAddress("wglSwapIntervalEXT");
		wglSwapIntervalEXT(1);
	}

	/* Generate the keyboard mappings */
	joy_state[0].active = 1;
	wkey_mapping[0]  = 0xE04B;  /* Left */
	wkey_mapping[1]  = 0xE04D;  /* Right */
	wkey_mapping[2]  = 0xE048;  /* Up */
	wkey_mapping[3]  = 0xE050;  /* Down */
	wkey_mapping[4]  = 0x002C;  /* Z */
	wkey_mapping[5]  = 0x002D;  /* X */
	wkey_mapping[6]  = 0x001E;  /* A */
	wkey_mapping[7]  = 0x001F;  /* S */
	wkey_mapping[8]  = 0x0010;  /* Q */
	wkey_mapping[9]  = 0x0011;  /* W */
	wkey_mapping[10] = 0x0012;  /* E */
	wkey_mapping[11] = 0x0020;  /* D */
	wkey_mapping[12] = 0x001C;  /* Return */
	wkey_mapping[13] = 0x0036;  /* Shift_R */

	vstate.flags = flags;
	ShowWindow(osw_win.hwin, SW_SHOW);
	return OSW_OK;
}


void __osw_VideoExit(void)
{
	/* Destroy the OpenGL context */
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(osw_win.glc);
	PostQuitMessage(0);
}


void __osw_VideoAttrSet(u32 attr, void *val)
{
	switch (attr) {
		case OSW_VIDEO_ATTR_TITLE: {
			SetWindowTextA(osw_win.hwin, (const char *) val);
		} break;
		case OSW_VIDEO_ATTR_FRAME: {
			u32 w = vstate.frame_w;
			u32 h = vstate.frame_h;
			__osw_winResize(osw_win.hwin, w, h);
			/* if (frame == OSW_VIDEO_FRAME_FULLSCREEN) { */
		} break;
	}
}



void __osw_VideoPoll(void)
{
	MSG msg = {0};
	/* events */
	GetMessage(&msg, NULL, 0, 0);
	TranslateMessage(&msg);
	DispatchMessage(&msg);

	input_state.mouse.btn0 = 0;
	input_state.mouse.btn1 = 0;
	input_state.mouse.scroll = 0;

}


void __osw_VideoSwapBuffers(void)
{
	SwapBuffers(osw_win.hdc);
	/* Add sleep timer ?? */
}
