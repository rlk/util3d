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

#ifndef UTIL3D_TYPE_H
#define UTIL3D_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/

typedef struct font font;
typedef struct line line;

/*----------------------------------------------------------------------------*/

font *font_create(const void *, size_t, int, float);
int   font_height(font *);
void  font_delete(font *);

line *line_layout(int, const char * const *, int *, double *, font *);
line *line_create(const char *, font *);
int   line_length(const char *, font *);
void  line_render(line *);
void  line_delete(line *);

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif
#endif