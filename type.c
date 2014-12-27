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

#include <assert.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "type.h"

/*----------------------------------------------------------------------------*/

typedef struct sort sort;
typedef struct vert vert;

/* A VERT gives the spatial position and atlas coordinate of one corner of a  */
/* glyph rectangle. A glyph has four verts, and a line is a string of glyphs. */
/* The process of type setting is the computation of verts from sorts.        */

struct vert
{
    float v[3];
    float t[2];
};

/* A SORT gives the position of a specific letter face in an atlas, including */
/* the UNICODE character code, glyph metrics, and atlas location.             */

struct sort
{
    int c;                              /* Character code                     */
    int x;                              /* Glyph X bearing                    */
    int y;                              /* Glyph Y bearing                    */
    int a;                              /* Glyph pixel advance                */
    int X;                              /* Atlas X position                   */
    int Y;                              /* Atlas Y position                   */
    int W;                              /* Atlas width                        */
    int H;                              /* Atlas height                       */
};

/* Sorts are indeed sorted by character. This comparison function allows the  */
/* standard C bsearch and qsort functions to be applied.                      */

static int sortcmp1(const void *A, const void *B)
{
    const int  *p = (const int  *) A;
    const sort *S = (const sort *) B;

    if (*p < S->c) return -1;
    if (*p > S->c) return +1;

    return 0;
}

static int sortcmp2(const void *A, const void *B)
{
    const sort *S = (const sort *) A;
    const sort *T = (const sort *) B;

    if (S->c < T->c) return -1;
    if (S->c > T->c) return +1;

    return 0;
}

/*----------------------------------------------------------------------------*/

struct font
{
    FT_Library library;                 /* FreeType library                   */
    FT_Face    face;                    /* FreeType face                      */
    int        h;                       /* Height                             */
    int        a;                       /* Normal space advance               */
    float      k;                       /* Letter space factor                */

    sort      *sorts;                   /* Sort array                         */
    int        n;                       /* Number of sorts renderered         */

    GLuint     atlas;                   /* Atlas texture object               */
    int        s;                       /* Size of the atlas                  */
    int        x;                       /* Current atlas cursor position      */
    int        y;                       /* Current atlas cursor position      */
    int        m;                       /* Height of the current atlas line   */
};

struct line
{
    GLuint vbo;                         /* Vertex buffer object               */
    vert  *v;                           /* Vertex buffer                      */
    font  *F;                           /* Font upref                         */
    int    n;                           /* Number of characters               */
};

/*----------------------------------------------------------------------------*/

/* Decode and return the next UTF8 character refered to by the given string   */
/* handle. See that *((*p)++) business down there? Avoid doing that.          */

static int utf8(const char **p)
{
    if ((**p & 0x80) == 0x00)
    {
        int a = *((*p)++);

        return a;
    }
    if ((**p & 0xE0) == 0xC0)
    {
        int a = *((*p)++) & 0x1F;
        int b = *((*p)++) & 0x3F;

        return (a << 6) | b;
    }
    if ((**p & 0xF0) == 0xE0)
    {
        int a = *((*p)++) & 0x0F;
        int b = *((*p)++) & 0x3F;
        int c = *((*p)++) & 0x3F;

        return (a << 12) | (b << 6) | c;
    }
    if ((**p & 0xF8) == 0xF0)
    {
        int a = *((*p)++) & 0x07;
        int b = *((*p)++) & 0x3F;
        int c = *((*p)++) & 0x3F;
        int d = *((*p)++) & 0x3F;

        return (a << 18) | (b << 12) | (c << 8) | d;
    }
    return 0;
}

/*----------------------------------------------------------------------------*/

/* Compute a reasonable size for the atlas. Find the first power of two value */
/* larger than a multiple of the font base line. This is quite arbitrary.     */

static int atlas_size(int h)
{
    int s = 1;

    while (s < h * 4)
        s *= 2;

    return s;
}

