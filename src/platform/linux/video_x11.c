

#include <libosw/osw.h>
#include "../../video_common.h"
#include "../../joypad_common.h"
#include "../../input_common.h"
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <GL/glx.h>
/* #include <stdio.h> */
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);


struct KTVideoX11_t {
	Display *dpy;
	Window win;
	GLXContext glc;
	/* struct timespec time_begin; */

	/* Atoms */
	Atom wmdelwin;
} osw_x11;

KeyCode xkey_mapping[16];

u32  __osw_VideoInit(u32 flags)
{
	/* Open the X11 display */
	s32 fb_attr[] = {
			GLX_X_RENDERABLE    , True,
			GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
			GLX_RENDER_TYPE     , GLX_RGBA_BIT,
			GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
			GLX_RED_SIZE        , 8,
			GLX_GREEN_SIZE      , 8,
			GLX_BLUE_SIZE       , 8,
			GLX_ALPHA_SIZE      , 8,
			GLX_DEPTH_SIZE      , 24,
			GLX_STENCIL_SIZE    , 8,
			GLX_DOUBLEBUFFER    , True,
			None
	};

	/* Open an X display */
	osw_x11.dpy = XOpenDisplay(NULL);
	if (osw_x11.dpy == NULL) {
		return 1;
	}
	int def_screen = XDefaultScreen(osw_x11.dpy);

	/* Get first framebuffer that matches the settings */
	s32 fbcount;
	GLXFBConfig *fbc_arr = glXChooseFBConfig(osw_x11.dpy, def_screen, fb_attr, &fbcount);
	if (!fbc_arr){
		return 1;
	}
	XVisualInfo *vinfo = glXGetVisualFromFBConfig(osw_x11.dpy, fbc_arr[0]);
	if (!vinfo){
		return 1;
	}

	XSetWindowAttributes win_attr;
	win_attr.colormap = XDefaultColormap(osw_x11.dpy, vinfo->screen);
	win_attr.background_pixel = None;
	win_attr.border_pixel = 0;
	win_attr.event_mask = KeyPressMask | KeyReleaseMask |
						ButtonPressMask | ButtonReleaseMask |
						PointerMotionMask | StructureNotifyMask;
	/* XXX: ResizeRedirectMask? ExposureMask? */

	osw_x11.win = XCreateWindow(osw_x11.dpy, RootWindow(osw_x11.dpy, vinfo->screen),
							   0, 0, 640, 480,
							   0, vinfo->depth, InputOutput, vinfo->visual,
							   CWColormap | CWEventMask | CWBackPixel | CWBorderPixel, &win_attr);

	osw_x11.wmdelwin = XInternAtom(osw_x11.dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(osw_x11.dpy, osw_x11.win, &osw_x11.wmdelwin, 1);

	if (!osw_x11.win) {
		return 2;
	}

	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
		GLX_CONTEXT_MINOR_VERSION_ARB, 1,
		None
    };
	/* Create the OpenGL context */
	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
		glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );
	osw_x11.glc = glXCreateContextAttribsARB(osw_x11.dpy, fbc_arr[0], 0,
											True, context_attribs);
	free(fbc_arr);
	glXMakeCurrent(osw_x11.dpy, osw_x11.win, osw_x11.glc);

	/* Get GLX extensions */
	const char *glxExts = glXQueryExtensionsString(osw_x11.dpy, DefaultScreen(osw_x11.dpy));

	/* Set Vsync, it won't cry if we can't get it */
	int (*glXSwapInterval)(unsigned int interval) = NULL;
	glXSwapInterval = (int (*)(unsigned int)) glXGetProcAddress((const GLubyte *) "glXSwapIntervalMESA");
	if (strstr(glxExts, "GLX_MESA_oswap_control") && glXSwapInterval) {
		glXSwapInterval(1);
	}

	/* Generate the keyboard mappings */
	joy_state[0].active = 1;
	xkey_mapping[0] = XKeysymToKeycode(osw_x11.dpy, XK_Left);
	xkey_mapping[1] = XKeysymToKeycode(osw_x11.dpy, XK_Right);
	xkey_mapping[2] = XKeysymToKeycode(osw_x11.dpy, XK_Up);
	xkey_mapping[3] = XKeysymToKeycode(osw_x11.dpy, XK_Down);
	xkey_mapping[4] = XKeysymToKeycode(osw_x11.dpy, XK_Z);
	xkey_mapping[5] = XKeysymToKeycode(osw_x11.dpy, XK_X);
	xkey_mapping[6] = XKeysymToKeycode(osw_x11.dpy, XK_A);
	xkey_mapping[7] = XKeysymToKeycode(osw_x11.dpy, XK_S);
	xkey_mapping[8] = XKeysymToKeycode(osw_x11.dpy, XK_Q);
	xkey_mapping[9] = XKeysymToKeycode(osw_x11.dpy, XK_W);
	xkey_mapping[10] = XKeysymToKeycode(osw_x11.dpy, XK_E);
	xkey_mapping[11] = XKeysymToKeycode(osw_x11.dpy, XK_D);
	xkey_mapping[12] = XKeysymToKeycode(osw_x11.dpy, XK_Return);
	xkey_mapping[13] = XKeysymToKeycode(osw_x11.dpy, XK_Shift_R);

	vstate.flags = flags;
	OSW_VideoHintFPS(60);
	/* clock_gettime(CLOCK_MONOTONIC, &osw_x11.time_begin); */
	XFree(vinfo);
	XMapWindow(osw_x11.dpy, osw_x11.win);
	XFlush(osw_x11.dpy);
	return OSW_OK;
}


