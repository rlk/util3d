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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <GL/glew.h>

#include "image.h"
#include "demo.h"
#include "glsl.h"

/*----------------------------------------------------------------------------*/

static int         demo_mode;
static demo_init_f demo_init;
static demo_tilt_f demo_tilt;
static demo_quit_f demo_quit;
static demo_step_f demo_step;
static demo_draw_f demo_draw;

static GLuint clear_vert;
static GLuint clear_frag;
static GLuint clear;

static struct timeval t0;
static struct timeval t1;

static double dt;

/*----------------------------------------------------------------------------*/

static int      last_time;

/* Camera state.                                                              */

static GLfloat  position[4] = {   0.0,  0.0,  5.0,  1.0 };
static GLfloat  rotation[2] = {   0.0,  0.0             };
static GLfloat  velocity[3] = {   0.0,  0.0,  0.0       };
static GLfloat  light[2]    = { -60.0, 30.0             };
static GLfloat  point[4]    = {   0.0,  0.0,  0.0,  0.0 };
static GLfloat  zoom        = 0.5;

/* Camera state at the beginning of a click.                                  */

static int      click_modifiers;
static int      click_button = -1;
static int      click_x;
static int      click_y;
static GLfloat  click_rotation[2];
static GLfloat  click_light[2];
static GLfloat  click_zoom;

const GLfloat *demo_get(int token)
{
    switch (token)
    {
    case DEMO_POSITION: return position;
    case DEMO_ROTATION: return rotation;
    case DEMO_LIGHT:    return light;
    case DEMO_POINT:    return point;
    case DEMO_ZOOM:     return &zoom;
    }
    return 0;
}

/*----------------------------------------------------------------------------*/

/* Write the current view state to the file named by the DEMO_STATE env var.  */

static void state_save()
{
    const char *filename;
    FILE       *file;

    if ((filename = getenv("DEMO_STATE")))
    {
        if ((file = fopen(filename, "w")))
        {
            fprintf(file, "%f %f %f %f %f %f %f %f\n",
                position[0],
                position[1],
                position[2],
                rotation[0],
                rotation[1],
                light[0],
                light[1],
                zoom);
            fclose(file);
        }
    }
}

/* Read the current view state from the file named by the DEMO_STATE env var. */

static void state_load()
{
    const char *filename;
    FILE       *file;

    if ((filename = getenv("DEMO_STATE")))
    {
        if ((file = fopen(filename, "r")))
        {
            fscanf(file, "%f %f %f %f %f %f %f %f",
                position + 0,
                position + 1,
                position + 2,
                rotation + 0,
                rotation + 1,
                light + 0,
                light + 1,
                &zoom);
            fclose(file);
        }
    }
}

/*----------------------------------------------------------------------------*/

static const char *clear_vert_txt = \
    "void main()                                                 \n" \
    "{                                                           \n" \
    "    gl_TexCoord[0] = gl_Vertex * 0.5 + 0.5;                 \n" \
    "    gl_Position    = gl_Vertex;                             \n" \
    "}                                                           \n";

static const char *clear_frag_txt = \
    "#version 120        \n" \

    "uniform float A[64];\n" \
    "uniform vec3  T;    \n" \
    "uniform vec3  B;    \n" \

    "void main()                                                 \n" \
    "{                                                           \n" \
    "    ivec2 p = ivec2(mod(gl_FragCoord.xy - vec2(0.5), 8.0)); \n" \
    "    vec3  c =   mix(B, T, gl_TexCoord[0].y);                \n" \
    "    vec3  d =  vec3(A[p.x * 8 + p.y]);                      \n" \

    "    gl_FragColor = vec4(d + c, 1.0); \n" \
    "}                                    \n";

