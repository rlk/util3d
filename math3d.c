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

#include <assert.h>
#include <math.h>
#include "math3d.h"

/*----------------------------------------------------------------------------*/
/* Vector operations                                                          */

/* Transform homegeneous vector b by matrix M.                                */

void wtransform(real *restrict a, const real *restrict M,
                                  const real *restrict b)
{
    assert(a != b);

    a[0] = M[ 0] * b[0] + M[ 4] * b[1] + M[ 8] * b[2] + M[12] * b[3];
    a[1] = M[ 1] * b[0] + M[ 5] * b[1] + M[ 9] * b[2] + M[13] * b[3];
    a[2] = M[ 2] * b[0] + M[ 6] * b[1] + M[10] * b[2] + M[14] * b[3];
    a[3] = M[ 3] * b[0] + M[ 7] * b[1] + M[11] * b[2] + M[15] * b[3];
}

/* Transform vector b by matrix M.                                            */

void vtransform(real *restrict a, const real *restrict M,
                                  const real *restrict b)
{
    assert(a != b);

    a[0] = M[ 0] * b[0] + M[ 4] * b[1] + M[ 8] * b[2];
    a[1] = M[ 1] * b[0] + M[ 5] * b[1] + M[ 9] * b[2];
    a[2] = M[ 2] * b[0] + M[ 6] * b[1] + M[10] * b[2];
}

/* Transform position b by matrix M.                                          */

void ptransform(real * restrict a, const real *restrict M,
                                   const real *restrict b)
{
    assert(a != b);

    a[0] = M[ 0] * b[0] + M[ 4] * b[1] + M[ 8] * b[2] + M[12];
    a[1] = M[ 1] * b[0] + M[ 5] * b[1] + M[ 9] * b[2] + M[13];
    a[2] = M[ 2] * b[0] + M[ 6] * b[1] + M[10] * b[2] + M[14];
}

/* Compute the vector spherical linear interpolation a of b and c at t.       */

void vslerp(real *a, const real *b, const real *c, real t)
{
    const real d = vdot(b, c);

    if (d < 1.0)
    {
        const real k = acos(d);
        const real u = sin(k - t * k) / sin(k);
        const real v = sin(    t * k) / sin(k);

        a[0] = b[0] * u + c[0] * v;
        a[1] = b[1] * u + c[1] * v;
        a[2] = b[2] * u + c[2] * v;
    }
    else vcpy(a, b);
}

/*----------------------------------------------------------------------------*/
/* Quaternion operations                                                      */

/* Compute the unchecked spherical quaternion interpolation of b and c at t.  */

static void slerp4(real *a, const real *b, const real *c, real t)
{
    const real d = qdot(b, c);

    if (d < 1.0)
    {
        const real k = acos(d);
        const real u = sin(k - t * k) / sin(k);
        const real v = sin(    t * k) / sin(k);

        a[0] = b[0] * u + c[0] * v;
        a[1] = b[1] * u + c[1] * v;
        a[2] = b[2] * u + c[2] * v;
        a[3] = b[3] * u + c[3] * v;
    }
    else qcpy(a, b);
}

/* Compute the auxiliary spline quaternion between c and d (for qsquad.)      */

static void qaux(real *a, const real *b, const real *c, const real *d)
{
    real s[4];
    real t[4];
    real u[4];

    qinvert(s, c);
    qmultiply(t, s, b);
    qmultiply(u, s, d);

    qlog(t, t);
    qlog(u, u);
    qadd(s, t, u);
    qscale(s, s, -0.25);
    qexp(s, s);

    qmultiply(a, c, s);
    qnormalize(a, a);
}

/* Compute the quaternion b raised to the power h.                            */

void qpow(real *a, const real *b, real h)
{
    const real k = acos(b[3]);
    const real s = sqrt(1.0 - b[3] * b[3]);

    a[0] = sin(k * h) * b[0] / s;
    a[1] = sin(k * h) * b[1] / s;
    a[2] = sin(k * h) * b[2] / s;
    a[3] = cos(k * h);
}

/* Compute the quaternion e raised to the power b.                            */

