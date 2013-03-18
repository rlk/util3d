# demo

This module implements a trivial framework for OpenGL demonstration. It is appropriate for the interactive presentation of rendering techniques and animations, but provides few facilities for mapping user input onto application control. A demo implementation using this module need only provide a function to perform rendering and, optionally, functions for setup, shutdown, and animation.

-   [demo.c](demo.c)
-   [demo.h](demo.h)

The module maintains several pieces of internal state including the position, orientation, and zoom value of the camera, and the direction toward the primary light source. Upon rendering, the camera zoom is loaded into the OpenGL projection matrix, the position and orientation are loaded into the OpenGL model-view matrix, the primary light source direction is loaded into `GL_LIGHT0`, and the mouse pointer vector is loaded into `GL_LIGHT1`.

The demo module is implemented using [GLUT](http://www.opengl.org/resources/libraries/glut/), and applications are free to co-opt any GLUT features not used internally. Notably, this includes [GLUT's menu functions](http://www.opengl.org/resources/libraries/glut/spec3/node35.html#SECTION00070000000000000000), which applications may use for interactive feature selection and configuration.

## Compilation

To use this module, simply link it with your own code. It requires OpenGL, [GLEW](http://glew.sourceforge.net/), and the [image](image.html) utility (to support the screenshot feature), which also pulls in the PNG and zlib libraries.

    cc -o program program.c demo.c image.c -lpng -lz -lm

## API

- `int demo(int mode, int argc, char **argv, demo_init_f init, demo_tilt_f tilt, demo_quit_f quit, demo_draw_f draw, demo_step_f step)`

    The demo API consists of one primary entry point that receives pointers to all pertainant information. This function initializes the OpenGL context and executes the main application loop, calling the provided functions as needed. It does not return until the application is closed. Any of the function arguments may be `NULL` to disable the related callback. The arguments are as follows:

- `int mode`

    This argument selects the application's control style, determining how keyboard and mouse events map onto view state. In general:

    - Mouse drag maps onto camera orientation.
    - Shift-drag maps onto camera zoom.
    - Alt-drag maps onto light source direction.
    - The keyboard maps onto camera position in the traditional first-person style.

    However, the meaning of the camera position and orientation depends upon the selected mode. The following modes are defined:

    - `DEMO_STATIC`

        The camera is locked in place.

    - `DEMO_DOLLY`

        The camera zooms and moves forward or back, but does not rotate.

    - `DEMO_TUMBLE`

        The camera zooms and moves normally, and rotates with an inward-looking object tumble.

    - `DEMO_FLY`

        The camera zooms and moves normally, and rotates with an outward-looking first-person view.

- `int argc, char **argv`

    These arguments should receive the command line argument count and array as passed to the C or C++ `main` function. This allows options to be passed to GLUT, most notably to control window position and size.

- `int (*demo_init_f)(int argc, char **argv)`

    The `init` function is called after the OpenGL context is created, but before the application main loop begins. This provides an opportunity for the application to initialize any necessary OpenGL state. The `argc` and `argv` arguments give the command line arguments remaining *after* GLUT has finished processing the values passed to `demo`. All arguments left by GLUT remain for use by the application.

- `void (*demo_tilt_f)(void)`

    The `tilt` function is called when the user presses the "tilt" key. If needed, applications should respond by flushing and reloading all OpenGL resources. This allows changes to be made to assets without requiring that the entire demo state be reset. For example, the user may modify a shader or texture, save it to disk, and reload it in the demo application without disturbing the current view state.

- `void (*demo_quit_f)(void)`

    The `quit` function is called just prior to application exit. It provides an opportunity to release resources and store state as needed.

- `void (*demo_draw_f)(void)`

    The `draw` function is called each time the display is repainted. Applications should perform rendering here, but should *not* swap buffers, as the `demo` module does this as part of the performance monitoring mechanism.

- `void (*demo_step_f)(float dt)`

    The `step` function is called as often as possible, usually whenever there are no other events to be serviced. The `dt` argument gives the amount of time passed in seconds since the previous call. Applications may use this value to update time-varying values and animations. Applications need *not* request a repaint of the display, as this is automatic due to the free-running nature of all demos.

- `const GLfloat *demo_get(int token)`

    The `demo_get` function allows the application to query the internal state of the demo. Values are returned as a pointer to an array of floats. Valid `token` values and their meanings are as follows:

    - `DEMO_POSITION`

        The current world-space position (x, y, z) of the camera.

    - `DEMO_ROTATION`

        The current orientation (&phi;, &theta;) of the camera.

    - `DEMO_LIGHT`

        The current orientation (&phi;, &theta;) of the light source.

    - `DEMO_POINT`

        The current eye-space vector (x, y, z) of the mouse pointer.

    - `DEMO_ZOOM`

        The current zoom value of the camera.

- `void demo_clear(const GLfloat *top, const GLfloat *bottom)`

    The `demo_clear` function clears the screen using a linear gradient from `top` to `bottom`. If the underlying hardware supports programmable fragment shading then an ordered dither is applied in an effort to smooth the gradient across high-resolution or low bit depth displays.

If the environment variable `DEMO_STATE` is defined at startup then the `demo` module will load camera and light source state from the file named there, if it exists. It will also store camera and light source state to that file upon normal exit.

## Keys

The following keys are bound, though their effect is limited to specific modes:

<table>
  <tr><td>W&nbsp;&nbsp;</td><td>Move forward.</td></tr>
  <tr><td>S&nbsp;&nbsp;</td><td>Move backward.</td></tr>
  <tr><td>A&nbsp;&nbsp;</td><td>Move left.</td></tr>
  <tr><td>D&nbsp;&nbsp;</td><td>Move right.</td></tr>
  <tr><td>C&nbsp;&nbsp;</td><td>Move down.</td></tr>
  <tr><td>Space&nbsp;&nbsp;</td><td>Move up.</td></tr>
  <tr><td>Tab&nbsp;&nbsp;</td><td>"Tilt" the application, triggering a reload of assets.</td></tr>
  <tr><td>Return&nbsp;&nbsp;</td><td>Capture a screenshot and write it to the file `out.png`.</td></tr>
  <tr><td>Escape&nbsp;&nbsp;</td><td>Exit the demo.</td></tr>
</table>

Dvorak equivalents for all keys are simultaneously bound, without conflict. Screenshots are written in 8-bit RGBA format, with the alpha channel masking the color buffer as rendered.