static int start(int argc, char **argv)
{
    /* Initialize the view and light state. */

    if (demo_mode == DEMO_FLY)
    {
        position[1] =  2.0;
        rotation[0] = 11.3;
    }

    state_load();

    /* Initialize the screen clearing shader. */

    clear_vert = glsl_init_shader(GL_VERTEX_SHADER,   clear_vert_txt, -1);
    clear_frag = glsl_init_shader(GL_FRAGMENT_SHADER, clear_frag_txt, -1);

    if ((clear = glsl_init_program(clear_vert, clear_frag)))
    {
        glUseProgram(clear);
        {
            static const GLfloat A[64] = {
                0.00006033f, 0.00295626f, 0.00078431f, 0.00368024f,
                0.00024132f, 0.00313725f, 0.00096530f, 0.00386124f,
                0.00199095f, 0.00102564f, 0.00271493f, 0.00174962f,
                0.00217195f, 0.00120664f, 0.00289593f, 0.00193062f,
                0.00054298f, 0.00343891f, 0.00030165f, 0.00319759f,
                0.00072398f, 0.00361991f, 0.00048265f, 0.00337858f,
                0.00247360f, 0.00150830f, 0.00223228f, 0.00126697f,
                0.00265460f, 0.00168929f, 0.00241327f, 0.00144796f,
                0.00018099f, 0.00307692f, 0.00090497f, 0.00380090f,
                0.00012066f, 0.00301659f, 0.00084464f, 0.00374057f,
                0.00211161f, 0.00114630f, 0.00283560f, 0.00187029f,
                0.00205128f, 0.00108597f, 0.00277526f, 0.00180995f,
                0.00066365f, 0.00355958f, 0.00042232f, 0.00331825f,
                0.00060331f, 0.00349925f, 0.00036199f, 0.00325792f,
                0.00259427f, 0.00162896f, 0.00235294f, 0.00138763f,
                0.00253394f, 0.00156863f, 0.00229261f, 0.00132730f
            };
            static const GLfloat T[3] = { 0.4f, 0.4f, 0.4f };
            static const GLfloat B[3] = { 0.2f, 0.2f, 0.2f };

            glUniform1fv(glGetUniformLocation(clear, "A"), 64, A);
            glUniform3fv(glGetUniformLocation(clear, "T"),  3, T);
            glUniform3fv(glGetUniformLocation(clear, "B"),  3, B);
        }
        glUseProgram(0);
    }

    /* Initialize the demo's internal state. */

    gettimeofday(&t0, 0);

    if (demo_init)
        return demo_init(argc, argv);
    else
        return 1;
}

static int close()
{
    if (demo_quit)
        demo_quit();

    glDeleteProgram(clear);
    glDeleteShader(clear_frag);
    glDeleteShader(clear_vert);

    state_save();

    exit(0);
}

/*----------------------------------------------------------------------------*/

