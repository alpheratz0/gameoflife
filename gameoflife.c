/*
	Copyright (C) 2022 <alpheratz99@protonmail.com>

	This program is free software; you can redistribute it and/or modify it under
	the terms of the GNU General Public License version 2 as published by the
	Free Software Foundation.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along with
	this program; if not, write to the Free Software Foundation, Inc., 59 Temple
	Place, Suite 330, Boston, MA 02111-1307 USA

	 __________
	( the game )
	 ----------
	   o
	    o

	     |\_/|
	     |o o|__
	     --*--__\
	     C_C_(___)

*/

#define _XOPEN_SOURCE 500

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <GL/freeglut.h>

#define UNUSED                             __attribute__((unused))

#define DEAD_COLOR                         (0x000000)
#define ALIVE_COLOR                        (0x30a3f4)
#define BORDER_COLOR                       (0x191919)
#define BAR_COLOR                          (0xf0f72a)
#define TEXT_COLOR                         (0x000000)

#define GENERATIONS_PER_SECOND             (12)
#define NANOSECONDS_PER_SECOND             (1000*1000*1000)
#define FONT_HEIGHT                        (8)
#define INFO_BAR_HEIGHT                    (20)

#define DEFAULT_COLUMNS                    (300)
#define DEFAULT_ROWS                       (300)
#define DEFAULT_CELLSIZE                   (20)

#define MIN_CELLSIZE                       (5)
#define MAX_CELLSIZE                       (50)

#define MOUSE_LEFT                         (0)
#define MOUSE_MIDDLE                       (1)
#define MOUSE_RIGHT                        (2)
#define MOUSE_WHEEL_UP                     (3)
#define MOUSE_WHEEL_DOWN                   (4)

#define KEY_SPACE                          (32)
#define KEY_CTRL_S                         (19)
#define KEY_N                              (110)

#define HEX_TO_GL_COLOR(hex) ((float)(hex >> 16)) / 0xff, \
                             ((float)((hex >> 8) & 0xff)) / 0xff, \
                             ((float)(hex & 0xff)) / 0xff


struct point {
	int16_t x;
	int16_t y;
};

static int window;
static int width;
static int height;

/* board */
static int32_t rows;
static int32_t columns;
static int32_t cellsize;
static uint8_t *cells[2];

/* game status */
static int running;
static struct point hovered;

/* dragging */
static int dragging;
static struct point offset;
static struct point mousepos;

/* blocksleep */
static struct timespec begin_ts;

static void
die(const char *err)
{
	fprintf(stderr, "gameoflife: %s\n", err);
	exit(1);
}

static void
dief(const char *err, ...)
{
	va_list list;
	fputs("gameoflife: ", stderr);
	va_start(list, err);
	vfprintf(stderr, err, list);
	va_end(list);
	fputc('\n', stderr);
	exit(1);
}

static void *
xcalloc(size_t n, size_t size)
{
	void *p;

	if (NULL == (p = calloc(n, size))) {
		die("error while calling calloc, no memory available");
	}

	return p;
}

static void
blockstart(void)
{
	clock_gettime(CLOCK_MONOTONIC, &begin_ts);
}

static void
blockwait(int nanoseconds)
{
	struct timespec now_ts, delta_ts, end_ts;

	end_ts = begin_ts;
	end_ts.tv_nsec += nanoseconds;

	if ((end_ts.tv_nsec / NANOSECONDS_PER_SECOND) > 0) {
		end_ts.tv_sec += end_ts.tv_nsec / NANOSECONDS_PER_SECOND;
		end_ts.tv_nsec = end_ts.tv_nsec % NANOSECONDS_PER_SECOND;
	}

	clock_gettime(CLOCK_MONOTONIC, &now_ts);

	/* check if we already reached the end */
	if (now_ts.tv_sec > end_ts.tv_sec) {
		return;
	}

	if (now_ts.tv_sec == end_ts.tv_sec && now_ts.tv_nsec >= end_ts.tv_nsec) {
		return;
	}

	delta_ts.tv_sec = end_ts.tv_sec - now_ts.tv_sec;
	delta_ts.tv_nsec = end_ts.tv_nsec - now_ts.tv_nsec;

	if (delta_ts.tv_nsec < 0) {
		delta_ts.tv_nsec += NANOSECONDS_PER_SECOND;
		--delta_ts.tv_sec;
	}

	nanosleep(&delta_ts, 0);
}

static void
create_window(void)
{
	int argc;
	char *argv[1];

	argc = 0;
	argv[0] = NULL;

	width = 800;
	height = 600;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(0, 0);
	window = glutCreateWindow("gameoflife");
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
	glClearColor(HEX_TO_GL_COLOR(DEAD_COLOR), 1.0);
	glShadeModel(GL_FLAT);
}

static void
create_board(int32_t c, int32_t r)
{
	cells[0] = xcalloc(c*r, sizeof(uint8_t));
	cells[1] = xcalloc(c*r, sizeof(uint8_t));

	columns = c;
	rows = r;
	cellsize = DEFAULT_CELLSIZE;
}