static GLuint atlas_create(int s)
{
    GLubyte *p = (GLubyte *) calloc(s, s);
    GLuint   o;

    glGenTextures(1, &o);
    glBindTexture(GL_TEXTURE_2D, o);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, s, s, 0,
                                   GL_ALPHA, GL_UNSIGNED_BYTE, p);
    free(p);

    return o;
}

static void atlas_delete(GLuint o)
{
    glDeleteTextures(1, &o);
}

/*----------------------------------------------------------------------------*/

static void font_sort_create(font *F, sort *S, int c)
{
    /* Render the glyph and cache its metrics. */

    FT_Load_Glyph(F->face, FT_Get_Char_Index(F->face, c), FT_LOAD_DEFAULT);

    S->x = F->face->glyph->metrics.horiBearingX >> 6;
    S->y = F->face->glyph->metrics.horiBearingY >> 6;
    S->W = F->face->glyph->metrics.width        >> 6;
    S->H = F->face->glyph->metrics.height       >> 6;
    S->a = F->face->glyph->advance.x            >> 6;

    FT_Render_Glyph(F->face->glyph, FT_RENDER_MODE_NORMAL);

    /* Assign this sort a position in the atlas. */

    if (F->x + S->W >= F->s)
    {
        F->y += F->m + 1;
        F->x  = 0;
        F->m  = 0;
    }

    S->X  = F->x;
    S->Y  = F->y;
    F->x += S->W + 1;
    F->m  = S->H > F->m ? S->H : F->m;

    /* Upload the glyph image to the atlas. */

    glPushAttrib(GL_PIXEL_MODE_BIT);
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glPixelTransferf(GL_RED_BIAS,   1.0f);
        glPixelTransferf(GL_GREEN_BIAS, 1.0f);
        glPixelTransferf(GL_BLUE_BIAS,  1.0f);

        glBindTexture  (GL_TEXTURE_2D, F->atlas);
        glTexSubImage2D(GL_TEXTURE_2D, 0, S->X, S->Y, S->W, S->H, GL_ALPHA,
                        GL_UNSIGNED_BYTE, F->face->glyph->bitmap.buffer);
    }
    glPopAttrib();
}

/* Locate and return the sort for the given character. If no sort is found,   */
/* insert and initialize a new one. This is a bit slow to insert a new sort   */
/* (being lazily written with realloc and qsort) but each character will be   */
/* used many times and inserted at most once, so the penalty is minimal.      */

static sort *font_sort_search(font *F, int c)
{
    size_t sz = sizeof (sort);
    sort   *S = NULL;
    sort   *T = NULL;

    if ((S = (sort *) bsearch(&c, F->sorts, (F->n), sz, sortcmp1)) == 0)
    {
        if ((T = (sort *) realloc(F->sorts, (F->n + 1) * sz)))
        {
            S = T + F->n;
            S->c     = c;
            F->n    += 1;
            F->sorts = T;

            font_sort_create(F, S, c);

            qsort(F->sorts, F->n, sz, sortcmp2);

            return font_sort_search(F, c);
        }
    }
    return S;
}

/*----------------------------------------------------------------------------*/

static void warn(const char *mesg, const char *name)
{
    if (name)
        fprintf(stderr, "type : %s : %s\n", mesg, name);
    else
        fprintf(stderr, "type : %s\n", mesg);
}

/* Try to use the given memory buffer as a raw font resource. Failing that,   */
/* assume the pointer gives the name of a font file.                          */

static int font_load(const void *ptr, size_t len, font *F)
{
    if (FT_New_Memory_Face(F->library, (const FT_Byte *) ptr, len, 0, &F->face))
    {
        if (FT_New_Face(F->library, (const char *) ptr, 0, &F->face))
        {
            return 0;
        }
        else return 1;
    }
    else return 1;
}

/* Initialize FreeType and load the given font at the given size. Create a    */
/* new empty texture atlas.                                                   */

