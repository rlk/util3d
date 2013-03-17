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
#include "plane.h"

/*----------------------------------------------------------------------------*/

struct plane
{
    GLuint  vbo[1];
    GLuint  ebo[2];
    GLfloat line[4];
    GLfloat fill[4];
    GLsizei n;
};

/*----------------------------------------------------------------------------*/

struct vert
{
    GLfloat v[3];
    GLfloat n[3];
};

static struct vert *add_vert(struct vert *v, GLfloat x, GLfloat z)
{
    v->v[0] = x;
    v->v[1] = 0;
    v->v[2] = z;

    v->n[0] = 0;
    v->n[1] = 1;
    v->n[2] = 0;

    return v + 1;
}

static GLushort *add_rect(GLushort *e, GLushort i0, GLushort i1,
                                       GLushort i2, GLushort i3)
{
    e[0] = i0;
    e[1] = i2;
    e[2] = i1;
    e[3] = i3;
    e[4] = i1;
    e[5] = i2;

    return e + 6;
}

static void init_verts(GLuint vbo[1], GLushort n, GLfloat g)
{
    GLsizei nv = 4 * (n + 1) * (n + 1), sv = nv * sizeof (struct vert);
    struct vert *v;
    struct vert *w;

    /* Initialize the vertex array. */

    if ((w = v = (struct vert *) malloc(sv)))
    {
        const GLfloat d = 0.50f * n;

        GLushort i, j;

        for     (i = 0; i <= n; ++i)
            for (j = 0; j <= n; ++j)
            {
                w = add_vert(w, j - d - g, i - d - g);
                w = add_vert(w, j - d + g, i - d - g);
                w = add_vert(w, j - d - g, i - d + g);
                w = add_vert(w, j - d + g, i - d + g);
            }
    }

    /* Copy the vertex data to the buffer object. */

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sv, v, GL_STATIC_DRAW);

    free(v);
}

static void init_elems(GLuint ebo[2], GLushort n)
{
    GLsizei nf = 6 * (n    ) * (    n    ), sf = nf * sizeof (GLushort);
    GLsizei nl = 6 * (n + 1) * (3 * n + 1), sl = nl * sizeof (GLushort);

    GLushort *f = 0;
    GLushort *p = 0;
    GLushort *l = 0;
    GLushort *q = 0;
    GLushort  i,  j;

    /* Initialize the element arrays. */

    if ((f = p = (GLushort *) malloc(sf)) &&
        (l = q = (GLushort *) malloc(sl)))

        for     (i = 0; i <= n; ++i)
            for (j = 0; j <= n; ++j)
            {
                const GLushort a = 4 * ((n + 1) * (i    ) + j);
                const GLushort b = 4 * ((n + 1) * (i + 1) + j);

                                    q = add_rect(q, a + 0, a + 1, a + 2, a + 3);
                if (j < n)          q = add_rect(q, a + 1, a + 4, a + 3, a + 6);
                if (i < n)          q = add_rect(q, a + 2, a + 3, b + 0, b + 1);
                if (i < n && j < n) p = add_rect(p, a + 3, a + 6, b + 1, b + 4);
            }

    /* Copy the element data to the buffer object. */

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sf, f, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sl, l, GL_STATIC_DRAW);

    free(f);
    free(l);
}

/*----------------------------------------------------------------------------*/

/* Allocate and initialize a new plane object. There must be a current OpenGL */
/* context at the time.                                                       */

plane *plane_create(int n, float g)
{
    plane *P;

    if ((P = (plane *) malloc(sizeof (plane))))
    {
        const float l[4] = { 0.9f, 0.9f, 0.9f, 1.0f };
        const float f[4] = { 0.8f, 0.8f, 0.8f, 1.0f };

        glGenBuffers(1, P->vbo);
        glGenBuffers(2, P->ebo);

        init_verts(P->vbo, n, g);
        init_elems(P->ebo, n);

        plane_color(P, l, f);

        P->n = n;
    }
    return P;
}

/* Release the given plane structure and all resources held by it.            */

void plane_delete(plane *P)
{
    assert(P);

    glDeleteBuffers(2, P->ebo);
    glDeleteBuffers(1, P->vbo);

    free(P);
}

/*----------------------------------------------------------------------------*/

/* Render the given plane structure.                                          */

void plane_render(plane *P)
{
    const GLsizei nf = 6 * (P->n    ) * (    P->n    );
    const GLsizei nl = 6 * (P->n + 1) * (3 * P->n + 1);

    const size_t sz = sizeof (GLfloat);

    glEnable(GL_COLOR_MATERIAL);
    {
        /* Enable the necessary array pointers. */

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        {
            /* Bind the array pointers to the array buffer object. */

            glBindBuffer(GL_ARRAY_BUFFER, P->vbo[0]);
            {
                glVertexPointer(3, GL_FLOAT, sz * 6, (GLvoid *) (     0));
                glNormalPointer(   GL_FLOAT, sz * 6, (GLvoid *) (sz * 3));

                /* Render the squares. */

                glColor4fv(P->fill);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, P->ebo[0]);
                glDrawElements(GL_TRIANGLES, nf, GL_UNSIGNED_SHORT, 0);

                /* Render the lines */

                glColor4fv(P->line);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, P->ebo[1]);
                glDrawElements(GL_TRIANGLES, nl, GL_UNSIGNED_SHORT, 0);

                /* Revert all state. */

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    glDisable(GL_COLOR_MATERIAL);
}

void plane_color(plane *P, const float *l, const float *f)
{
    P->line[0] = l[0];
    P->line[1] = l[1];
    P->line[2] = l[2];
    P->line[3] = l[3];
    
    P->fill[0] = f[0];
    P->fill[1] = f[1];
    P->fill[2] = f[2];
    P->fill[3] = f[3];
}        

/*----------------------------------------------------------------------------*/
