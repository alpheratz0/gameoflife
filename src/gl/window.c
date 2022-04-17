#include <GL/freeglut.h>

#include "window.h"

static u32 width = 800, height = 600;
static window_display_callback_t display_callback;
static window_key_press_callback_t key_press_callback;
static window_mouse_down_callback_t mouse_down_callback;

static void
internal_reshape_callback(i32 w, i32 h) {
	width = w; height = h;
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);
	glScalef(1, -1, 1);
	glTranslatef(0, -h, 0);
}

static void
internal_display_callback(void) {
	if (display_callback) {
		display_callback(width, height);
		glutSwapBuffers();
	}
}

static void
internal_key_press_callback(u8 key, i32 x, i32 y) {
	if (key_press_callback) {
		key_press_callback(key);
	}
}

static void
internal_mouse_down_callback(i32 button, i32 mstate, i32 x, i32 y) {
	if (mouse_down_callback && mstate == GLUT_DOWN) {
		mouse_down_callback(x, y, button);
	}
}

extern void
window_init(const char *name) {
	int argc = 0;
	char *argv[] = { NULL };

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(name);
	glClearColor(0, 0, 0, 1.0);
	glShadeModel(GL_FLAT);

	glutDisplayFunc(internal_display_callback);
	glutReshapeFunc(internal_reshape_callback);
	glutMouseFunc(internal_mouse_down_callback);
	glutKeyboardFunc(internal_key_press_callback);
}

extern void
window_loop_start(void) {
	glutMainLoop();
}

extern void
window_set_display_callback(window_display_callback_t cb) {
	display_callback = cb;
}

extern void
window_set_mouse_down_callback(window_mouse_down_callback_t cb) {
	mouse_down_callback = cb;
}

extern void
window_set_key_press_callback(window_key_press_callback_t cb) {
	key_press_callback = cb;
}

extern void
window_set_loop(window_loopfunc_t f) {
	glutIdleFunc(f);
}

extern void
window_clear_loop() {
	glutIdleFunc(NULL);
}

extern void
window_force_redraw(void) {
	glutPostRedisplay();
}