font *font_create(const void *ptr, size_t len, int size, float k)
{
    font *F = NULL;

    assert(ptr);
    assert(size > 0);

    if ((F = (font *) calloc(1, sizeof (font))))
    {
        if (FT_Init_FreeType(&F->library) == 0)
        {
            if (font_load(ptr, len, F))
            {
                if (FT_Set_Pixel_Sizes(F->face, 0, size) == 0)
                {
                    FT_UInt i = FT_Get_Char_Index(F->face, ' ');

                    FT_Load_Glyph(F->face, i, FT_LOAD_DEFAULT);

                    F->k     = k;
                    F->a     = F->face->glyph->advance.x     >> 6;
                    F->h     = F->face->size->metrics.height >> 6;
                    F->s     = atlas_size  (F->h);
                    F->atlas = atlas_create(F->s);
                }
                else warn("Failure to set font pixel size", NULL);
            }
            else warn("Failure to load font", NULL);
        }
        else warn("Failure to initialize FreeType", NULL);
    }
    return F;
}

/* Release all resources held by the given font structure.                    */

void font_delete(font *F)
{
    assert(F);

    atlas_delete(F->atlas);

    FT_Done_Face(F->face);
    FT_Done_FreeType(F->library);

    free(F->sorts);
    free(F);
}

int font_height(font *F)
{
    assert(F);
    return F->h;
}

/*----------------------------------------------------------------------------*/

static int line_set(vert *v, sort *S, int s, int x, float k, const double *M)
{
    /* Compute the texture coordinate rectangle. */

    float tl = (float)  S->X         / s;
    float tr = (float) (S->X + S->W) / s;
    float tt = (float)  S->Y         / s;
    float tb = (float) (S->Y + S->H) / s;

    /* Compute the vertex rectangle. */

    float vl = (float) S->x        + x;
    float vr = (float) S->x + S->W + x;
    float vt = (float) S->y;
    float vb = (float) S->y - S->H;

    /* Set the attributes of each vertex. */

    v[0].v[0] = (float) (M[0] * vl + M[4] * vb + M[12]);
    v[0].v[1] = (float) (M[1] * vl + M[5] * vb + M[13]);
    v[0].v[2] = (float) (M[2] * vl + M[6] * vb + M[14]);
    v[0].t[0] = tl;
    v[0].t[1] = tb;

    v[1].v[0] = (float) (M[0] * vr + M[4] * vb + M[12]);
    v[1].v[1] = (float) (M[1] * vr + M[5] * vb + M[13]);
    v[1].v[2] = (float) (M[2] * vr + M[6] * vb + M[14]);
    v[1].t[0] = tr;
    v[1].t[1] = tb;

    v[2].v[0] = (float) (M[0] * vr + M[4] * vt + M[12]);
    v[2].v[1] = (float) (M[1] * vr + M[5] * vt + M[13]);
    v[2].v[2] = (float) (M[2] * vr + M[6] * vt + M[14]);
    v[2].t[0] = tr;
    v[2].t[1] = tt;

    v[3].v[0] = (float) (M[0] * vl + M[4] * vt + M[12]);
    v[3].v[1] = (float) (M[1] * vl + M[5] * vt + M[13]);
    v[3].v[2] = (float) (M[2] * vl + M[6] * vt + M[14]);
    v[3].t[0] = tl;
    v[3].t[1] = tt;

    return (int) (x + S->a * k);
}

static int layout(vert *v, const char *str, int exp, double *mat, font *F)
{
    const char *p;
    sort *S;

    int s = 0;
    int n = 0;
    int e = 0;
    int x = 0;
    int i = 0;
    int c;

    FT_UInt   r = 0;
    FT_UInt   l = 0;
    FT_Vector k;

    /* Count the number of spaces and non-spaces in this string. */

    for (p = str; (c = utf8(&p));)
        if (c == ' ')
            s++;
        else
            n++;

    /* Find a sort and a kern, and set each character. */

    for (p = str; (c = utf8(&p));)

        if (c == ' ')
        {
            x += (int) (F->a * F->k + (exp * (e + 1) / s) - (exp * e / s));
            e += 1;
        }
        else if ((S = font_sort_search(F, c)))
        {
            r = FT_Get_Char_Index(F->face, c);
                FT_Get_Kerning   (F->face, l, r, FT_KERNING_DEFAULT, &k);
            l = r;

            x = line_set(v + 4 * i, S, F->s, x + (k.x >> 6), F->k, mat);
            i++;
        }

    return i;
}

