#include <GL/freeglut.h>

#include "window.h"

static u32 width = 800, height = 600;
static window_display_callback_t display_callback;
static window_keydown_callback_t keydown_callback;
static window_mousedown_callback_t mousedown_callback;

static void
reshape(i32 w, i32 h) {
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
display(void) {
	if (display_callback) {
		display_callback(width, height);
		glutSwapBuffers();
	}
}

static void
keydown(u8 key, i32 x, i32 y) {
	if (keydown_callback) {
		keydown_callback(key);
	}
}

static void
mousedown(i32 button, i32 mstate, i32 x, i32 y) {
	if (mousedown_callback && mstate == GLUT_DOWN) {
		mousedown_callback(x, y, button);
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

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mousedown);
	glutKeyboardFunc(keydown);
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
window_set_mousedown_callback(window_mousedown_callback_t cb) {
	mousedown_callback = cb;
}

extern void
window_set_keydown_callback(window_keydown_callback_t cb) {
	keydown_callback = cb;
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