void qexp(real *a, const real *b)
{
    const real k = vlen(b);

    if (k > 0.0)
    {
        const real s = sin(k);
        const real c = cos(k);

        a[0] = b[0] * s / k;
        a[1] = b[1] * s / k;
        a[2] = b[2] * s / k;
        a[3] = c;
    }
    else
    {
        a[0] = 0.0;
        a[1] = 0.0;
        a[2] = 0.0;
        a[3] = 0.0;
    }
}

/* Compute the natural logarithm of quaternion b.                             */

void qlog(real *a, const real *b)
{
    const real s = sqrt(1.0 - b[3] * b[3]);

    if (s > 0.0)
    {
        const real k = acos(b[3]);

        a[0] = b[0] * k / s;
        a[1] = b[1] * k / s;
        a[2] = b[2] * k / s;
        a[3] = 0.0;
    }
    else
    {
        a[0] = 0.0;
        a[1] = 0.0;
        a[2] = 0.0;
        a[3] = 0.0;
    }
}

/* Compute the quaternion spherical linear interpolation a of b and c at t.   */

void qslerp(real *a, const real *b, const real *c, real t)
{
    real C[4];

    /* Check the sign to ensure we interpolate the short way around. */

    qsign(C, b, c);

    slerp4(a, b, C, t);
}

/* Compute the quaternion spherical quadratic interpolation of c and d at t.  */

void qsquad(real *a, const real *b, const real *c,
                     const real *d, const real *e, real t)
{
    real A[4], C[4], D[4], E[4], u[4], v[4], w[4];

    /* Check the signs to ensure we interpolate the short way around. */

    qsign(C, b, c);
    qsign(D, C, d);
    qsign(E, D, e);

    /* Compute auxiliary quaternions giving the spline tangent. */

    qaux(u, b, C, D);
    qaux(v, C, D, E);

    /* Interpolate along the spline. */

    slerp4(A, C, D, t);
    slerp4(w, u, v, t);
    slerp4(a, A, w, 2.0 * t * (1.0 - t));
}

/* Compute the quaternion q giving rotation about vector v through angle a.   */

void qrotate(real *restrict q, const real *restrict v, real a)
{
    const real c = cos(a * 0.5);
    const real s = sin(a * 0.5);

    real t[4];

    t[0] = s * v[0];
    t[1] = s * v[1];
    t[2] = s * v[2];
    t[3] = c;

    qnormalize(q, t);
}

/* Multiply quaternions b and c.                                              */

void qmultiply(real *restrict a, const real *restrict b,
                                 const real *restrict c)
{
    assert(a != b);
    assert(a != c);

    a[0] = b[0] * c[3] + b[3] * c[0] + b[1] * c[2] - b[2] * c[1];
    a[1] = b[1] * c[3] + b[3] * c[1] + b[2] * c[0] - b[0] * c[2];
    a[2] = b[2] * c[3] + b[3] * c[2] + b[0] * c[1] - b[1] * c[0];
    a[3] = b[3] * c[3] - b[0] * c[0] - b[1] * c[1] - b[2] * c[2];

    qnormalize(a, a);
}

/*----------------------------------------------------------------------------*/
/* Transformation matrices                                                    */

/* Give the identity matrix M.                                                */

void midentity(real *restrict M)
{
    M[ 0] =  1.0; M[ 4] =  0.0; M[ 8] =  0.0; M[12] =  0.0;
    M[ 1] =  0.0; M[ 5] =  1.0; M[ 9] =  0.0; M[13] =  0.0;
    M[ 2] =  0.0; M[ 6] =  0.0; M[10] =  1.0; M[14] =  0.0;
    M[ 3] =  0.0; M[ 7] =  0.0; M[11] =  0.0; M[15] =  1.0;
}

/* Give the matrix M rotating about the X axis through angle a.               */

void mrotatex(real *restrict M, real a)
{
    const real s = sin(a);
    const real c = cos(a);

    M[ 0] =  1.0; M[ 4] =  0.0; M[ 8] =  0.0; M[12] =  0.0;
    M[ 1] =  0.0; M[ 5] =    c; M[ 9] =   -s; M[13] =  0.0;
    M[ 2] =  0.0; M[ 6] =    s; M[10] =    c; M[14] =  0.0;
    M[ 3] =  0.0; M[ 7] =  0.0; M[11] =  0.0; M[15] =  1.0;
}

