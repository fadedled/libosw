
**OSW** is a small C library for OS creating windows. it's main purpouse is for creating software rasterizers.

## Building for Windows (MSVC)

If you are using Visual Studio you must set up the Visual Studio enviroment variables (this can be done in Windows Terminal by selecting the VS enviroment in the arrow dropdown menu). Then just run the ```build_win.bat``` batch file, this creates the static library ```lib/Osw.lib```.

### Using for your projects

You will need MSVC installed, link with ```User32.lib```, ```Gdi32.lib```, ```Opengl32.lib```, ```Xinput9_1_0.lib``` (or ```Xinput1_4.lib```) and ```Osw.lib```. If you are using the console/terminal you should set your environment to use MSVC. Then compile with:

```
$> cl -Tc <your_source_code> -I".\include" -link User32.lib Gdi32.lib Opengl32.lib Xinput9_1_0.lib lib\Osw.lib -OUT:mygame.exe
$> mygame.exe
```

## Building for Linux (Makefile/GCC)

Run the ```make``` command, this will generate the static library ```lib/libosw.a```.

### Using for your projects

Development packages for X11, OpenGL, and PipeWire are needed when compiling your project. You can download them from your package manager as ```libx11-dev``` and ```libgl1-mesa-dev``` for Debian-based distros or as ```libX11-devel``` and ```mesa-libGL-devel``` for RedHat-based distros. After they are installed, you can compile and run from the terminal with:

```
$> gcc <your_source_code> -o mygame -I./include  -L./lib -lGL -lX11 -lm -ldl -losw
$> ./mygame
```

## Installing OSW for your project

Copy the static library to your project's ```lib``` folder, also copy the ```include/libosw``` to your project's ```include``` folder.


### Example

The following is a very basic example for creating a window:

```c
#include <libosw/osw.h>

#define WIN_WIDTH 320
#define WIN_HEIGHT 240

u32 framebuffer[WIN_WIDTH * WIN_HEIGHT] = {0};

int main() {
	//Create window and check for errors
	u32 err = OSW_Init("My window", WIN_WIDTH * 2, WIN_HEIGHT * 2, 0);
	if (err != OSW_OK) {
		return err;
	}

	//Enable mouse/keyboard polling
	OSW_MouseSetPolling(OSW_ENABLE);
	OSW_KeyboardSetPolling(OSW_ENABLE); // Also disables keyboard as joypad

	/* Init application */

	OSWMouse mouse;
	OSWKeyEvent kev;
	while (1) { // loop forever
		OSW_Poll();	// Poll mouse/keyboard/joypads
		OSW_MouseGetState(&mouse);
		while (OSW_KeyboardGetEvent(&kev)) {
			if (kev.type == OSW_KEYEV_TYPE_PRESSED) {
				/* do something */
			}
		}

		if (mouse.btn & OSW_MOUSE_BTN0) {
			/* do something when left button is pressed */
		}

		/* Modify the framebuffer pixels */

		// Draw the frame and show it on screen
		OSW_VideoDrawBuffer(framebuffer, WIN_WIDTH, WIN_HEIGHT);
		OSW_VideoSwapBuffers();
	}
	return 0;
}
```


## Joypads and Keyboard mappings

There is support for upto four joypad controllers that are found automatically. The keyboard defaults to the first joypad channel that has no controller connected. All applications use the same 11 keys from the keyboard that map onto a joypad (the mapping is disabled when OSW_KeyboardSetPolling(OSW_ENABLE) is called):

- The four arrow keys and Z, X, A, S, Q, W, D, E, Enter, and RightShift
<!-- Press Esc to exit form full screen -->

