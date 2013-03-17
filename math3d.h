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

#ifndef UTIL3D_MATH3D_H
#define UTIL3D_MATH3D_H

#ifdef __cplusplus
extern "C" {
#endif

/* When C99 is unavailable, GCC and MSVC allow __inline and __restrict.       */

#if __STDC_VERSION__ < 199901L
#define inline   __inline
#define restrict __restrict
#endif

/*----------------------------------------------------------------------------*/

#ifdef CONFIG_MATH3D_FLOAT
typedef float  real;
#else
typedef double real;
#endif

/*----------------------------------------------------------------------------*/

static inline real radians(real a)
{
    return a *  0.017453292519943295769236907684886;
}

static inline real degrees(real a)
{
    return a * 57.295779513082320876798154814105170;
}

static inline real lerp(real a, real b, real t)
{
    return (1.0 - t) * a + t * b;
}

/*----------------------------------------------------------------------------*/
/* Vector operations                                                          */

static inline real vdot(const real *a, const real *b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static inline real vlen(const real *a)
{
    return sqrt(vdot(a, a));
}

static inline void vcpy(real *restrict a, const real *restrict b)
{
    a[0] = b[0];
    a[1] = b[1];
    a[2] = b[2];
}

static inline void vneg(real *a, const real *b)
{
    a[0] = -b[0];
    a[1] = -b[1];
    a[2] = -b[2];
}

static inline void vmul(real *a, const real *b, real k)
{
    a[0] = b[0] * k;
    a[1] = b[1] * k;
    a[2] = b[2] * k;
}

static inline void vcrs(real *restrict a, const real *restrict b,
                                          const real *restrict c)
{
    a[0] = b[1] * c[2] - b[2] * c[1];
    a[1] = b[2] * c[0] - b[0] * c[2];
    a[2] = b[0] * c[1] - b[1] * c[0];
}

static inline void vadd(real *a, const real *b, const real *c)
{
    a[0] = b[0] + c[0];
    a[1] = b[1] + c[1];
    a[2] = b[2] + c[2];
}

static inline void vsub(real *a, const real *b, const real *c)
{
    a[0] = b[0] - c[0];
    a[1] = b[1] - c[1];
    a[2] = b[2] - c[2];
}

static inline void vmad(real *a, const real *b, const real *c, real k)
{
    a[0] = b[0] + c[0] * k;
    a[1] = b[1] + c[1] * k;
    a[2] = b[2] + c[2] * k;
}

static inline void vproject(real *a, const real *b,  const real *c)
{
    const real k = vdot(b, c);

    a[0] = b[0] - c[0] * k;
    a[1] = b[1] - c[1] * k;
    a[2] = b[2] - c[2] * k;
}

static inline void vnormalize(real *a, const real *b)
{
    const real k = 1.0 / vlen(b);

    a[0] = b[0] * k;
    a[1] = b[1] * k;
    a[2] = b[2] * k;
}

void vtransform(real *restrict, const real *restrict, const real *restrict);
void ptransform(real *restrict, const real *restrict, const real *restrict);
void wtransform(real *restrict, const real *restrict, const real *restrict);
void vslerp    (real *,         const real *,         const real *, real);

/*----------------------------------------------------------------------------*/
/* Quaternion operations                                                      */

static inline real qdot(const real *a, const real *b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

static inline void qcpy(real *restrict a, const real *restrict b)
{
    a[0] = b[0];
    a[1] = b[1];
    a[2] = b[2];
    a[3] = b[3];
}

static inline void qadd(real *a, const real *b, const real *c)
{
    a[0] = b[0] + c[0];
    a[1] = b[1] + c[1];
    a[2] = b[2] + c[2];
    a[3] = b[3] + c[3];
}

static inline void qscale(real *a, const real *b, real k)
{
    a[0] = b[0] * k;
    a[1] = b[1] * k;
    a[2] = b[2] * k;
    a[3] = b[3] * k;
}

static inline void qconjugate(real *a, const real *b)
{
    a[0] = -b[0];
    a[1] = -b[1];
    a[2] = -b[2];
    a[3] =  b[3];
}

static inline void qinvert(real *a, const real *b)
{
    const real k = 1.0 / qdot(b, b);

    a[0] = -b[0] * k;
    a[1] = -b[1] * k;
    a[2] = -b[2] * k;
    a[3] =  b[3] * k;
}

static inline void qnormalize(real *a, const real *b)
{
    const real k = 1.0 / sqrt(qdot(b, b));

    a[0] = b[0] * k;
    a[1] = b[1] * k;
    a[2] = b[2] * k;
    a[3] = b[3] * k;
}

static inline void qsign(real *a, const real *b, const real *c)
{
    if (qdot(b, c) > 0.0)
    {
        a[0] =  c[0];
        a[1] =  c[1];
        a[2] =  c[2];
        a[3] =  c[3];
    }
    else
    {
        a[0] = -c[0];
        a[1] = -c[1];
        a[2] = -c[2];
        a[3] = -c[3];
    }
}

void qpow  (real *, const real *, real);
void qexp  (real *, const real *);
void qlog  (real *, const real *);
void qslerp(real *, const real *, const real *, real);
void qsquad(real *, const real *, const real *,
                    const real *, const real *, real);

void qrotate  (real *restrict, const real *restrict, real);
void qmultiply(real *restrict, const real *restrict, const real *restrict);

/*----------------------------------------------------------------------------*/
/* Tranformation matrices                                                     */

static inline void mcpy(real *M, const real *N)
{
    M[ 0] = N[ 0];
    M[ 1] = N[ 1];
    M[ 2] = N[ 2];
    M[ 3] = N[ 3];
    M[ 4] = N[ 4];
    M[ 5] = N[ 5];
    M[ 6] = N[ 6];
    M[ 7] = N[ 7];
    M[ 8] = N[ 8];
    M[ 9] = N[ 9];
    M[10] = N[10];
    M[11] = N[11];
    M[12] = N[12];
    M[13] = N[13];
    M[14] = N[14];
    M[15] = N[15];
}

void midentity   (real *restrict);
void mrotatex    (real *restrict, real);
void mrotatey    (real *restrict, real);
void mrotatez    (real *restrict, real);
void mrotate     (real *restrict, const real *restrict, real);
void mtranslate  (real *restrict, const real *restrict);
void mscale      (real *restrict, const real *restrict);
void mbasis      (real *restrict, const real *restrict,
                                  const real *restrict,
                                  const real *restrict);
void morthogonal (real *restrict, real, real, real, real, real, real);
void mperspective(real *restrict, real, real, real, real, real, real);

/*----------------------------------------------------------------------------*/
/* Matrix operations                                                          */

void mcompose  (real *restrict, const real *restrict);
void minvert   (real *restrict, const real *restrict);
void mtranspose(real *restrict, const real *restrict);
void mmultiply (real *restrict, const real *restrict, const real *restrict);

void morthonormalize(real *restrict, const real *restrict);

/*----------------------------------------------------------------------------*/
/* Rotation conversions                                                       */

void mquaternion(real *restrict, const real *restrict);
void meuler     (real *restrict, const real *restrict);

void qeuler     (real *restrict, const real *restrict);
void qmatrix    (real *restrict, const real *restrict);

void equaternion(real *restrict, const real *restrict);
void ematrix    (real *restrict, const real *restrict);

static inline void vquaternionx(real *v, const real *restrict q)
{
    v[0] = 1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]);
    v[1] =       2.0 * (q[0] * q[1] + q[2] * q[3]);
    v[2] =       2.0 * (q[0] * q[2] - q[1] * q[3]);
}

static inline void vquaterniony(real *v, const real *restrict q)
{
    v[0] =       2.0 * (q[0] * q[1] - q[2] * q[3]);
    v[1] = 1.0 - 2.0 * (q[0] * q[0] + q[2] * q[2]);
    v[2] =       2.0 * (q[1] * q[2] + q[0] * q[3]);
}

static inline void vquaternionz(real *v, const real *restrict q)
{
    v[0] =       2.0 * (q[0] * q[2] + q[1] * q[3]);
    v[1] =       2.0 * (q[1] * q[2] - q[0] * q[3]);
    v[2] = 1.0 - 2.0 * (q[0] * q[0] + q[1] * q[1]);
}

/*----------------------------------------------------------------------------*/

#define vprint(v) \
    printf("%8.3f%8.3f%8.3f\n",      (v)[0], (v)[1], (v)[2])
#define qprint(q) \
    printf("%8.3f%8.3f%8.3f%8.3f\n", (q)[0], (q)[1], (q)[2], (q)[3])
#define mprint(M) \
    printf("%8.3f%8.3f%8.3f%8.3f\n"                                      \
           "%8.3f%8.3f%8.3f%8.3f\n"                                      \
           "%8.3f%8.3f%8.3f%8.3f\n"                                      \
           "%8.3f%8.3f%8.3f%8.3f\n", (M)[ 0], (M)[ 4], (M)[ 8], (M)[12], \
                                     (M)[ 1], (M)[ 5], (M)[ 9], (M)[13], \
                                     (M)[ 2], (M)[ 6], (M)[10], (M)[14], \
                                     (M)[ 3], (M)[ 7], (M)[11], (M)[15])

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif
#endif