/* Give the matrix M rotating about the Y axis through angle a.               */

void mrotatey(real *restrict M, real a)
{
    const real s = sin(a);
    const real c = cos(a);

    M[ 0] =    c; M[ 4] =  0.0; M[ 8] =    s; M[12] =  0.0;
    M[ 1] =  0.0; M[ 5] =  1.0; M[ 9] =  0.0; M[13] =  0.0;
    M[ 2] =   -s; M[ 6] =  0.0; M[10] =    c; M[14] =  0.0;
    M[ 3] =  0.0; M[ 7] =  0.0; M[11] =  0.0; M[15] =  1.0;
}

/* Give the matrix M rotating about the Z axis through angle a.               */

void mrotatez(real *restrict M, real a)
{
    const real s = sin(a);
    const real c = cos(a);

    M[ 0] =    c; M[ 4] =   -s; M[ 8] =  0.0; M[12] =  0.0;
    M[ 1] =    s; M[ 5] =    c; M[ 9] =  0.0; M[13] =  0.0;
    M[ 2] =  0.0; M[ 6] =  0.0; M[10] =  1.0; M[14] =  0.0;
    M[ 3] =  0.0; M[ 7] =  0.0; M[11] =  0.0; M[15] =  1.0;
}

/* Give the matrix M giving rotation about vector v through angle a.          */

void mrotate(real *restrict M, const real *restrict v, real a)
{
    const real s = sin(a);
    const real c = cos(a);

    real u[3];

    vnormalize(u, v);

    M[ 0] = u[0] * u[0];
    M[ 1] = u[1] * u[0];
    M[ 2] = u[2] * u[0];
    M[ 4] = u[0] * u[1];
    M[ 5] = u[1] * u[1];
    M[ 6] = u[2] * u[1];
    M[ 8] = u[0] * u[2];
    M[ 9] = u[1] * u[2];
    M[10] = u[2] * u[2];

    M[ 0] += (1.0 - M[ 0]) * c;
    M[ 1] += (0.0 - M[ 1]) * c + u[2] * s;
    M[ 2] += (0.0 - M[ 2]) * c - u[1] * s;
    M[ 3]  =  0.0;
    M[ 4] += (0.0 - M[ 4]) * c - u[2] * s;
    M[ 5] += (1.0 - M[ 5]) * c;
    M[ 6] += (0.0 - M[ 6]) * c + u[0] * s;
    M[ 7]  =  0.0;
    M[ 8] += (0.0 - M[ 8]) * c + u[1] * s;
    M[ 9] += (0.0 - M[ 9]) * c - u[0] * s;
    M[10] += (1.0 - M[10]) * c;
    M[11]  =  0.0;
    M[12]  =  0.0;
    M[13]  =  0.0;
    M[14]  =  0.0;
    M[15]  =  1.0;
}

/* Möller et al Eq 4.54 gives a very nice formulation for a rotation matrix   */
/* taking one vector onto another. It's worth adding here.                    */

/* Give the matrix M translating along vector v.                              */

void mtranslate(real *restrict M, const real *restrict v)
{
    M[ 0] =  1.0; M[ 4] =  0.0; M[ 8] =  0.0; M[12] = v[0];
    M[ 1] =  0.0; M[ 5] =  1.0; M[ 9] =  0.0; M[13] = v[1];
    M[ 2] =  0.0; M[ 6] =  0.0; M[10] =  1.0; M[14] = v[2];
    M[ 3] =  0.0; M[ 7] =  0.0; M[11] =  0.0; M[15] =  1.0;
}

/* Give the matrix M scaling by vector v.                                     */

void mscale(real *restrict M, const real *restrict v)
{
    M[ 0] = v[0]; M[ 4] =  0.0; M[ 8] =  0.0; M[12] =  0.0;
    M[ 1] =  0.0; M[ 5] = v[1]; M[ 9] =  0.0; M[13] =  0.0;
    M[ 2] =  0.0; M[ 6] =  0.0; M[10] = v[2]; M[14] =  0.0;
    M[ 3] =  0.0; M[ 7] =  0.0; M[11] =  0.0; M[15] =  1.0;
}