static uint8_t
get_cell(int x, int y)
{
	x = ((x % columns) + columns) % columns;
	y = ((y % rows) + rows) % rows;
	return cells[0][y * columns + x];
}

static void
set_cell(int x, int y, uint8_t value)
{
	x = ((x % columns) + columns) % columns;
	y = ((y % rows) + rows) % rows;
	cells[0][y * columns + x] = value;
}

static void
toggle_cell(int x, int y)
{
	x = ((x % columns) + columns) % columns;
	y = ((y % rows) + rows) % rows;
	cells[0][y * columns + x] ^= 1;
}

static int
count_neighbours_alive(int x, int y)
{
	int dx, dy;
	int count;

	count = -get_cell(x, y);

	for (dx = -1; dx < 2; ++dx) {
		for (dy = -1; dy < 2; ++dy) {
			count += get_cell(x + dx, y + dy) ? 1 : 0;
		}
	}

	return count;
}

static void
advance_to_next_generation(void)
{
	int x, y, n;
	uint8_t cell, *tmp;

	for (x = 0; x < columns; ++x) {
		for (y = 0; y < rows; ++y) {
			cell = get_cell(x, y);
			n = count_neighbours_alive(x, y);

			cells[1][y*columns+x] = n == 3 || (cell && n == 2);
		}
	}

	/* swap */
	tmp = cells[0];
	cells[0] = cells[1];
	cells[1] = tmp;
}

static void
save_board(void)
{
	int x, y;
	struct tm *now;
	char filename[19];
	FILE *fp;

	now = localtime((const time_t[]) { time(NULL) });
	strftime(filename, sizeof(filename), "%Y%m%d%H%M%S.xg", now);

	if (NULL == (fp = fopen(filename, "w"))) {
		dief("failed to open file %s: %s", filename, strerror(errno));
	}

	fprintf(fp, "%dx%d\n", columns, rows);

	for (x = 0; x < columns; ++x) {
		for (y = 0; y < rows; ++y) {
			if (get_cell(x, y)) {
				fprintf(fp, "%d,%d\n", x, y);
			}
		}
	}

	fclose(fp);
}

static void
load_board(const char *path)
{
	int x, y;
	FILE *fp;

	if (NULL == (fp = fopen(path, "r"))) {
		dief("failed to open file %s: %s", path, strerror(errno));
	}

	if (fscanf(fp, "%dx%d\n", &columns, &rows) != 2) {
		columns = DEFAULT_COLUMNS;
		rows = DEFAULT_ROWS;

		rewind(fp);
	}

	create_board(columns, rows);

	while (fscanf(fp, "%d,%d\n", &x, &y) == 2) {
		set_cell(x, y, 1);
	}

	fclose(fp);
}

static void
destroy_board(void)
{
	free(cells[0]);
	free(cells[1]);
}

static void
render_scene(void)
{
	int32_t col, row;
	int32_t vcolumns, vrows;
	int32_t coloff, rowoff;
	int32_t celloffx, celloffy;
	int32_t i;

	char text[200] = "* RUNNING";

	/* full visible columns & rows */
	vcolumns = width / cellsize;
	vrows = height / cellsize;

	/* column & row offset */
	coloff = offset.x / cellsize;
	rowoff = offset.y / cellsize;

	/* cell x & y offset */
	celloffx = offset.x % cellsize;
	celloffy = offset.y % cellsize;

	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(HEX_TO_GL_COLOR(ALIVE_COLOR));

#define DRAW_RECT(x,y,w,h) glRectf(x,y,x+w,y+h)

	for (col = -1; col < vcolumns + 1; ++col) {
		for (row = -1; row < vrows + 1; ++row) {
			if (get_cell(col - coloff, row - rowoff)) {
				DRAW_RECT(col * cellsize + celloffx, row * cellsize +
						celloffy, cellsize, cellsize);
			}
		}
	}

	glColor3f(HEX_TO_GL_COLOR(BORDER_COLOR));

	for (i = -1; i < vcolumns + 1; ++i) {
		glBegin(GL_LINES);
		glVertex2d(i*cellsize+celloffx, 0);
		glVertex2d(i*cellsize+celloffx, height);
		glEnd();
	}

	for (i = -1; i < vrows + 1; ++i) {
		glBegin(GL_LINES);
		glVertex2d(0, i*cellsize+celloffy);
		glVertex2d(width, i*cellsize+celloffy);
		glEnd();
	}

	glColor3f(HEX_TO_GL_COLOR(BAR_COLOR));
	DRAW_RECT(0, height - INFO_BAR_HEIGHT, width, INFO_BAR_HEIGHT);

#undef DRAW_RECT

	if (!running) {
		snprintf(text, sizeof(text), "* PAUSED (%hd, %hd)", hovered.x, hovered.y);
	}

	glColor3f(HEX_TO_GL_COLOR(TEXT_COLOR));
	glRasterPos2f(INFO_BAR_HEIGHT / 2, height - (INFO_BAR_HEIGHT - FONT_HEIGHT) / 2);
	glutBitmapString(GLUT_BITMAP_8_BY_13, (uint8_t *)(text));
}

