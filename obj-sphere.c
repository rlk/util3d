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
#include "obj.h"

/*----------------------------------------------------------------------------*/

/* Generate an OBJ model of a unit sphere with n subdivisions south-to-north. */
/* Include texture coordinates for a cylindrical mapping and tangent vectors  */
/* to match it. Do not allow the OBJ module to compute the tangent vectors,   */
/* as it is especially bad at "combing the hair on a sphere."                 */

obj *obj_sphere_create(int N)
{
    obj *O = obj_create(0);
    int  M = 2 * N;
    int  i;
    int  j;

    /* Generate the surface. */

    int si = obj_add_surf(O);

    /* Generate the vertices. */

    for     (i = 0; i <= N; ++i)
        for (j = 0; j <= M; ++j)
        {
            int vi = obj_add_vert(O);

            float u[3];
            float n[3];
            float t[2];

            t[0] =  (float) j / (float) M;
            t[1] =  (float) i / (float) N;

            n[0] = -(float) (sin(2.0 * M_PI * t[0]) * sin(M_PI * t[1]));
            n[1] = -(float) (                         cos(M_PI * t[1]));
            n[2] = -(float) (cos(2.0 * M_PI * t[0]) * sin(M_PI * t[1]));

            u[0] = -(float) cos(2.0 * M_PI * t[0]);
            u[1] =  (float) (0.0);
            u[2] =  (float) sin(2.0 * M_PI * t[0]);

            obj_set_vert_v(O, vi, n);
            obj_set_vert_t(O, vi, t);
            obj_set_vert_n(O, vi, n);
            obj_set_vert_u(O, vi, u);
        }

    /* Generate the polys. */

    for     (i = 0; i < N; ++i)
        for (j = 0; j < M; ++j)
        {
            int i00 = (i    ) * (M + 1) + (j    );
            int i01 = (i    ) * (M + 1) + (j + 1);
            int i10 = (i + 1) * (M + 1) + (j    );
            int i11 = (i + 1) * (M + 1) + (j + 1);

            int vi[3];

            vi[0] = i00;
            vi[1] = i01;
            vi[2] = i11;

            obj_set_poly(O, si, obj_add_poly(O, si), vi);

            vi[0] = i11;
            vi[1] = i10;
            vi[2] = i00;

            obj_set_poly(O, si, obj_add_poly(O, si), vi);
        }

    return O;
}

/*----------------------------------------------------------------------------*/
