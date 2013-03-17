/* Copyright (c) 2010 Robert Kooima                                           */
/*                                                                            */
/* Permission is hereby granted, free of charge, to any person obtaining a    */
/* copy of this software and associated documentation files (the "Software"), */
/* to deal in the Software without restriction, including without limitation  */
/* the rights to use, copy, modify, merge, publish, distribute, sublicense,   */
/* and/or sell copies of the Software, and to permit persons to whom the      */
/* Software is furnished to do so, subject to the following conditions:       */
/*                                                                            */
/* The above copyright notice and this permission notice shall be included in */
/* all copies or substantial portions of the Software.                        */
/*                                                                            */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    */
/* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    */
/* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        */
/* DEALINGS IN THE SOFTWARE.                                                  */

#ifndef UTIL3D_DEMO_H
#define UTIL3D_DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/

typedef int  (*demo_init_f)(int, char **);
typedef void (*demo_tilt_f)(void);
typedef void (*demo_quit_f)(void);
typedef void (*demo_draw_f)(void);
typedef void (*demo_step_f)(float);

/*----------------------------------------------------------------------------*/

enum {
    DEMO_STATIC,
    DEMO_TUMBLE,
    DEMO_DOLLY,
    DEMO_FLY
};

enum {
    DEMO_POSITION,
    DEMO_ROTATION,
    DEMO_LIGHT,
    DEMO_POINT,
    DEMO_ZOOM
};

int demo(int, int, char **, demo_init_f, demo_tilt_f, demo_quit_f,
                                         demo_draw_f, demo_step_f);
const float *demo_get(int);

void demo_clear(const float *, const float *);

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif
#endif