/* Give the matrix M with basis vectors x, y, and z.                          */

void mbasis(real *restrict M, const real *restrict x,
                              const real *restrict y,
                              const real *restrict z)
{
    M[ 0] = x[0]; M[ 4] = y[0]; M[ 8] = z[0]; M[12] = 0.0;
    M[ 1] = x[1]; M[ 5] = y[1]; M[ 9] = z[1]; M[13] = 0.0;
    M[ 2] = x[2]; M[ 6] = y[2]; M[10] = z[2]; M[14] = 0.0;
    M[ 3] =  0.0; M[ 7] =  0.0; M[11] =  0.0; M[15] = 1.0;
}

/* Give the orthogonal projection matrix M with given distances to the left,  */
/* right, bottom, top, near and far clipping planes.                          */

void morthogonal(real *restrict M, real l, real r,
                                   real b, real t,
                                   real n, real f)
{
    M[ 0] =  2.0 / (r - l);
    M[ 1] =  0.0;
    M[ 2] =  0.0;
    M[ 3] =  0.0;
    M[ 4] =  0.0;
    M[ 5] =  2.0 / (t - b);
    M[ 6] =  0.0;
    M[ 7] =  0.0;
    M[ 8] =  0.0;
    M[ 9] =  0.0;
    M[10] = -2.0 / (f - n);
    M[11] =  0.0;
    M[12] = -(r + l) / (r - l);
    M[13] = -(t + b) / (t - b);
    M[14] = -(f + n) / (f - n);
    M[15] =  1.0;
}

/* Give the perspective projection matrix with given distances to the left,   */
/* right, bottom, top, near and far clipping planes.                          */

void mperspective(real *restrict M, real l, real r,
                                    real b, real t,
                                    real n, real f)
{
    M[ 0] =  (n + n) / (r - l);
    M[ 1] =  0.0;
    M[ 2] =  0.0;
    M[ 3] =  0.0;
    M[ 4] =  0.0;
    M[ 5] =  (n + n) / (t - b);
    M[ 6] =  0.0;
    M[ 7] =  0.0;
    M[ 8] =  (r + l) / (r - l);
    M[ 9] =  (t + b) / (t - b);
    M[10] =  (n + f) / (n - f);
    M[11] = -1.0;
    M[12] =  0.0;
    M[13] =  0.0;
    M[14] = -2.0 * (f * n) / (f - n);
    M[15] =  0.0;
}

/*----------------------------------------------------------------------------*/
/* Matrix operations                                                          */

/* Compose matrix N with matrix M.                                            */

void mcompose(real *restrict M, const real *restrict N)
{
    real T[16];

    mmultiply(T, M, N);
    mcpy     (M, T);
}

/* Compute the inverse I of matrix M.                                         */

