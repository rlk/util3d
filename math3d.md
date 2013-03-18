# math3d

This module implements a right-handed 3D mathematics library supporting three-component vectors, four-by-four matrices, quaternions, and Euler angles. These values are represented using ordinary C arrays of type `real`.

By default, `real` is `typedef`-ed to `double`. However, `float` is used instead if the preprocessor symbol `CONFIG_MATH3D_FLOAT` is defined during inclusion of `math3d.h` and compilation of `math3d.c`.

-   [math3d.c](math3d.c)
-   [math3d.h](math3d.h)

Array layout is as follows. Note that the quaternion scalar part follows the vector part, allowing vector functions to operate upon the vector part of a quaternion. Also, the matrix representation is column-wise, matching the layout expected by OpenGL. Here, *a<sub><small>ij</small></sub>* is the element at row *i*, column *j*.

<table style="text-align:center;border:none">
  <tr style="background:inherit;border:none">
    <td style="text-align:right;border:none">Vector:</td>
    <td><i>x</i></td>
    <td><i>y</i></td>
    <td><i>z</i></td>
  </tr>
  <tr style="background:inherit;border:none">
    <td style="text-align:right;border:none">Quaternion:</td>
    <td><i>x</i></td>
    <td><i>y</i></td>
    <td><i>z</i></td>
    <td><i>s</i></td>
  </tr>
  <tr style="background:inherit;border:none">
    <td style="text-align:right;border:none">Euler:</td>
    <td><i>&alpha;</i></td>
    <td><i>&beta;</i></td>
    <td><i>&gamma;</i></td>
  </tr>
  <tr style="background:inherit;border:none">
    <td style="text-align:right;border:none">Matrix:</td>
    <td><i>a<sub><small>00</small></sub></i></td>
    <td><i>a<sub><small>10</small></sub></i></td>
    <td><i>a<sub><small>20</small></sub></i></td>
    <td><i>a<sub><small>30</small></sub></i></td>
    <td><i>a<sub><small>01</small></sub></i></td>
    <td><i>a<sub><small>11</small></sub></i></td>
    <td><i>a<sub><small>21</small></sub></i></td>
    <td><i>a<sub><small>31</small></sub></i></td>
    <td><i>a<sub><small>02</small></sub></i></td>
    <td><i>a<sub><small>12</small></sub></i></td>
    <td><i>a<sub><small>22</small></sub></i></td>
    <td><i>a<sub><small>32</small></sub></i></td>
    <td><i>a<sub><small>03</small></sub></i></td>
    <td><i>a<sub><small>13</small></sub></i></td>
    <td><i>a<sub><small>23</small></sub></i></td>
    <td><i>a<sub><small>33</small></sub></i></td>
  </tr>
</table>

Euler angle *&alpha;* gives rotation about the *x* axis, *&beta;* about the *y* axis, and *&gamma;* about the *z* axis. (Please beware the difference between the *y* and the Greek gamma *&gamma;*.) These rotations are concatenated in *zxy*-order, as this is a common standard for interactive 3D. Angle *&alpha;* ranges from &minus;&frac12; *&pi;* to &frac12; *&pi;* while *&beta;* and *&gamma;* range from &minus;*&pi;* to *&pi;*.

The signatures of all functions follow. The first argument to most functions is a pointer to an array to receive the result. Subsequent arguments are constant pointers to operands, or real values. Pointers are declared `restrict` when two or more arguments may not reasonably refer to the same array. Light-weight functions are declared `static inline` in the header while heavy functions are exported from the C module.

Rotation arguments and Euler angles are given in radians.

- `real radians(real a)`

    Convert angle `a` from degrees to radians.

- `real degrees(real a)`

    Convert angle `a` from radians to degrees.

## Vector operations

- `real vlen(const real *a)`

    Length. `||a||`

- `void vcpy(real *restrict a, const real *restrict b)`

    Copy. `a = b`

- `void vneg(real *a, const real *b)`

    Negation. `a = -b`

- `real vdot(const real *a, const real *b)`

    Dot product. `a` &sdot; `b`

- `void vmul(real *a, const real *b, real k)`

    Scalar product. `a = b * k`

- `void vcrs(real *restrict a, const real *restrict b, const real *restrict c)`

    Cross product. `a = b `&times;` c`

- `void vadd(real *a, const real *b, const real *c)`

    Addition. `a = b + c`

- `void vsub(real *a, const real *b, const real *c)`

    Subtraction. `a = b - c`

- `void vmad(real *a, const real *b, const real *c, real k)`

    Scalar-multiply-add. `a = b + c * k`

- `void vproject(real *a, const real *b, const real *c)`

    Project vector `c` parallel with vector `b`.

- `void vnormalize(real *a, const real *b)`

    Normalization. `a = b / ||b||`