static int
match_opt(const char *in, const char *sh, const char *lo)
{
	return (strcmp(in, sh) == 0) || (strcmp(in, lo) == 0);
}

static inline void
print_opt(const char *sh, const char *lo, const char *desc)
{
	printf("%7s | %-25s %s\n", sh, lo, desc);
}

static void
usage(void)
{
	puts("Usage: gameoflife [ -hv ] [ -l FILE ]");
	puts("Options are:");
	print_opt("-h", "--help", "display this message and exit");
	print_opt("-v", "--version", "display the program version");
	print_opt("-l", "--load", "load saved board");
	exit(0);
}

static void
version(void)
{
	puts("gameoflife version "VERSION);
	exit(0);
}

static void
h_display(void)
{
	render_scene();
	glutSwapBuffers();
}

static void
h_key_press(unsigned char key, UNUSED int x, UNUSED int y)
{
	switch (key) {
		case KEY_SPACE:
			if (!dragging) {
				running = !running;
				glutPostRedisplay();
			}
			break;
		case KEY_N:
			if (!running) {
				advance_to_next_generation();
				glutPostRedisplay();
			}
			break;
		case KEY_CTRL_S:
			if (!running) {
				save_board();
			}
			break;
	}
}

static void
h_button_press(int button, int x, int y)
{
	int16_t zoom;

	if (running) {
		return;
	}

	zoom = 0;

	switch (button) {
		case MOUSE_LEFT:
			toggle_cell(hovered.x, hovered.y);
			glutPostRedisplay();
			break;
		case MOUSE_MIDDLE:
			dragging = 1;
			mousepos.x = x;
			mousepos.y = y;
			break;
		case MOUSE_WHEEL_UP:
			if (cellsize < MAX_CELLSIZE) {
				zoom = 1;
			}
			break;
		case MOUSE_WHEEL_DOWN:
			if (cellsize > MIN_CELLSIZE) {
				zoom = -1;
			}
			break;
	}

	if (zoom) {
		offset.x = (offset.x * (cellsize + zoom) - zoom * (width / 2)) / cellsize;
		offset.y = (offset.y * (cellsize + zoom) - zoom * (height / 2)) / cellsize;
		cellsize += zoom;
		glutPostRedisplay();
	}
}

static void
h_button_release(int button, UNUSED int x, UNUSED int y)
{
	if (button == MOUSE_MIDDLE) {
		dragging = 0;
	}
}

static void
h_mouse(int button, int mstate, int x, int y)
{
	if (mstate == GLUT_DOWN) h_button_press(button, x, y);
	else if (mstate == GLUT_UP) h_button_release(button, x, y);
}

static void
h_passive_motion(int x, int y)
{
	hovered.x = floor(((float)(x - offset.x)) / cellsize);
	hovered.y = floor(((float)(y - offset.y)) / cellsize);

	if (!running) {
		glutPostRedisplay();
	}
}

static void
h_motion(int x, int y)
{
	if (dragging) {
		offset.x += x - mousepos.x;
		offset.y += y - mousepos.y;

		mousepos.x = x;
		mousepos.y = y;

		glutPostRedisplay();
	}
}

static void
h_loop(void)
{
	if (running) {
		advance_to_next_generation();
		glutPostRedisplay();
		blockwait(NANOSECONDS_PER_SECOND / GENERATIONS_PER_SECOND);
		blockstart();
	}
}

static void
h_reshape(int new_width, int new_height)
{
	width = new_width;
	height = new_height;

	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1);
	glScalef(1, -1, 1);
	glTranslatef(0, -height, 0);
}

int
main(int argc, char **argv)
{
	const char *loadpath = NULL;

	if (++argv, --argc > 0) {
		if (match_opt(*argv, "-l", "--load") && --argc > 0) loadpath = *++argv;
		else if (match_opt(*argv, "-h", "--help")) usage();
		else if (match_opt(*argv, "-v", "--version")) version();
		else if (**argv == '-') dief("invalid option %s", *argv);
		else dief("unexpected argument: %s", *argv);
	}

	if (NULL == loadpath) create_board(DEFAULT_COLUMNS, DEFAULT_ROWS);
	else load_board(loadpath);

	create_window();

	glutDisplayFunc(h_display);
	glutReshapeFunc(h_reshape);
	glutMouseFunc(h_mouse);
	glutKeyboardFunc(h_key_press);
	glutIdleFunc(h_loop);
	glutMotionFunc(h_motion);
	glutPassiveMotionFunc(h_passive_motion);

	glutMainLoop();

	destroy_board();

	return 0;
}