void minvert(real *restrict I, const real *restrict M)
{
    real d, T[16];

    assert(I != M);

    T[ 0] = +(M[ 5] * (M[10] * M[15] - M[11] * M[14]) -
              M[ 9] * (M[ 6] * M[15] - M[ 7] * M[14]) +
              M[13] * (M[ 6] * M[11] - M[ 7] * M[10]));
    T[ 1] = -(M[ 4] * (M[10] * M[15] - M[11] * M[14]) -
              M[ 8] * (M[ 6] * M[15] - M[ 7] * M[14]) +
              M[12] * (M[ 6] * M[11] - M[ 7] * M[10]));
    T[ 2] = +(M[ 4] * (M[ 9] * M[15] - M[11] * M[13]) -
              M[ 8] * (M[ 5] * M[15] - M[ 7] * M[13]) +
              M[12] * (M[ 5] * M[11] - M[ 7] * M[ 9]));
    T[ 3] = -(M[ 4] * (M[ 9] * M[14] - M[10] * M[13]) -
              M[ 8] * (M[ 5] * M[14] - M[ 6] * M[13]) +
              M[12] * (M[ 5] * M[10] - M[ 6] * M[ 9]));

    T[ 4] = -(M[ 1] * (M[10] * M[15] - M[11] * M[14]) -
              M[ 9] * (M[ 2] * M[15] - M[ 3] * M[14]) +
              M[13] * (M[ 2] * M[11] - M[ 3] * M[10]));
    T[ 5] = +(M[ 0] * (M[10] * M[15] - M[11] * M[14]) -
              M[ 8] * (M[ 2] * M[15] - M[ 3] * M[14]) +
              M[12] * (M[ 2] * M[11] - M[ 3] * M[10]));
    T[ 6] = -(M[ 0] * (M[ 9] * M[15] - M[11] * M[13]) -
              M[ 8] * (M[ 1] * M[15] - M[ 3] * M[13]) +
              M[12] * (M[ 1] * M[11] - M[ 3] * M[ 9]));
    T[ 7] = +(M[ 0] * (M[ 9] * M[14] - M[10] * M[13]) -
              M[ 8] * (M[ 1] * M[14] - M[ 2] * M[13]) +
              M[12] * (M[ 1] * M[10] - M[ 2] * M[ 9]));

    T[ 8] = +(M[ 1] * (M[ 6] * M[15] - M[ 7] * M[14]) -
              M[ 5] * (M[ 2] * M[15] - M[ 3] * M[14]) +
              M[13] * (M[ 2] * M[ 7] - M[ 3] * M[ 6]));
    T[ 9] = -(M[ 0] * (M[ 6] * M[15] - M[ 7] * M[14]) -
              M[ 4] * (M[ 2] * M[15] - M[ 3] * M[14]) +
              M[12] * (M[ 2] * M[ 7] - M[ 3] * M[ 6]));
    T[10] = +(M[ 0] * (M[ 5] * M[15] - M[ 7] * M[13]) -
              M[ 4] * (M[ 1] * M[15] - M[ 3] * M[13]) +
              M[12] * (M[ 1] * M[ 7] - M[ 3] * M[ 5]));
    T[11] = -(M[ 0] * (M[ 5] * M[14] - M[ 6] * M[13]) -
              M[ 4] * (M[ 1] * M[14] - M[ 2] * M[13]) +
              M[12] * (M[ 1] * M[ 6] - M[ 2] * M[ 5]));

    T[12] = -(M[ 1] * (M[ 6] * M[11] - M[ 7] * M[10]) -
              M[ 5] * (M[ 2] * M[11] - M[ 3] * M[10]) +
              M[ 9] * (M[ 2] * M[ 7] - M[ 3] * M[ 6]));
    T[13] = +(M[ 0] * (M[ 6] * M[11] - M[ 7] * M[10]) -
              M[ 4] * (M[ 2] * M[11] - M[ 3] * M[10]) +
              M[ 8] * (M[ 2] * M[ 7] - M[ 3] * M[ 6]));
    T[14] = -(M[ 0] * (M[ 5] * M[11] - M[ 7] * M[ 9]) -
              M[ 4] * (M[ 1] * M[11] - M[ 3] * M[ 9]) +
              M[ 8] * (M[ 1] * M[ 7] - M[ 3] * M[ 5]));
    T[15] = +(M[ 0] * (M[ 5] * M[10] - M[ 6] * M[ 9]) -
              M[ 4] * (M[ 1] * M[10] - M[ 2] * M[ 9]) +
              M[ 8] * (M[ 1] * M[ 6] - M[ 2] * M[ 5]));

    d = M[ 0] * T[ 0] + M[ 4] * T[ 4] + M[ 8] * T[ 8] + M[12] * T[12];

    if (fabs(d) > 0.0)
    {
        d = 1.0 / d;
        I[ 0] = T[ 0] * d;
        I[ 1] = T[ 4] * d;
        I[ 2] = T[ 8] * d;
        I[ 3] = T[12] * d;
        I[ 4] = T[ 1] * d;
        I[ 5] = T[ 5] * d;
        I[ 6] = T[ 9] * d;
        I[ 7] = T[13] * d;
        I[ 8] = T[ 2] * d;
        I[ 9] = T[ 6] * d;
        I[10] = T[10] * d;
        I[11] = T[14] * d;
        I[12] = T[ 3] * d;
        I[13] = T[ 7] * d;
        I[14] = T[11] * d;
        I[15] = T[15] * d;
    }
}

/* Give the transpose T of matrix M.                                          */