- `void vtransform(real *restrict a, const real *restrict M, const real *restrict b)`

    Transform vector `b` by matrix `M` (neglecting translation.)

- `void ptransform(real *restrict a, const real *restrict M, const real *restrict b)`

    Transform position `b` by matrix `M` (including translation.)

- `void vslerp(real *a, const real *b, const real *c, real t)`

    Compute the spherical linear interpolation of vectors `b` and `c` at a time `t` between zero and one.

## Quaternion operations

- `real qdot(const real *a, const real *b)`

    Dot product. `a `&sdot;` b`

- `void qadd(real *a, const real *b, const real *c)`

    Add. `a = b + c`

- `void qscale(real *a, const real *b, real k)`

    Scalar multiply. `a = b * k`

- `void qconjugate(real *a, const real *b)`

    Conjugate.

- `void qinvert(real *a, const real *b)`

    Inverse.

- `void qnormalize(real *a, const real *b)`

    Normalization.

- `void qpow(real *a, const real *b, real h);`

    Compute quaternion `b` raised to the power `h`.

- `void qexp(real *a, const real *b);`

    Compute *e* raised to the power quaternion `b`.

- `void qlog(real *a, const real *b);`

    Compute the natural logarithm of quaternion `b`.

- `void qsign(real *a, const real *b, const real *c)`

    Provide quaternion `c` with the same "sign" as quaternion `b`, selecting `a = c` or `a = -c` such that `a` &sdot; `b` &ge; 0. This ensures that interpolation from `b` to `c` goes "the short way around."

- `void qslerp(real *a, const real *b, const real *c, real t)`

    Compute the spherical linear interpolation of quaternions `b` and `c` at a time `t` between zero and one.

- `void qsquad(real *a, const real *b, const real *c, const real *d, const real *e, real t)`

    Compute the spherical quadrangle interpolation of quaternions `c` and `d` at time `t` along the spline through `b`, `c`, `d`, and `e`.

- `void qrotate(real *restrict q, const real *restrict v, real a)`

    Compute the quaternion giving rotation about vector `v` through angle `a`.

- `void qmultiply(real *restrict a, const real *restrict b, const real *restrict c)`

    Multiply quaternions `b` and `c`.

## Matrix operations

- `void minvert(real *restrict I, const real *restrict M)`

    Compute the inverse of matrix `M`.

- `void mtranspose(real *restrict T, const real *restrict M)`

    Return the transpose of matrix `M`.

- `void mmultiply(real *restrict M, const real *restrict A, const real *restrict B)`

    Multiply matrices `A` and `B`.

- `void morthonormalize(real *restrict O, const real *restrict M)`

    Compute the orthonormalization of the rotation of matrix `M`, preserving the direction of the `z` axis, and the non-rotation elements of `M`.

## Transformation matrices

- `void midentity(real *restrict M)`

    The identity.

- `void mrotatex(real *restrict M, real a)`

    Rotation about the *x* axis through angle `a`.

- `void mrotatey(real *restrict M, real a)`

    Rotation about the *y* axis through angle `a`.

- `void mrotatez(real *restrict M, real a)`

    Rotation about the *z* axis through angle `a`.

- `void mrotate(real *restrict M, const real *restrict v, real a)`

    Rotation about vector `v` through angle `a`.

- `void mtranslate(real *restrict M, const real *restrict v)`

    Translation along vector `v`.

- `void mscale(real *restrict M, const real *restrict v)`

    Scaling by vector `v`.

- `void mbasis(real *restrict M, const real *restrict x, const real *restrict y, const real *restrict z)`

    The basis given by vectors `x`, `y`, and `z`.

- `void morthogonal(real *restrict M,real l, real r,real b, real t,real n, real f)`

    The orthogonal projection with left, right, bottom, top, near, and far clipping plane distances.

- `void mperspective(real *restrict M,real l, real r,real b, real t,real n, real f)`

    The perspective projection with left, right, bottom, top, near, and far clipping plane distances.

## Rotation conversions

### To matrix

- `void mquaternion(real *restrict M, const real *restrict q)`

    Compute the rotation matrix given by quaternion `q`.

- `void meuler(real *restrict M, const real *restrict e)`

    Compute the rotation matrix given by Euler angles `e`.

### To quaternion

- `void qmatrix(real *restrict q, const real *restrict M)`

    Compute the quaternion given by rotation matrix `M`.

- `void qeuler(real *restrict q, const real *restrict e)`

    Compute the quaternion given by Euler angles `e`.

### To Euler

- `void ematrix(real *restrict e, const real *restrict M)`

    Extract a set of Euler angles from rotation matrix `M`.

- `void equaternion(real *restrict e, const real *restrict q)`

    Extract a set of Euler angles from quaternion `q`.
