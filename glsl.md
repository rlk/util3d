# glsl

This API consists of an extremely simple structure of OpenGL objects and a small set of functions that operate upon it.

-   [glsl.c](glsl.c)
-   [glsl.h](glsl.h)

The structure contains a program object, vertex and fragment shader objects, and character pointers used to cache shader file names, if provided.

    struct glsl
    {
        GLuint program;

        GLuint vert_shader;
        GLuint frag_shader;

        char  *vert_filename;
        char  *frag_filename;
    };

    typedef struct glsl glsl;

The functions that initialize and release these resources are as follows.

- `GLboolean glsl_source(glsl *G, const char *vert_str, int vert_len, const char *frag_str, int frag_len);`

    Initialize a GLSL program object using the given vertex shader source string `vert_str` and fragment shader source string `frag_str`. If these strings are *not* null-terminated then `vert_len` and `frag_len` give their lengths. If they are null-terminated, these lengths may be given as -1. In the event of an error in shader compilation or program linking, the relevant log contents are printed to the standard error stream and `GL_FALSE` is returned.

- `GLboolean glsl_create(glsl *G, const char *vert_filename, const char *frag_filename);`

    Initialize a GLSL program object using the named vertex shader source file `vert_filename` and fragment shader source file `frag_filename`. These file names are cached within the GLSL structure. On error, relevant log contents are printed to the standard error stream and `GL_FALSE` is returned.

- `void glsl_delete(glsl *G);`

    Release the shader objects, program object, and cached file names held by the given GLSL structure.

Once initialized, the `program` entry of the `glsl` structure may be used normally.

    glsl G;

    if (glsl_create(&G, "phong.vert", "phong.frag"))
        glUseProgram(G.program);

A few convenience functions are also provided.

- `GLint glsl_uniform(GLuint, const char *, ...);`

    Return the location of a GLSL uniform. This is a more powerful form of the `glGetUniformLocation` function taking a variable arguments list that enables the construction of array and structure uniform names in the style of `printf`. For example, to initialize an array of sampler uniforms in a loop:

        for (int i = 0; i < 16; i++)
            glUniform1i(glsl_uniform(G.program, "image[%d]", i), i);

- `GLboolean glsl_reload(glsl *G);`

    If the given GLSL structure was initialized using `glsl_create` and the shader file names are cached then `glsl_reload` flushes and reloadss the current shader and program objects. This allows modified shader files to be activated without stopping and restarting the running application.