void mtranspose(real *restrict T, const real *restrict M)
{
    assert(T != M);

    T[ 0] = M[ 0]; T[ 4] = M[ 1]; T[ 8] = M[ 2]; T[12] = M[ 3];
    T[ 1] = M[ 4]; T[ 5] = M[ 5]; T[ 9] = M[ 6]; T[13] = M[ 7];
    T[ 2] = M[ 8]; T[ 6] = M[ 9]; T[10] = M[10]; T[14] = M[11];
    T[ 3] = M[12]; T[ 7] = M[13]; T[11] = M[14]; T[15] = M[15];
}

/* Multiply matrices A and B.                                                 */

void mmultiply(real *restrict M, const real *restrict A,
                                 const real *restrict B)
{
    assert(M != A);
    assert(M != B);

    M[ 0] = A[ 0] * B[ 0] + A[ 4] * B[ 1] + A[ 8] * B[ 2] + A[12] * B[ 3];
    M[ 1] = A[ 1] * B[ 0] + A[ 5] * B[ 1] + A[ 9] * B[ 2] + A[13] * B[ 3];
    M[ 2] = A[ 2] * B[ 0] + A[ 6] * B[ 1] + A[10] * B[ 2] + A[14] * B[ 3];
    M[ 3] = A[ 3] * B[ 0] + A[ 7] * B[ 1] + A[11] * B[ 2] + A[15] * B[ 3];

    M[ 4] = A[ 0] * B[ 4] + A[ 4] * B[ 5] + A[ 8] * B[ 6] + A[12] * B[ 7];
    M[ 5] = A[ 1] * B[ 4] + A[ 5] * B[ 5] + A[ 9] * B[ 6] + A[13] * B[ 7];
    M[ 6] = A[ 2] * B[ 4] + A[ 6] * B[ 5] + A[10] * B[ 6] + A[14] * B[ 7];
    M[ 7] = A[ 3] * B[ 4] + A[ 7] * B[ 5] + A[11] * B[ 6] + A[15] * B[ 7];

    M[ 8] = A[ 0] * B[ 8] + A[ 4] * B[ 9] + A[ 8] * B[10] + A[12] * B[11];
    M[ 9] = A[ 1] * B[ 8] + A[ 5] * B[ 9] + A[ 9] * B[10] + A[13] * B[11];
    M[10] = A[ 2] * B[ 8] + A[ 6] * B[ 9] + A[10] * B[10] + A[14] * B[11];
    M[11] = A[ 3] * B[ 8] + A[ 7] * B[ 9] + A[11] * B[10] + A[15] * B[11];

    M[12] = A[ 0] * B[12] + A[ 4] * B[13] + A[ 8] * B[14] + A[12] * B[15];
    M[13] = A[ 1] * B[12] + A[ 5] * B[13] + A[ 9] * B[14] + A[13] * B[15];
    M[14] = A[ 2] * B[12] + A[ 6] * B[13] + A[10] * B[14] + A[14] * B[15];
    M[15] = A[ 3] * B[12] + A[ 7] * B[13] + A[11] * B[14] + A[15] * B[15];
}

/* Orthonormalize the rotation of matrix M, preserving the Z direction.       */

void morthonormalize(real *restrict O, const real *restrict M)
{
    assert(O != M);

    vcrs(O + 0, M + 4, M + 8);
    vcrs(O + 4, M + 8, O + 0);

    vnormalize(O + 8, M + 8);
    vnormalize(O + 4, O + 4);
    vnormalize(O + 0, O + 0);

    O[ 3] = M[ 3];
    O[ 7] = M[ 7];
    O[11] = M[11];
    O[12] = M[12];
    O[13] = M[13];
    O[14] = M[14];
    O[15] = M[15];
}

/*----------------------------------------------------------------------------*/

/* Compute the matrix M given by quaternion q.                                */

void mquaternion(real *restrict M, const real *restrict q)
{
    real t[4];

    qnormalize(t, q);

    vquaternionx(M + 0, t);
    vquaterniony(M + 4, t);
    vquaternionz(M + 8, t);

    M[ 3] = 0.0;
    M[ 7] = 0.0;
    M[11] = 0.0;
    M[12] = 0.0;
    M[13] = 0.0;
    M[14] = 0.0;
    M[15] = 1.0;
}