void demo_clear(const float *T, const float *B)
{
    if (clear)
    {
        glPushAttrib(GL_DEPTH_BUFFER_BIT);
        glEnableClientState(GL_VERTEX_ARRAY);
        {
            static const GLfloat p[4][4] = {
                { -1.0f, -1.0f,  1.0f,  1.0f },
                {  1.0f, -1.0f,  1.0f,  1.0f },
                { -1.0f,  1.0f,  1.0f,  1.0f },
                {  1.0f,  1.0f,  1.0f,  1.0f },
            };

            glDisable(GL_DEPTH_TEST);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glVertexPointer(4, GL_FLOAT, 0, p);

            glUseProgram(clear);
            {
                if (T) glUniform3fv(glGetUniformLocation(clear, "T"), 3, T);
                if (B) glUniform3fv(glGetUniformLocation(clear, "B"), 3, B);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
            glUseProgram(0);
        }
        glDisableClientState(GL_VERTEX_ARRAY);
        glPopAttrib();
    }
    else glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

/*----------------------------------------------------------------------------*/

static void camera_fly(void)
{
    glRotatef(rotation[0], 1.0f, 0.0f, 0.0f);
    glRotatef(rotation[1], 0.0f, 1.0f, 0.0f);
    glTranslatef(-position[0], -position[1], -position[2]);
}

static void camera_dolly(void)
{
    glTranslatef(0.0f, 0.0f, -position[2]);
}

static void camera_tumble(void)
{
    glTranslatef(-position[0], -position[1], -position[2]);
    glRotatef(rotation[0], 1.0f, 0.0f, 0.0f);
    glRotatef(rotation[1], 0.0f, 1.0f, 0.0f);
}

static void camera(void)
{
    /* Apply the camera transformation. */

    switch (demo_mode)
    {
    case DEMO_FLY:    camera_fly();    break;
    case DEMO_DOLLY:  camera_dolly();  break;
    case DEMO_TUMBLE: camera_tumble(); break;
    }
}

static void lights()
{
    /* Position the global light. */

    const GLfloat L[4] = { 0.0f, 0.0f, 1.0f, 0.0f };

    glPushMatrix();
    {
        glRotatef(light[1], 0.0f, 1.0f, 0.0f);
        glRotatef(light[0], 1.0f, 0.0f, 0.0f);
        glLightfv(GL_LIGHT0, GL_POSITION, L);
    }
    glPopMatrix();

    /* Position the flashlight. */

    glLightfv(GL_LIGHT1, GL_POSITION, point);
}

/*----------------------------------------------------------------------------*/

static void pan_camera(GLfloat dx, GLfloat dy)
{
    rotation[0] = click_rotation[0] +  90.0 * dy * zoom;
    rotation[1] = click_rotation[1] + 180.0 * dx * zoom;

    if (rotation[0] >   90.0) rotation[0]  =  90.0;
    if (rotation[0] <  -90.0) rotation[0]  = -90.0;
    if (rotation[1] >  180.0) rotation[1] -= 360.0;
    if (rotation[1] < -180.0) rotation[1] += 360.0;

    glutPostRedisplay();
}

static void pan_light(GLfloat dx, GLfloat dy)
{
    light[0] = click_light[0] +  90.0 * dy;
    light[1] = click_light[1] + 180.0 * dx;

    if (light[0] >   90.0) light[0]  =  90.0;
    if (light[0] <  -90.0) light[0]  = -90.0;
    if (light[1] >  180.0) light[1] -= 360.0;
    if (light[1] < -180.0) light[1] += 360.0;

    glutPostRedisplay();
}

static void zoom_camera(GLfloat dy)
{
    zoom = click_zoom + dy;

    if (zoom < 0.01) zoom = 0.01;

    glutPostRedisplay();
}

/*----------------------------------------------------------------------------*/

static void snap(void)
{
    int   w = glutGet(GLUT_WINDOW_WIDTH);
    int   h = glutGet(GLUT_WINDOW_HEIGHT);
    void *p;

    if ((p = malloc(w * h * 4)))
    {
        glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, p);
        image_flip (           w, h, 4, 1, p);
        image_write("out.png", w, h, 4, 1, p);
        free(p);
    }
}

static void tilt(void)
{
    velocity[0] = 0.f;
    velocity[1] = 0.f;
    velocity[2] = 0.f;

    if (demo_tilt)
        demo_tilt();
}
/*----------------------------------------------------------------------------*/

static void keyboardup(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 'a':           velocity[0] += 1.0; break;
        case 'd': case 'e': velocity[0] -= 1.0; break;
        case 'c': case 'j': velocity[1] += 1.0; break;
        case ' ':           velocity[1] -= 1.0; break;
        case 'w': case ',': velocity[2] += 1.0; break;
        case 's': case 'o': velocity[2] -= 1.0; break;

        case  9: tilt();  break;
        case 13: snap();  break;
        case 27: close(); break;
    }
}

static void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 'a':           velocity[0] -= 1.0; break;
        case 'd': case 'e': velocity[0] += 1.0; break;
        case 'c': case 'j': velocity[1] -= 1.0; break;
        case ' ':           velocity[1] += 1.0; break;
        case 'w': case ',': velocity[2] -= 1.0; break;
        case 's': case 'o': velocity[2] += 1.0; break;
    }
}

/*----------------------------------------------------------------------------*/

static void motion(int x, int y)
{
    const int w = glutGet(GLUT_WINDOW_WIDTH);
    const int h = glutGet(GLUT_WINDOW_HEIGHT);

    GLfloat H = 0.1f * zoom * w / h;
    GLfloat V = 0.1f * zoom;
    GLfloat r;

    /* Compute the pointer motion as a fraction of window size. */

    GLfloat dx = (GLfloat) (x - click_x) / w;
    GLfloat dy = (GLfloat) (y - click_y) / h;

    /* Apply the pointer motion to the camera or light. */

    if (click_button == GLUT_LEFT_BUTTON)
    {
        if      (click_modifiers == 0)                  pan_camera(dx, dy);
        else if (click_modifiers == GLUT_ACTIVE_CTRL)   pan_light (dx, dy);
        else if (click_modifiers == GLUT_ACTIVE_SHIFT) zoom_camera(    dy);
    }

    /* Compute the eye-space pointer vector. */

    point[0] =  (2.0f * x / w - 1.0f) * H;
    point[1] = -(2.0f * y / h - 1.0f) * V;
    point[2] = -(0.1f);

    r = 1.0f / (GLfloat) sqrt(point[0] * point[0] +
                              point[1] * point[1] +
                              point[2] * point[2]);
    point[0] *= r;
    point[1] *= r;
    point[2] *= r;
}

