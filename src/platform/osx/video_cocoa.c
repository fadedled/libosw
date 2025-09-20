
#include <libosw/osw.h>
#include "../../video_common.h"
#include "../../input_common.h"
#include "libosw/input.h"

#include <limits.h>
#include <AppKit/NSEvent.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <objc/NSObjCRuntime.h>
#include <objc/NSObjCRuntime.h>
#include <OpenGL/gl.h>

#include <CoreGraphics/CGBase.h>
#include <CoreGraphics/CGGeometry.h>
typedef CGPoint NSPoint;
typedef CGRect NSRect;

//TODO: Is this somewhere?
#if __LP64__ || (TARGET_OS_EMBEDDED && !TARGET_OS_IPHONE) || TARGET_OS_WIN32 || NS_BUILD_32_LIKE_64
	#define NSEncSInt "q"
	#define NSEncUInt "L"
#else
	#define NSEncSInt "i"
	#define NSEncUInt "I"
#endif

extern id NSApp;
extern id const NSDefaultRunLoopMode;

#ifdef __arm64__ /* ARM just uses objc_msgSend */
	#define abi_objc_msgSend_stret objc_msgSend
	#define abi_objc_msgSend_fpret objc_msgSend
#else /* __i386__ */ /* x86 has two msgSend functions */
	#define abi_objc_msgSend_stret objc_msgSend_stret
	#define abi_objc_msgSend_fpret objc_msgSend_fpret
#endif

#ifndef MAC_OS_X_VERSION_10_12
#define MAC_OS_X_VERSION_10_12 101200
#endif

/* macOS 10.12 deprecated many constants, #define the new names for older SDKs */
#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_12
	#define NSEventMaskAny                  NSAnyEventMask
	#define NSEventModifierFlagCommand      NSCommandKeyMask
	#define NSEventModifierFlagControl      NSControlKeyMask
	#define NSEventModifierFlagOption       NSAlternateKeyMask
	#define NSEventTypeFlagsChanged         NSFlagsChanged
	#define NSEventTypeKeyUp                NSKeyUp
	#define NSEventTypeKeyDown              NSKeyDown
	#define NSEventTypeMouseMoved           NSMouseMoved
	#define NSEventTypeLeftMouseDown        NSLeftMouseDown
	#define NSEventTypeRightMouseDown       NSRightMouseDown
	#define NSEventTypeOtherMouseDown       NSOtherMouseDown
	#define NSEventTypeLeftMouseDragged     NSLeftMouseDragged
	#define NSEventTypeRightMouseDragged    NSRightMouseDragged
	#define NSEventTypeOtherMouseDragged    NSOtherMouseDragged
	#define NSEventTypeLeftMouseUp          NSLeftMouseUp
	#define NSEventTypeRightMouseUp         NSRightMouseUp
	#define NSEventTypeOtherMouseUp         NSOtherMouseUp
	#define NSEventTypeScrollWheel          NSScrollWheel
	#define NSTextAlignmentCenter           NSCenterTextAlignment
	#define NSWindowStyleMaskBorderless     NSBorderlessWindowMask
	#define NSWindowStyleMaskClosable       NSClosableWindowMask
	#define NSWindowStyleMaskMiniaturizable NSMiniaturizableWindowMask
	#define NSWindowStyleMaskTitled         NSTitledWindowMask
#endif


/* Use this to make the casting a bit more bearable */
#define MSG(rtype, ...)      ((rtype (*)(__VA_ARGS__))objc_msgSend)
#define MSGS(rtype, ...)     ((rtype (*)(__VA_ARGS__))abi_objc_msgSend_stret)
#define MSGF(rtype, ...)     ((rtype (*)(__VA_ARGS__))abi_objc_msgSend_fpret)
#define cls                 objc_getClass
#define sel                 sel_registerName
#define alloc_cls(name)     MSG(id, Class, SEL)(cls(name), sel_Alloc)

