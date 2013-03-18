# image

This module implements a basic image I/O library supporting the reading and writing of PNG and JPEG images. It provides a uniform simplified front-end to [libpng](http://www.libpng.org/), [libtiff](http://www.remotesensing.org/libtiff/), [libjpeg](http://www.ijg.org/), and [OpenEXR](http://www.openexr.com/). Usable color depths and channel counts vary depending on format.

-   [image.c](image.c)
-   [image.h](image.h)

## Compilation

To use this module, simply link it with your own code and the supporting libraries for all necessary image formats.

    cc -o program program.c image.c -lpng -ltiff -ljpeg -lIlmImf -lz -lm

PNG, TIFF, JPEG, and EXR support may be omitted as desired with the definition of `CONFIG_NO_PNG`, `CONFIG_NO_TIF`, `CONFIG_NO_JPG`, or `CONFIG_NO_EXR`. For example, to build with PNG support only:

    cc -DCONFIG_NO_TIF -DCONFIG_NO_JPG -DCONFIG_NO_EXR -o program program.c image.c -lpng -lz -lm

## Image I/O

- `void *image_read(const char *name, int *w, int *h, int *c, int *b)`

    Read the image file named `name`. The return value is a newly-allocated buffer containing the image data. Arguments `w`, `h`, `c`, and `b`  point to integers that receive the width, height, channel count, and bytes-per-channel of the image. Return null upon failure.

- `void image_write(const char *name, int w, int h, int c, int b, const void *p)`

    Write the image file named `name`. Argument `p` points to the buffer of image data. Arguments `w`, `h`, `c`, and `b` give the width, height, channel count, and bytes-per-channel of the image.

Both the reader and writer functions examine the extension of the given name to determine the format of the file.

## Format-specific I/O

These functions ignore the extension of the given name string.

- `void *image_read_png(const char *name, int *w, int *h, int *c, int *b)`
- `void image_write_png(const char *name, int w, int h, int c, int b, const void *p)`

    Read or write image file `name`, forcing the file type to PNG.

- `void *image_read_jpg(const char *name, int *w, int *h, int *c, int *b)`
- `void image_write_jpg(const char *name, int w, int h, int c, int b, const void *p)`

    Read or write image file `name`, forcing the file type to JPEG.

- `void *image_read_exr(const char *name, int *w, int *h, int *c, int *b)`
- `void image_write_exr(const char *name, int w, int h, int c, int b, const void *p)`

    Read or write image file `name`, forcing the file type to OpenEXR.

- `void *image_read_tif(const char *name, int *w, int *h, int *c, int *b, int i)`
- `void image_write_tif(const char *name, int w, int h, int c, int b, int n, void **p)`

    Read or write image file `name`, forcing the file type to TIFF. The `i` argument to `image_read_tif` selects the *i*th page of the TIFF for reading. The `n` argument to `image_write_tif` gives the number of image buffer pointers in array `p` for writing. These enable the reading and writing of multi-page TIFF files.

## Utilities

- `void image_flip(int w, int h, int c, int b, void *p)`

    Flip the given image buffer vertically. Arguments `w`, `h`, `c`, and `b` give the width, height, channel count, and bytes-per-channel of the image, and `p` points to the pixel buffer. This function may be used to rectify disagreement over whether the image origin lies at the upper left or the lower left.

- `GLenum image_internal_form(int c, int b)`

    Return an OpenGL internal texture format enumerator appropriate for an image with `c` channels and `b` bytes per channel: `GL_LUMINANCE`, `GL_LUMINANCE_ALPHA`, `GL_RGB`, `GL_RGBA`, `GL_LUMINANCE16`, `GL_LUMINANCE_ALPHA16`, `GL_RGB16`, or `GL_RGBA16`.

- `GLenum image_external_form(int c)`

    Return an OpenGL external pixel format enumerator appropriate for an image with `c` channels: `GL_LUMINANCE`, `GL_LUMINANCE_ALPHA`, `GL_RGB`, or `GL_RGBA`.

- `GLenum image_external_type(int b)`

    Return an OpenGL external pixel type enumerator appropriate for an image with `b` bytes per channel: `GL_UNSIGNED_BYTE` or `GL_UNSIGNED_SHORT`.

## Example

The following code fragment demonstrates the common case of loading an image file to an OpenGL texture. Texture mappings often assume an origin at the lower left of the image, so the image is flipped after reading. The buffer is freed after uploading, as OpenGL has cached the contents internally.

    int w, h, c, b;

    void *p = image_read(name, &w, &h, &c, &b);

    int i = image_internal_form(c, b);
    int e = image_external_form(c);
    int t = image_external_type(b);

    image_flip(w, h, c, b, p);

    glTexImage2D(GL_TEXTURE_2D, 0, i, w, h, 0, e, t, p);

    free(p);