static void mouse(int button, int state, int x, int y)
{
    /* Note all camera state at the beginning of a click. */

    if (state == GLUT_DOWN)
    {
        click_modifiers   = glutGetModifiers();
        click_button      = button;
        click_x           = x;
        click_y           = y;
        click_zoom        = zoom;
        click_rotation[0] = rotation[0];
        click_rotation[1] = rotation[1];
        click_light[0]    = light[0];
        click_light[1]    = light[1];
    }
    else click_button = -1;
}

/*----------------------------------------------------------------------------*/

static void perf()
{
    char   str[256];
    double ft;

    /* Compute the frame time. */

    gettimeofday(&t1, 0);

    ft = (t1.tv_sec  - t0.tv_sec) +
         (t1.tv_usec - t0.tv_usec) / 1000000.0;

    dt = (dt * 15.0 + ft) / 16.0;
    t0 = t1;

    /* Display it in the window title. */

    sprintf(str, "%5.2f ms %4d fps\n", (1000.0 * dt), (int) (1.0 / dt));
    glutSetWindowTitle(str);
}

static void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}

static void display(void)
{
    /* Initialize the projection and model-view matrices. */

    GLfloat V = 0.1f * zoom;
    GLfloat H = 0.1f * zoom * glutGet(GLUT_WINDOW_WIDTH)
                            / glutGet(GLUT_WINDOW_HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-H, H, -V, V, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Draw the scene. */

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera();
    lights();

    if (demo_draw)
        demo_draw();

    glutSwapBuffers();
    perf();
}

/*----------------------------------------------------------------------------*/

static void idle(void)
{
    int curr_time = glutGet(GLUT_ELAPSED_TIME);

    GLfloat speed = 3.0f, dt = (curr_time - last_time) / 1000.0f;

    /* Compute the position change from the speed, time, and velocity. */

    GLfloat v[3];

    v[0] = speed * dt * velocity[0];
    v[1] = speed * dt * velocity[1];
    v[2] = speed * dt * velocity[2];

    /* Apply the position change to the position, possibly in world space. */

    if (demo_mode == DEMO_FLY)
    {
        GLfloat M[16];

        glGetFloatv(GL_MODELVIEW_MATRIX, M);

        position[0] += M[ 0] * v[0] + M[ 1] * v[1] + M[ 2] * v[2];
        position[1] += M[ 4] * v[0] + M[ 5] * v[1] + M[ 6] * v[2];
        position[2] += M[ 8] * v[0] + M[ 9] * v[1] + M[10] * v[2];
    }
    else
    {
        position[0] += v[0];
        position[1] += v[1];
        position[2] += v[2];
    }

    /* Step the demo as needed. */

    if (demo_step)
        demo_step(dt);

    glutPostRedisplay();

    last_time = curr_time;
}

/*----------------------------------------------------------------------------*/

int demo(int mode, int argc, char *argv[], demo_init_f init, demo_tilt_f tilt,
                         demo_quit_f quit, demo_draw_f draw, demo_step_f step)
{
    demo_mode = mode;
    demo_init = init;
    demo_tilt = tilt;
    demo_quit = quit;
    demo_draw = draw;
    demo_step = step;

    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(1024, 768);
    glutInit(&argc, argv);

    glutCreateWindow(argv[0]);

    glutKeyboardUpFunc(keyboardup);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutPassiveMotionFunc(motion);
    glutMotionFunc(motion);
    glutMouseFunc(mouse);
    glutIdleFunc(idle);

    glutIgnoreKeyRepeat(1);

    if (glewInit() == GLEW_OK)
    {
        if (start(argc, argv))
            glutMainLoop();

    }
    return 0;
}
