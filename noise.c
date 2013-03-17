/* Copyright (c) 2007 Robert Kooima                                           */
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

#include <math.h>
#include <float.h>
#include "noise.h"

/* This implementation provides Perlin Simplex Noise. It is a C translation   */
/* of Java given by Ken Perlin in Appendix B of "Real-Time Shading SIGGRAPH   */
/* Course Notes 2001, Chapter 2: Noise Hardware."                             */

/*----------------------------------------------------------------------------*/

struct state
{
    int    A[3];
    int    i;
    int    j;
    int    k;
    double u;
    double v;
    double w;
};

/*----------------------------------------------------------------------------*/

static int c(int N, int B)
{
    return N >> B & 1;
}

static int b(int i, int j, int k, int B)
{
    static const int T[8] = { 0x15, 0x38, 0x32, 0x2c, 0x0d, 0x13, 0x07, 0x2a }; 

    return T[c(i, B) << 2 |
             c(j, B) << 1 |
             c(k, B)];
}

static int shuffle(int i, int j, int k)
{
    return (b(i, j, k, 0) + b(j, k, i, 1) + b(k, i, j, 2) + b(i, j, k, 3) +
            b(j, k, i, 4) + b(k, i, j, 5) + b(i, j, k, 6) + b(j, k, i, 7));
}

double K(int A[3], const int i[3], const double v[3], int a)
{
    /* Compute skewed input points. */

    double s = (A[0] + A[1] + A[2]) / 6.0;
    double t;

    double x = v[0] - A[0] + s;
    double y = v[1] - A[1] + s;
    double z = v[2] - A[2] + s;

    /* Compute the index of the pseudo-random gradient. */

    int h = shuffle(i[0] + A[0], i[1] + A[1], i[2] + A[2]);

    /* Iterate to the next simplex vertex. */

    A[a]++;

    /* Find the contribution of this simplex vertex. */

    if ((t = 0.6 - x * x - y * y - z * z) >= 0) 
    {
        /* Isolate the bits of the gradient index. */

        int b5 = h >> 5 & 1;
        int b4 = h >> 4 & 1;
        int b3 = h >> 3 & 1;
        int b2 = h >> 2 & 1;
        int b  = h      & 3; 

        /* Compute the gradient magnitude using the 3 lower bits. */

        double p = (b == 1) ? x : ((b == 2) ? y : z);
        double q = (b == 1) ? y : ((b == 2) ? z : x);
        double r = (b == 1) ? z : ((b == 2) ? x : y); 
        double m;
        
        /* Compute the gradient octant using the 3 upper bits. */

        p = (b5 == (     b3)) ? -p : p;
        q = (b5 == (b4     )) ? -q : q;
        r = (b5 != (b4 ^ b3)) ? -r : r; 

        /* Sum the gradient components giving magnitude. */

        m = p + ((b == 0) ? q + r : ((b2 == 0) ? q : r)); 

        /* Evaluate the spherical kernel giving this vertex's contribution. */

        return 8 * (t * t * t * t) * m;
    }
    return 0; 
} 

/*----------------------------------------------------------------------------*/

double noise_sample(double x, double y, double z)
{
    int hi, A[3] = { 0, 0, 0 };
    int lo, i[3];
    double  v[3];
    double  s;
    
    /* Find the integer coordinates (i, j, k) in simplex grid skewed space. */

    s = (x + y + z) / 3.0; 

    i[0] = (int) floor(x + s);
    i[1] = (int) floor(y + s);
    i[2] = (int) floor(z + s); 

    /* Find the coordinates relative to the unskewed cube. */

    s = (i[0] + i[1] + i[2]) / 6.0;

    v[0] = x - i[0] + s;
    v[1] = y - i[1] + s;
    v[2] = z - i[2] + s; 

    /* Determine which simplex contains the input point. */

    hi = (v[0] >= v[2]) ? ((v[0] >= v[1]) ? 0 : 1) : ((v[1] >= v[2]) ? 1 : 2); 
    lo = (v[0] <  v[2]) ? ((v[0] <  v[1]) ? 0 : 1) : ((v[1] <  v[2]) ? 1 : 2); 

    /* Evaluate the contribution of each vertex of the simplex. */

    return K(A, i, v,     hi     )
         + K(A, i, v, 3 - hi - lo)
         + K(A, i, v,          lo)
         + K(A, i, v, 0          ); 
}
 
/*----------------------------------------------------------------------------*/

void noise_buffer(double x, double y, double z,
                  double f, int w, int h, double *v)
{
    /* Fill the given buffer with noise at frequency f.  Note the extrema. */

    double k0 = +DBL_MAX;
    double k1 = -DBL_MAX;
    
    int i;
    int j;

    for (i = 0; i < h; ++i)
        for (j = 0; j < w; ++j)
        {
            double dx = f * (j + 0.5) / w;
            double dy = f * (i + 0.5) / h;

            double k = noise_sample(x + dx, y + dy, z);

            v[i * w + j] = k;

            if (k0 > k) k0 = k;
            if (k1 < k) k1 = k;
        }

    /* Normalize the noise to [-1, 1]. */

    for (i = 0; i < h; ++i)
        for (j = 0; j < w; ++j)
            v[i * w + j] = 2.0 * (v[i * w + j] - k0) / (k1 - k0) - 1.0;
}

/*----------------------------------------------------------------------------*/