/* Compute the matrix M given by the Euler angles e.                          */

void meuler(real *restrict M, const real *restrict e)
{
    const real cx = cos(e[0]), sx = sin(e[0]);
    const real cy = cos(e[1]), sy = sin(e[1]);
    const real cz = cos(e[2]), sz = sin(e[2]);

    M[ 0] =  cy * cz - sx * sy * sz;
    M[ 1] =  cy * sz + sx * sy * cz;
    M[ 2] = -cx * sy;
    M[ 3] = 0.0;
    M[ 4] = -cx * sz;
    M[ 5] =  cx * cz;
    M[ 6] =  sx;
    M[ 7] = 0.0;
    M[ 8] =  sy * cz + sx * cy * sz;
    M[ 9] =  sy * sz - sx * cy * cz;
    M[10] =  cx * cy;
    M[11] = 0.0;
    M[12] = 0.0;
    M[13] = 0.0;
    M[14] = 0.0;
    M[15] = 1.0;
}

/*----------------------------------------------------------------------------*/

/* Compute the quaternion q given by the Euler angles e.                      */

void qeuler(real *restrict q, const real *restrict e)
{
    const real cx = cos(e[0] * 0.5), sx = sin(e[0] * 0.5);
    const real cy = cos(e[1] * 0.5), sy = sin(e[1] * 0.5);
    const real cz = cos(e[2] * 0.5), sz = sin(e[2] * 0.5);

    q[0] = sx * cy * cz - cx * sy * sz;
    q[1] = cx * sy * cz + sx * cy * sz;
    q[2] = sx * sy * cz + cx * cy * sz;
    q[3] = cx * cy * cz - sx * sy * sz;

    qnormalize(q, q);
}

/* Compute the quaternion q given by rotation matrix M.                       */

void qmatrix(real *restrict q, const real *restrict M)
{
    if (1.0 + M[0] + M[5] + M[10] > 0.0)
    {
        const real s = 0.5 / sqrt(1.0 + M[0] + M[5] + M[10]);

        q[2] = (M[1] - M[4]) * s;
        q[1] = (M[8] - M[2]) * s;
        q[0] = (M[6] - M[9]) * s;
        q[3] =         0.25  / s;
    }
    else if (M[0] > M[5] && M[0] > M[10])
    {
        const real s = 2.0 * sqrt(1.0 + M[0] - M[5] - M[10]);

        q[1] = (M[1] + M[4]) / s;
        q[2] = (M[8] + M[2]) / s;
        q[3] = (M[6] - M[9]) / s;
        q[0] =         0.25  * s;
    }
    else if (M[5] > M[10])
    {
        const real s = 2.0 * sqrt(1.0 + M[5] - M[0] - M[10]);

        q[0] = (M[1] + M[4]) / s;
        q[3] = (M[8] - M[2]) / s;
        q[2] = (M[6] + M[9]) / s;
        q[1] =         0.25  * s;
    }
    else
    {
        const real s = 2.0 * sqrt(1.0 + M[10] - M[0] - M[5]);

        q[3] = (M[1] - M[4]) / s;
        q[0] = (M[8] + M[2]) / s;
        q[1] = (M[6] + M[9]) / s;
        q[2] =         0.25  * s;
    }
}

/*----------------------------------------------------------------------------*/

/* Extract a set of Euler angles from quaternion q.                           */

void equaternion(real *restrict e, const real *restrict q)
{
    real M[16];

    mquaternion(M, q);
    ematrix    (e, M);
}

/* Extract a set of Euler angles from rotation matrix M.                      */

void ematrix(real *restrict e, const real *restrict M)
{
    real sx = M[6];
    real cx = sqrt(1.0 - sx * sx);

    if (cx > 0.0)
    {
        real cy =  M[10] / cx;
        real sy = -M[ 2] / cx;
        real cz =  M[ 5] / cx;
        real sz = -M[ 4] / cx;

        e[0] = atan2(sx, cx);
        e[1] = atan2(sy, cy);
        e[2] = atan2(sz, cz);
    }
    else
    {
        e[0] = 0.0;
        e[1] = 0.0;
        e[2] = 0.0;
    }
}
