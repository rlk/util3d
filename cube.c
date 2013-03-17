/* Copyright (c) 2009 Robert Kooima                                           */
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

#include <stdlib.h>
#include <assert.h>

#include <GL/glew.h>

#include "image.h"
#include "cube.h"

/*----------------------------------------------------------------------------*/

struct cube
{
    GLuint tex[6];
    GLuint vbo[1];
    GLuint ebo[1];
};

/*----------------------------------------------------------------------------*/

/* The following data give the vertices of the standard unit cube.  These     */
/* may be stored in a vertex array or buffer object and used directly with    */
/* a call to glDrawArrays(GL_QUADS, 0, 24). The vertex ordering and texture   */
/* coordinates provided give face orientations in line with the definition    */
/* of GL_TEXTURE_CUBE_MAP.                                                    */

struct vert
{
    GLfloat v[3];
    GLfloat n[3];
    GLfloat t[2];
};

static const struct vert verts[24] = {

    /* +X */
    {{  1.f,  1.f,  1.f }, {  1.f,  0.f,  0.f }, { 0.f, 1.f }},
    {{  1.f, -1.f,  1.f }, {  1.f,  0.f,  0.f }, { 0.f, 0.f }},
    {{  1.f, -1.f, -1.f }, {  1.f,  0.f,  0.f }, { 1.f, 0.f }},
    {{  1.f,  1.f, -1.f }, {  1.f,  0.f,  0.f }, { 1.f, 1.f }},

    /* -X */
    {{ -1.f,  1.f, -1.f }, { -1.f,  0.f,  0.f }, { 0.f, 1.f }},
    {{ -1.f, -1.f, -1.f }, { -1.f,  0.f,  0.f }, { 0.f, 0.f }},
    {{ -1.f, -1.f,  1.f }, { -1.f,  0.f,  0.f }, { 1.f, 0.f }},
    {{ -1.f,  1.f,  1.f }, { -1.f,  0.f,  0.f }, { 1.f, 1.f }},

    /* +Y */
    {{ -1.f,  1.f, -1.f }, {  0.f,  1.f,  0.f }, { 0.f, 0.f }},
    {{ -1.f,  1.f,  1.f }, {  0.f,  1.f,  0.f }, { 0.f, 1.f }},
    {{  1.f,  1.f,  1.f }, {  0.f,  1.f,  0.f }, { 1.f, 1.f }},
    {{  1.f,  1.f, -1.f }, {  0.f,  1.f,  0.f }, { 1.f, 0.f }},

    /* -Y */
    {{ -1.f, -1.f,  1.f }, {  0.f, -1.f,  0.f }, { 0.f, 0.f }},
    {{ -1.f, -1.f, -1.f }, {  0.f, -1.f,  0.f }, { 0.f, 1.f }},
    {{  1.f, -1.f, -1.f }, {  0.f, -1.f,  0.f }, { 1.f, 1.f }},
    {{  1.f, -1.f,  1.f }, {  0.f, -1.f,  0.f }, { 1.f, 0.f }},

    /* +Z */
    {{ -1.f,  1.f,  1.f }, {  0.f,  0.f,  1.f }, { 0.f, 0.f }},
    {{ -1.f, -1.f,  1.f }, {  0.f,  0.f,  1.f }, { 0.f, 1.f }},
    {{  1.f, -1.f,  1.f }, {  0.f,  0.f,  1.f }, { 1.f, 1.f }},
    {{  1.f,  1.f,  1.f }, {  0.f,  0.f,  1.f }, { 1.f, 0.f }},

    /* -Z */
    {{  1.f,  1.f, -1.f }, {  0.f,  0.f, -1.f }, { 0.f, 0.f }},
    {{  1.f, -1.f, -1.f }, {  0.f,  0.f, -1.f }, { 0.f, 1.f }},
    {{ -1.f, -1.f, -1.f }, {  0.f,  0.f, -1.f }, { 1.f, 1.f }},
    {{ -1.f,  1.f, -1.f }, {  0.f,  0.f, -1.f }, { 1.f, 0.f }},
};

static const GLushort elems[36] = {
    0,  1,  2,  0,  2,  3,
    4,  5,  6,  4,  6,  7,
    8,  9, 10,  8, 10, 11,
    12, 13, 14, 12, 14, 15,
    16, 17, 18, 16, 18, 19,
    20, 21, 22, 20, 22, 23
};

static void init_vbo(struct cube *C)
{
    glBindBuffer(GL_ARRAY_BUFFER,         C->vbo[0]);
    glBufferData(GL_ARRAY_BUFFER,         sizeof(verts), verts, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, C->ebo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elems), elems, GL_STATIC_DRAW);
}

/*----------------------------------------------------------------------------*/

static const char *names[6] = {
    "cubepx.png",
    "cubenx.png",
    "cubepy.png",
    "cubeny.png",
    "cubepz.png",
    "cubenz.png",
};

/* Load all texture images and initialize all OpenGL exture objects.          */

static void init_tex(struct cube *C)
{
    void *p;
    int   w;
    int   h;
    int   c;
    int   b;
    int   i;

    for (i = 0; i < 6; ++i)
        if ((p = image_read(names[i], &w, &h, &c, &b)))
        {
            int f = image_internal_form(c, b);
            int e = image_external_form(c);
            int t = image_external_type(b);
            
            glBindTexture(GL_TEXTURE_2D, C->tex[i]);
            glTexImage2D (GL_TEXTURE_2D, 0, f, w, h, 0, e, t, p);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
}

/*----------------------------------------------------------------------------*/

/* Allocate and initialize a new cube object. There must be a current OpenGL  */
/* context at the time.                                                       */

cube *cube_create(void)
{
    cube *C;

    if ((C = (cube *) malloc(sizeof (cube))))
    {
        glGenBuffers (1, C->vbo);
        glGenBuffers (1, C->ebo);
        glGenTextures(6, C->tex);

        init_vbo(C);
        init_tex(C);
    }
    return C;
}

/* Release the given cube structure and all resources held by it.             */

void cube_delete(cube *C)
{
    assert(C);

    glDeleteTextures(6, C->tex);
    glDeleteBuffers (1, C->ebo);
    glDeleteBuffers (1, C->vbo);

    free(C);
}

/*----------------------------------------------------------------------------*/

/* Render the given cube structure.                                           */

void cube_render(cube *C)
{
    const size_t sz = sizeof (GLfloat);

    assert(C);

    /* Enable the necessary array pointers. */

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    {
        /* Bind the array pointers to the array buffer object. */

        glBindBuffer(GL_ARRAY_BUFFER,         C->vbo[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, C->ebo[0]);
        {
            size_t i;

            glVertexPointer(  3, GL_FLOAT, sz * 8, (GLvoid *) (     0));
            glNormalPointer(     GL_FLOAT, sz * 8, (GLvoid *) (sz * 3));
            glTexCoordPointer(2, GL_FLOAT, sz * 8, (GLvoid *) (sz * 6));

            /* Draw six quads. */

            for (i = 0; i < 6; ++i)
            {
                glBindTexture(GL_TEXTURE_2D, C->tex[i]);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT,
                               (const GLvoid *) (i * 12));
            }
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER,         0);
    }
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);
}

/*----------------------------------------------------------------------------*/