/* Initialize a new line and its array of vertices. Request a sort for each   */
/* character of the given string. Set the vertices of the line accordingly.   */

line *line_layout(int strc, const char * const *strv, int *expv, double *matv, font *F)
{
    const size_t sz = sizeof (vert);
    const char   *p = NULL;

    line *L = NULL;
    int   n = 0;
    int   i;
    int   c;

    double I[16] = {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    };

    assert(strv);
    assert(F);

    /* Count the total number of non-spaces in the text. */

    for (i = 0; i < strc; i++)
        for (p = strv[i]; (c = utf8(&p));)
            if (c != ' ')
                n++;

    /* Allocate and initialize a line structure. */

    if ((L = (line *) calloc(1, sizeof (line))))
    {
        /* Allocate storage for the vertex array. */

        if ((L->v = (vert *) calloc(n * 4, sz)))
        {
            /* Typeset each string in turn. */

            int  j = 0;
            for (i = 0; i < strc; i++)
            {
                I[13] = -i * F->h;
                j += layout(L->v + 4 * j, strv[i], expv ? expv[i] : 0,
                                                   matv ? matv + i * 16 : I, F);
            }

            /* Copy the data to a vertex buffer object. */

            glGenBuffers(1, &L->vbo);
            glBindBuffer(GL_ARRAY_BUFFER, L->vbo);
            glBufferData(GL_ARRAY_BUFFER, 4 * sz * n, L->v, GL_STATIC_DRAW);
        }

        L->n = n;
        L->F = F;
    }
    return L;
}

/* Create a single line of text with default expansion. This is just a short  */
/* cut that invokes the layout function with default arguments.               */

line *line_create(const char *str, font *F)
{
    return line_layout(1, &str, NULL, NULL, F);
}

void line_delete(line *L)
{
    if (L)
    {
        glDeleteBuffers(1, &L->vbo);
        free(L->v);
        free(L);
    }
}

/* Compute the unexpanded pixel length of the given string when rendered with */
/* the given font at its current size.                                        */

int line_length(const char *str, font *F)
{
    FT_UInt   r = 0;
    FT_UInt   l = 0;
    FT_Vector k;

    const char *p;
    sort *S;
    int   c;
    int   x = 0;

    assert(F);

    for (p = str; (c = utf8(&p));)

        if (c == ' ')
        {
            x += (int) (F->a * F->k);
        }
        else if ((S = font_sort_search(F, c)))
        {
            r = FT_Get_Char_Index(F->face, c);
                FT_Get_Kerning   (F->face, l, r, FT_KERNING_DEFAULT, &k);
            l = r;
            x += (int) (S->a * F->k);
        }

    return x;
}

void line_render(line *L)
{
    if (L)
    {
        const size_t sv = sizeof (vert);
        const size_t sf = sizeof (float);

        glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
        {
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            /* Activate the atlas texture and vertex buffer. */

            glBindTexture(GL_TEXTURE_2D,   L->F->atlas);
            glBindBuffer (GL_ARRAY_BUFFER, L->vbo);

            /* Set the location and layout of the vertex attributes. */

            glVertexPointer  (3, GL_FLOAT, sv, (GLvoid *) (0));
            glTexCoordPointer(2, GL_FLOAT, sv, (GLvoid *) (3 * sf));

            /* Render all text. */

            glDrawArrays(GL_QUADS, 0, L->n * 4);
        }
        glPopClientAttrib();
    }
}

/*----------------------------------------------------------------------------*/