/* Selectors ========================== */
SEL sel_distantPast;
SEL sel_nextEventMatchingMask;
SEL sel_frame;
SEL sel_type;
SEL sel_buttonNumber;
SEL sel_keyCode;
SEL sel_keyWindow;
SEL sel_mouseLocationOutsideOfEventStream;
SEL sel_convertRectToBacking;
SEL sel_scrollingDeltaX;
SEL sel_scrollingDeltaY;
SEL sel_hasPreciseScrollingDeltas;
SEL sel_modifierFlags;
SEL sel_characters;
SEL sel_UTF8String;
SEL sel_sendEvent;
SEL sel_setTitle;
SEL sel_updateWindows;
SEL sel_update;
SEL sel_flushBuffer;
Class class_NSString;
id content_view;

struct KTVideoOSX_t {
	id win; /* window */
	id glc; /* GL context */
	/* struct timespec time_begin; */
} osw_osx;


void __windowClose(id self, SEL _sel, id notification)
{
	//Do something...
}

NSUInteger __appTerminateRequest(id self, SEL _sel, id sender)
{
	//Should do things before terminating window??
	return 0;
}

void __initSel(void)
{
	sel_distantPast = sel("distantPast");
	sel_nextEventMatchingMask = sel("nextEventMatchingMask:untilDate:inMode:dequeue:");
	sel_frame = sel("frame");
	sel_type = sel("type");
	sel_buttonNumber = sel("buttonNumber");
	sel_keyCode = sel("keyCode");
	sel_keyWindow = sel("keyWindow");
	sel_mouseLocationOutsideOfEventStream = sel("mouseLocationOutsideOfEventStream");
	sel_convertRectToBacking = sel("convertRectToBacking:");
	sel_scrollingDeltaX = sel("scrollingDeltaX");
	sel_scrollingDeltaY = sel("scrollingDeltaY");
	sel_hasPreciseScrollingDeltas = sel("hasPreciseScrollingDeltas");
	sel_modifierFlags = sel("modifierFlags");
	sel_characters = sel("characters");
	sel_UTF8String = sel("UTF8String");
	sel_sendEvent = sel("sendEvent:");
	sel_setTitle = sel("setTitle:");
	sel_updateWindows = sel("updateWindows");
	sel_update = sel("update");
	sel_flushBuffer = sel("flushBuffer");
}

