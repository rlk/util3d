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

/*----------------------------------------------------------------------------*/

#ifndef UTIL3D_GLSL_H
#define UTIL3D_GLSL_H

#include <GL/glew.h>

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/

struct glsl
{
    char  *vert_filename;
    char  *frag_filename;

    GLuint vert_shader;
    GLuint frag_shader;

    GLuint program;
};

typedef struct glsl glsl;

/*----------------------------------------------------------------------------*/

char *copy_str(const char *);
char *load_str(const char *);

GLuint glsl_init_shader (GLenum, const char *, int);
GLuint glsl_init_program(GLuint, GLuint);

int check_shader_log(GLuint);
int check_program_log(GLuint);

/*----------------------------------------------------------------------------*/

GLboolean glsl_source(glsl *, const char *, int, const char *, int);
GLboolean glsl_create(glsl *, const char *,      const char *);
GLboolean glsl_reload(glsl *);
void      glsl_delete(glsl *);

GLint     glsl_uniform(GLuint, const char *, ...);

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif
#endif