void __osw_VideoExit(void)
{
	XUnmapWindow(osw_x11.dpy, osw_x11.win);

	/* Destroy the OpenGL context */
	glXMakeCurrent(osw_x11.dpy, None, NULL);
	glXDestroyContext(osw_x11.dpy, osw_x11.glc);

	XDestroyWindow(osw_x11.dpy, osw_x11.win);
	XFlush(osw_x11.dpy);
	XCloseDisplay(osw_x11.dpy);
}


void __osw_VideoAttrSet(u32 attr, void *val)
{
	switch (attr) {
		case OSW_VIDEO_ATTR_TITLE: {
			XStoreName(osw_x11.dpy, osw_x11.win, (char *) val);
		} break;
		case OSW_VIDEO_ATTR_FRAME: {
			u32 w = vstate.frame_w;
			u32 h = vstate.frame_h;
			XResizeWindow(osw_x11.dpy, osw_x11.win, w, h);
#if 0
			//if (frame == OSW_VIDEO_FRAME_FULLSCREEN) {
			//}
			//else {

			//}
#endif
		} break;
	}
}



void __osw_VideoPoll(void)
{
	KeySym sym;
	XEvent xev;
	OSWKeyEvent kev;
	u32 polling_mask = KeyPressMask | KeyReleaseMask | StructureNotifyMask;
	u32 k, mod;

	/* TODO: should be done here or after using osw_MouseGetState()? */
	input_state.mouse.btn0 = 0;
	input_state.mouse.btn1 = 0;
	input_state.mouse.scroll = 0;

	/* Check that the user requested a window close */
	while (XCheckTypedWindowEvent(osw_x11.dpy, osw_x11.win, ClientMessage, &xev)) {
		if (xev.xclient.data.l[0] == osw_x11.wmdelwin) {
			OSW_Exit(0);
		}
	}

	/* Only check button events when mouse polling is active */
	if (input_state.polling & OSW_MOUSE_POLLING) {
		polling_mask |= ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
	}

	/* Check the rest of the events */
	while (XCheckWindowEvent(osw_x11.dpy, osw_x11.win, polling_mask, &xev)) {
		switch(xev.type) {
			case KeyPress: {
				XKeyEvent key_ev = xev.xkey;
				if (input_state.polling & OSW_KEYBOARD_POLLING) {
					/* If keyboard polling is active report key */
					kev.type = OSW_KEYEV_TYPE_PRESSED;
					kev.keycode = key_ev.keycode;
					XkbLookupKeySym(osw_x11.dpy, key_ev.keycode, key_ev.state, &mod, &sym);
					kev.mod = mod;
					kev.sym = sym;
					__osw_KeyboardAddEvent(&kev);
				} else {
					/* Inactive keyboard polling makes keyboard act as a joypad */
					for (k = 0; k < 14; ++k) {
						if (xkey_mapping[k] == key_ev.keycode) {
							joy_state[0].btn |= (1 << k);
						}
					}
				}
			} break;
			case KeyRelease:{
				XKeyEvent key_ev = xev.xkey;
				if (input_state.polling & OSW_KEYBOARD_POLLING) {
					/* If keyboard polling is active report key */
					kev.type = OSW_KEYEV_TYPE_PRESSED;
					kev.keycode = key_ev.keycode;
					XkbLookupKeySym(osw_x11.dpy, key_ev.keycode, key_ev.state, &mod, &sym);
					kev.mod = mod;
					kev.sym = sym;
					__osw_KeyboardAddEvent(&kev);
				} else {
					/* Inactive keyboard polling makes keyboard act as a joypad */
					for (k = 0; k < 14; ++k) {
						if (xkey_mapping[k] == key_ev.keycode) {
							joy_state[0].btn &= ~(1 << k);
						}
					}
				}
			} break;
			case ButtonPress: {
				switch(xev.xbutton.button) {
					case Button1: input_state.mouse.btn0++; input_state.mouse.btn |= OSW_MOUSE_BTN0; break;
					case Button3: input_state.mouse.btn1++; input_state.mouse.btn |= OSW_MOUSE_BTN1; break;
					case Button4: input_state.mouse.scroll--; break;
					case Button5: input_state.mouse.scroll++; break;
				}
			} break;
			case ButtonRelease: {
				switch(xev.xbutton.button) {
					case Button1: input_state.mouse.btn &= ~OSW_MOUSE_BTN0; break;
					case Button3: input_state.mouse.btn &= ~OSW_MOUSE_BTN1; break;
				}
			} break;
			case MotionNotify: {
				__osw_MouseUpdateState(xev.xmotion.x, xev.xmotion.y);
			} break;
			case ConfigureNotify:{
				vstate.frame_w = xev.xconfigure.width;
				vstate.frame_h = xev.xconfigure.height;
				if (vstate.flags & OSW_FLAG_USE_OPENGL) {
					vstate.output_w = vstate.frame_w;
					vstate.output_h = vstate.frame_h;
				}
				__osw_UpdateOutputDims();
			} break;
		}
	}
}

void __osw_VideoSwapBuffers(void)
{
	glXSwapBuffers(osw_x11.dpy, osw_x11.win);
#if 0
	/* Calculate next wakeup time (TODO: only when using fps?)*/
	struct timespec	next = osw_x11.time_begin;
	next.tv_sec += (vstate.target_us / 1000000000L);
	next.tv_nsec += (vstate.target_us % 1000000000L);
	if (next.tv_nsec >= 1000000000L) {
		next.tv_sec++;
		next.tv_nsec -= 1000000000L;
	} else if (next.tv_nsec < 0) {
		next.tv_sec--;
		next.tv_nsec += 1000000000L;
	}

	/* Sleep some time until next frame */
	while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL));
	clock_gettime(CLOCK_MONOTONIC, &osw_x11.time_begin);
#endif
}