u32  __osw_VideoInit(u32 flags)
{
	SEL sel_Alloc = sel("alloc");
	SEL sel_Init = sel("init");
	SEL sel_Autorelease = sel("autorelease");
	Class class_NSString = cls("NSString");

	/* Autorelease Pool */
	id pool = MSG(id, id, SEL)(alloc_cls("NSAutoreleasePool"), sel_Init);

	/* NSApplication */
	Class class_NSApplication = cls("NSApplication");
	MSG(id, Class, SEL)(class_NSApplication, sel("sharedApplication"));

	/* NSApp */
	MSG(void, id, SEL, NSInteger)(NSApp, sel("setActivationPolicy"), 0);

	/* AppDelegate */
	Class class_AppDelegate = objc_allocateClassPair(cls("NSObject"), "AppDelegate", 0);
	Protocol * prot_NSApplicationDelegate = objc_getProtocol("NSApplicationDelegate");
	/* TODO: Is this necesary? */
	class_addProtocol(class_AppDelegate, prot_NSApplicationDelegate);
	class_addMethod(class_AppDelegate, sel("applicationShouldTerminate:"), (IMP)__appTerminateRequest, NSEncUInt "@:@");
	id dg = MSG(id, id, SEL)(MSG(id, Class, SEL)(class_AppDelegate, sel_Alloc), sel_Init);
	MSG(void, id, SEL)(dg, sel_Autorelease);
	MSG(void, id, SEL, id)(NSApp, sel("setDelegate:"), dg);

	/* Finish launching (will not use [NSApp run]) */
	MSG(void, id, SEL)(NSApp, sel("finishLaunching"));

	//TODO: MENUBAR & ITEMS

	/* Create window */
	NSRect wrect = {{0, 0}, {640, 480}};
	/* TODO: Check styleMask = 15 constants */
	osw_osx.win = MSG(id, id, SEL, NSRect, NSUInteger, NSUInteger, BOOL)(alloc_cls("NSWindow"), sel("initWithContentRect:styleMask:backing:defer:"), wrect, 15, 2, NO);
	MSG(void, id, SEL)(osw_osx.win, sel_Autorelease);
	MSG(void, id, SEL, BOOL)(osw_osx.win, sel("setTeleasedWhenClosed:"), NO);

	/* WindowDelegate for window resize and closing */
	Class class_WindowDelegate = objc_allocateClassPair(cls("NSObject"), "WindowDelegate", 0);
	Protocol *prot_NSWindowDelegate = objc_getProtocol("NSWindowDelegate");
	class_addProtocol(class_WindowDelegate, prot_NSWindowDelegate);
	class_addMethod(class_WindowDelegate, sel("windowWillClose:"), (IMP)__windowClose, "v@:@");
	//??
	//class_addMethod(class_WindowDelegate, sel("windowWillClose:"), (IMP)__windowClose, "v@:@");
	//TODO: Other things like content view
	id wdg = MSG(id, id, SEL)(MSG(id, Class, SEL)(class_WindowDelegate, sel_Alloc), sel_Init);
	MSG(void, id, SEL)(wdg, sel_Autorelease);
	MSG(void, id, SEL, id)(osw_osx.win, sel("setDelegate:"), wdg);

	content_view = MSG(id, id, SEL)(osw_osx.win, sel("contentView"));
	MSG(void, id, SEL, BOOL)(content_view, sel("setWantsBestResolutionOpenGLSurface:"), YES);

	NSPoint point = {20, 20};
	MSG(void, id, SEL, NSPoint)(osw_osx.win, sel("cascadeTopLeftFromPoint:"), point);

	/* Create OpengGL context */
	u32 gl_attr[] = {
		8, 24,  /* Color size */
		11, 8,  /* Alpha size */
		5,          /* Use double buffer */
		73,         /* Hardware accelerate */
		72,         /* No recovery (TODO: what?) */
		/* v Is this for MSAA? v */
		55, 1,  /* # of sample buffers */
		56, 4, /* # of samples */
		99, 0x4100, /* OpenGL version 4.1 (last version supported by Apple) */
		0 /* END */
	};
	id pix_format = MSG(id, id, SEL, const u32*)(alloc_cls("NSOpenGlPixelFormat"), sel("initWithAttributes:"), gl_attr);
	MSG(void, id, SEL)(pix_format, sel_Autorelease);
	osw_osx.glc = MSG(id, id, SEL, id, id)(alloc_cls("NSOpenGLContext"), sel("initWithFormat:shareContext:"), pix_format, nil);
	MSG(void, id, SEL)(osw_osx.glc, sel_Autorelease);

	MSG(void, id, SEL, id)(osw_osx.glc, sel("setView:"), content_view);
	MSG(void, id, SEL, id)(osw_osx.win, sel("makeKeyAndOrderFront:"), osw_osx.win);
	MSG(void, id, SEL, BOOL)(osw_osx.win, sel("setAcceptsMouseMovedEvents:"), YES);

	//TODO: Do some other things
	id back_color = MSG(id, Class, SEL)(cls("NSColor"), sel("blackColor"));
	MSG(void, id, SEL, id)(osw_osx.win, sel("setBackgroundColor:"), back_color);

	//TODO: Only need to be done once?
	MSG(void, id, SEL)(osw_osx.glc, sel("makeCurrentContext"));

	/* Add joypad keybindings */


	return OSW_OK;
}


void __osw_VideoExit(void)
{
	/* Should I do something? */
}


