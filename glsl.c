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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glsl.h"

/*----------------------------------------------------------------------------*/

char *copy_str(const char *text)
{
    /* Copy the given string into a newly-allocated buffer. */

    size_t len = strlen(text);
    char  *str = 0;

    if ((len > 0) && (str = (char *) malloc(len + 1)))
        memcpy(str, text, len + 1);

    return str;
}

char *load_str(const char *name)
{
    /* Load the named file into a newly-allocated buffer. */

    FILE *fp = 0;
    void  *p = 0;
    size_t n = 0;

    if ((fp = fopen(name, "rb")))
    {
        if (fseek(fp, 0, SEEK_END) == 0)
        {
            if ((n = (size_t) ftell(fp)))
            {
                if (fseek(fp, 0, SEEK_SET) == 0)
                {
                    if ((p = calloc(n + 1, 1)))
                    {
                        fread(p, 1, n, fp);
                    }
                }
            }
        }
        fclose(fp);
    }
    return (char *) p;
}

/*----------------------------------------------------------------------------*/

int check_shader_log(GLuint shader)
{
    GLchar *p = 0;
    GLint   s = 0;
    GLint   n = 0;

    /* Check the shader compile status.  If failed, print the log. */

    glGetShaderiv(shader, GL_COMPILE_STATUS,  &s);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &n);

    if (s == 0)
    {
        if ((p = (GLchar *) calloc(n + 1, 1)))
        {
            glGetShaderInfoLog(shader, n, NULL, p);

            fprintf(stderr, "OpenGL Shader Error:\n%s", p);
            free(p);
        }
        return 0;
    }
    return 1;
}

int check_program_log(GLuint program)
{
    GLchar *p = 0;
    GLint   s = 0;
    GLint   n = 0;

    /* Check the program link status.  If failed, print the log. */

    glGetProgramiv(program, GL_LINK_STATUS,     &s);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &n);

    if (s == 0)
    {
        if ((p = (GLchar *) calloc(n + 1, 1)))
        {
            glGetProgramInfoLog(program, n, NULL, p);

            fprintf(stderr, "OpenGL Program Error:\n%s", p);
            free(p);
        }
        return 0;
    }
    return 1;
}

/*----------------------------------------------------------------------------*/

GLuint glsl_init_shader(GLenum type, const char *str, int len)
{
    if (str)
    {
        /* Compile a new shader with the given source. */

        GLuint shader = glCreateShader(type);

        glShaderSource (shader, 1, (const GLchar **) &str,
                                   (const GLint  *)  &len);
        glCompileShader(shader);

        /* If the shader is valid, return it.  Else, delete it. */

        if (check_shader_log(shader))
            return shader;
        else
        {
            fprintf(stderr, "%s", str);
            glDeleteShader(shader);
        }
    }
    return 0;
}

GLuint glsl_init_program(GLuint shader_vert,
                         GLuint shader_frag)
{
    /* Link a new program object. */

    GLuint program = glCreateProgram();

    glBindAttribLocation(program, 6, "my_Tangent");

    glAttachShader(program, shader_vert);
    glAttachShader(program, shader_frag);

    glLinkProgram(program);

    /* If the program is valid, return it.  Else, delete it. */

    if (check_program_log(program))
        return program;
    else
        glDeleteProgram(program);

    return 0;
}

/*----------------------------------------------------------------------------*/

GLboolean glsl_source(glsl *G, const char *vert_str, int vert_len,
                               const char *frag_str, int frag_len)
{
    if (vert_str && frag_str)
    {
        /* Compile the shaders. */

        G->vert_filename = NULL;
        G->frag_filename = NULL;

        G->vert_shader = glsl_init_shader(GL_VERTEX_SHADER,   vert_str,
                                                        (int) vert_len);
        G->frag_shader = glsl_init_shader(GL_FRAGMENT_SHADER, frag_str,
                                                        (int) frag_len);

        /* Link the program. */

        if (G->vert_shader && G->frag_shader)
        {
            G->program = glsl_init_program(G->vert_shader, G->frag_shader);
            return GL_TRUE;
        }
    }
    return GL_FALSE;
}

GLboolean glsl_create(glsl *G, const char *vert_filename,
                               const char *frag_filename)
{
    GLboolean ret = GL_FALSE;

    if (vert_filename && frag_filename)
    {
        /* Load the shader source. */

        char *vert_str = load_str(vert_filename);
        char *frag_str = load_str(frag_filename);

        /* Compile the shaders. */

        ret = glsl_source(G, vert_str, -1, frag_str, -1);

        /* Cache the given file names to ease reloading. */

        G->vert_filename = copy_str(vert_filename);
        G->frag_filename = copy_str(frag_filename);

        free(frag_str);
        free(vert_str);
    }
    return ret;
}

GLboolean glsl_reload(glsl *G)
{
    if (G->vert_filename && G->frag_filename)
    {
        /* Reload the shader source. */

        char *vert_str = load_str(G->vert_filename);
        char *frag_str = load_str(G->frag_filename);

        /* Delete the old program and shaders. */

        glDeleteProgram(G->program);
        glDeleteShader(G->frag_shader);
        glDeleteShader(G->vert_shader);

        /* Compile the new shaders. */

        G->vert_shader = glsl_init_shader(GL_VERTEX_SHADER,   vert_str, -1);
        G->frag_shader = glsl_init_shader(GL_FRAGMENT_SHADER, frag_str, -1);

        free(frag_str);
        free(vert_str);

        /*  Compile the new program. */

        if (G->vert_shader && G->frag_shader)
        {
            G->program = glsl_init_program(G->vert_shader, G->frag_shader);
            return GL_TRUE;
        }
    }
    return GL_FALSE;
}

void glsl_delete(glsl *G)
{
    /* Delete the program and shaders. */

    glDeleteProgram(G->program);
    glDeleteShader(G->frag_shader);
    glDeleteShader(G->vert_shader);

    /* Release the filename cache. */

    if (G->vert_filename) free(G->vert_filename);
    if (G->frag_filename) free(G->frag_filename);
}

/*----------------------------------------------------------------------------*/

#include <stdarg.h>

GLint glsl_uniform(GLuint program, const char *fmt, ...)
{
    GLint loc = 0;
    va_list ap;

    va_start(ap, fmt);
    {
        char str[256];
        vsprintf(str, fmt, ap);
        loc = glGetUniformLocation(program, str);
    }
    va_end(ap);

    return loc;
}

/*----------------------------------------------------------------------------*/