void __osw_VideoAttrSet(u32 attr, void *val)
{
	switch (attr) {
		case OSW_VIDEO_ATTR_TITLE: {
			/* TODO: does this allocate mem? */
			id title_str = MSG(id, Class, SEL, const char*)(class_NSString, sel("stringWithUTF8String"), (char *)val);
			MSG(void, id, SEL, id)(osw_osx.win, sel_setTitle, title_str);
		} break;
		case OSW_VIDEO_ATTR_FRAME: {
			u32 w = vstate.frame_w;
			u32 h = vstate.frame_h;
			/* What does this do? */
			MSG(void, id, SEL, CGSize)(osw_osx.win, sel("setContentSize:"), CGSizeMake((CGFloat)w, (CGFloat)h));
#if 0
			XResizeWindow(osw_x11.dpy, osw_x11.win, w, h);

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
	OSWKeyEvent kev;
	NSInteger mod_flags = 0;
	//TODO: Depracated types
	NSUInteger event_mask = NSEventMaskKeyDown | NSEventMaskKeyUp | StructureNotifyMask;

	/* Only check button events when mouse polling is active */
	if (input_state.polling & OSW_MOUSE_POLLING) {
		polling_mask |= NSEventMaskLeftMouseDown | NSEventMaskLeftMouseUp | NSEventMaskRightMouseDown | NSEventMaskRightMouseUp | NSEventMaskScrollWheel;
	}

	input_state.mouse.btn0 = 0;
	input_state.mouse.btn1 = 0;
	input_state.mouse.scroll = 0;

	id dist_past = MSG(id, Class, SEL)(cls("NSDate"), sel_distantPast);
	id event = MSG(id, id, SEL, NSUInteger, id, id, BOOL)(NSApp, sel_nextEventMatchingMask, NSUIntegerMax, dist_past, NSDefaultRunLoopMode, YES);
	if (event) {
		NSUInteger ev_type = MSG(NSUInteger, id, SEL)(event, sel_type);
		switch (ev_type) {
			case NSEventTypeMouseMoved: {/* NSMouseMoved */
			} break;
			case NSEventTypeLeftMouseDown: {/* NSLeftMouseDown */
				input_state.mouse.btn0++; input_state.mouse.btn |= OSW_MOUSE_BTN0;
			} break;
			case NSEventTypeLeftMouseUp: {/* NSLeftMouseUp */
				input_state.mouse.btn &= ~OSW_MOUSE_BTN0;
			} break;
			case NSEventTypeRightMouseDown: {/* NSRightMouseDown */
				input_state.mouse.btn1++; input_state.mouse.btn |= OSW_MOUSE_BTN1;
			} break;
			case NSEventTypeRightMouseUp: {/* NSRightMouseUp */
				input_state.mouse.btn &= ~OSW_MOUSE_BTN1;
			} break;
			case NSEventTypeScrollWheel: {/* NSScrollWheel */
			} break;
			case NSEventTypeFlagsChanged: {
				mod_flags = MSG(NSInteger, id, SEL)(event, sel_modifierFlags);
				/* TODO: change mod keys */
			} break;
			case NSEventTypeKeyDown: {/* NSKeyDown */
				NSInteger key_code = MSG(NSInteger, id, SEL)(event, sel_keyCode);
				if (input_state.polling & OSW_KEYBOARD_POLLING) {
					/* If keyboard polling is active report key */
					kev.type = OSW_KEYEV_TYPE_PRESSED;
					kev.keycode = key_code;
					kev.mod = mod_flags;
					//kev.sym = sym;
					__osw_KeyboardAddEvent(&kev);
				} else {
					/* Inactive keyboard polling makes keyboard act as a joypad */
					//for (k = 0; k < 14; ++k) {
					//	if (xkey_mapping[k] == key_ev.keycode) {
					//		joy_state[0].btn |= (1 << k);
					//	}
					//}
				}
			} break;
			case NSEventTypeKeyUp: {/* NSKeyUp */
				NSInteger key_code = MSG(NSInteger, id, SEL)(event, sel_keyCode);
				if (input_state.polling & OSW_KEYBOARD_POLLING) {
					/* If keyboard polling is active report key */
					kev.type = OSW_KEYEV_TYPE_RELEASED;
					kev.keycode = key_code;
					kev.mod = mod_flags;
					//kev.sym = sym;
					__osw_KeyboardAddEvent(&kev);
				} else {
					/* Inactive keyboard polling makes keyboard act as a joypad */
					//for (k = 0; k < 14; ++k) {
					//	if (xkey_mapping[k] == key_ev.keycode) {
					//		joy_state[0].btn |= (1 << k);
					//	}
					//}
				}
			} break;

		}
		//TODO: What does thid do?
		MSG(void, id, SEL, id)(NSApp, sel_sendEvent, event);
		MSG(void, id, SEL)(NSApp, sel_updateWindows);
	}
	/* Update frame rectangle */
	NSRect rect = MSGS(NSRect, id, SEL)(content_view, sel_frame);
	rect = MSGS(NSRect, id, SEL, NSRect)(content_view, sel_convertRectToBacking, rect);
	vstate.frame_w = rect.size.width;
	vstate.frame_h = rect.size.height;
}


void __osw_VideoSwapBuffers(void)
{
	//TODO: don't use sel()
	MSG(void, id, SEL)(osw_osx.glc, sel_flushBuffer);
}
