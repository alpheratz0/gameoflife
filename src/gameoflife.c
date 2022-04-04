/*
	Copyright (C) 2021-2022 <alpheratz99@protonmail.com>

	This program is free software; you can redistribute it and/or modify it under 
	the terms of the GNU General Public License version 2 as published by the 
	Free Software Foundation.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY 
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
	FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along with 
	this program; if not, write to the Free Software Foundation, Inc., 59 Temple 
	Place, Suite 330, Boston, MA 02111-1307 USA	

*/

#define CGOL_VERSION 1
#define MATCH_OPT(input,sh,lo) (strcmp((input), (sh)) == 0 || strcmp((input), (lo)) == 0)

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <ctype.h>
#include <GL/freeglut.h>

#include "frame.h"
#include "keyconfig.h"

static bool reversedcontrols = false;
static bool showgrid = true;
static bool paused = true;

static uint8_t *cells;
static uint8_t cellsize = 20;

static int32_t rows = 300;
static int32_t columns = 300;

/* window */
static int32_t width = 640;
static int32_t height = 480;

/* pointer offset from origin */
static char offsetstr[30];
static int32_t pointer_xoffset = 0;
static int32_t pointer_yoffset = 0;

void usage(void) {
	printf("Usage: gameoflife [ -hG ] FILENAME\n");
	printf("Options are:\n");
	printf("     -G | --no-grid                 hide the grid\n");
	printf("     -r | --reversed-controls       reverse the controls\n");
	printf("     -h | --help                    display this message and exit\n");
}

void die(const char* message) {
	fprintf(stderr, "error: %s\n", message);
	exit(1);
}

uint8_t get_cell(int32_t x, int32_t y) {
	x = ((x % columns) + columns) % columns;
	y = ((y % rows) + rows) % rows;
	return cells[y * columns + x];
}

void toggle_cell(int32_t x, int32_t y) {
	x = ((x % columns) + columns) % columns;
	y = ((y % rows) + rows) % rows;
	cells[y * columns + x] ^= 1;
}

void set_cell(int32_t x, int32_t y, uint8_t value) {
	x = ((x % columns) + columns) % columns;
	y = ((y % rows) + rows) % rows;
	cells[y * columns + x] = value;
}

void world_init(void) {
	cells = (uint8_t*)(malloc(sizeof(uint8_t)*rows*columns));
	memset(cells, 0, sizeof(uint8_t)*rows*columns);
}

void world_advance(void) {
	uint8_t *temp_cells = (uint8_t*)(malloc(sizeof(uint8_t)*columns*rows));
	memcpy(temp_cells, cells, sizeof(uint8_t)*columns*rows);

	uint8_t neighbours_alive = 0;

	for (int32_t x = 0; x < columns; ++x) {
		for (int32_t y = 0; y < rows; ++y) {

			neighbours_alive = 0;

			neighbours_alive += get_cell(x - 1, y - 1);
			neighbours_alive += get_cell(x - 1, y);
			neighbours_alive += get_cell(x - 1, y + 1);

			neighbours_alive += get_cell(x, y - 1);
			neighbours_alive += get_cell(x, y + 1);

			neighbours_alive += get_cell(x + 1, y - 1);
			neighbours_alive += get_cell(x + 1, y);
			neighbours_alive += get_cell(x + 1, y + 1);

			/* follow the rules */
			if (get_cell(x, y) == 0) {
				if (neighbours_alive == 3) {
					temp_cells[y*columns+x] = 1;
				}
				continue;
			} 

			if (neighbours_alive < 2 || neighbours_alive > 3) {
				temp_cells[y*columns+x] = 0;
			}
		}
	}

	memcpy(cells, temp_cells, sizeof(uint8_t)*columns*rows);
	free(temp_cells);
}

void world_save(void) {
	char filename[21];
	time_t timer;
	struct tm* tm_info;

	timer = time(NULL);
	tm_info = localtime(&timer);

	strftime(filename, 21, "%Y%m%d%H%M%S.cgol", tm_info);

	FILE *savefile = fopen(filename, "w");

	if (savefile == NULL) {
		savefile = fopen("/dev/stdout", "w");
	}

	fprintf(savefile, "cgol-version: %d\n", CGOL_VERSION);
	fprintf(savefile, "columns: %d\n", columns);
	fprintf(savefile, "rows: %d\n", rows);

	for (int32_t x = 0; x < columns; ++x) {
		for (int32_t y = 0; y < rows; ++y) {
			if (get_cell(x, y) == 1) {
				fprintf(savefile, "(%d, %d)\n", x, y);
			}
		}
	}

	fclose(savefile);
}

void world_load(FILE* savefile) {
	if (savefile == NULL) {
		die("file does not exist");
	}

	int32_t version;
	int32_t x;
	int32_t y;

	if (fscanf(savefile, "cgol-version: %d\n", &version) == 0) {
		die("version field not found");
	}
	
	if (version != 1) {
		die("unknown file version");
	}

	if (fscanf(savefile, "columns: %d\n", &columns) == 0) {
		die("columns field not found");
	}

	if (fscanf(savefile, "rows: %d\n", &rows) == 0) {
		die("rows field not found");
	}

	if (rows < 1) {
		die("invalid number of rows");
	}

	if (columns < 1) {
		die("invalid number of columns");
	}

	cells = (uint8_t*)(realloc(cells, sizeof(uint8_t)*columns*rows));
	memset(cells, 0, sizeof(uint8_t)*columns*rows);

	while (fscanf(savefile, "(%d, %d)\n", &x, &y) == 2) {
		set_cell(x, y, 1);
	}
}

void move(int32_t xoffset, int32_t yoffset) {
	uint8_t *temp_cells = (uint8_t*)(malloc(sizeof(uint8_t)*columns*rows));
	memcpy(temp_cells, cells, sizeof(uint8_t)*columns*rows);

	for (int32_t y = 0; y < rows; ++y) {
		for (int32_t x = 0; x < columns; ++x) {
			set_cell(x + xoffset, y + yoffset, temp_cells[y*columns+x]);
		}
	}

	free(temp_cells);
}

void loop() {
	if (is_on_frame()) {
		world_advance();
		glutPostRedisplay();
	}
}

void toggle_pause(void) {
	paused = !paused;

	if (!paused) {
		set_frame_start();
		glutIdleFunc(loop);
		return;
	}

	glutIdleFunc(NULL);
}

void zoom(int8_t units) {
	cellsize += units;
	if (cellsize < 4) cellsize = 4;
	if (cellsize > 80) cellsize = 80;
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.188, 0.639, 0.956);

	for (int32_t x = 0; x <= width / cellsize; ++x) {
		for (int32_t y = 0; y <= height / cellsize; ++y) {
			if (get_cell(x, y) == 1) {
				glRectf(x * cellsize, y * cellsize, (x + 1) * cellsize, (y + 1) * cellsize);
			}
		}
	}

	if (showgrid) {
		glColor3f(0.1, 0.1, 0.1);

		for (int32_t col = 0; col < (1 + width / cellsize); ++col) {
			glBegin(GL_LINES);
			glVertex2d(col * cellsize, 0);
			glVertex2d(col * cellsize, height);
			glEnd();
		}

		for (int32_t row = 0; row < (1 + height / cellsize); ++row) {
			glBegin(GL_LINES);
			glVertex2d(0, row * cellsize);
			glVertex2d(width, row * cellsize);
			glEnd();
		}
	}

	sprintf(offsetstr, "(%d, %d)", pointer_xoffset, pointer_yoffset);
	glColor4f(0.94117, 0.96862, 0.16470, 1.0);
	glRasterPos2f(10, height - 10);
	glutBitmapString((void*)(GLUT_BITMAP_HELVETICA_12), (unsigned char*)(offsetstr));

	glutSwapBuffers();
}

void reshape(int w, int h) {
	width = (int32_t)(w); 
	height = (int32_t)(h);

	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);
	glScalef(1, -1, 1);
	glTranslatef(0, -h, 0);
}

void mouse(int button, int mstate, int x, int y) {
	if (paused && button == GLUT_LEFT_BUTTON && mstate == GLUT_UP) {
		toggle_cell((x/cellsize), (y/cellsize));
		glutPostRedisplay();
	}
}

void mouse_motion(int x, int y) {
	pointer_xoffset=x/cellsize;
	pointer_yoffset=y/cellsize;
	glutPostRedisplay();
}

void keyboard(unsigned char c, int x, int y) {	
	c = tolower(c);
	switch (c) {
		case KEY_QUIT:
			exit(0);
			break;
		case KEY_NEXT:
			world_advance();
			glutPostRedisplay();
			break;
		case KEY_SAVE:
			world_save();
			break;
		case KEY_PAUSE:
			toggle_pause();
			break;
		case KEY_ZOOM_IN:
			zoom(1);
			glutPostRedisplay();
			break;
		case KEY_ZOOM_OUT:
			zoom(-1);
			glutPostRedisplay();
			break;
		case KEY_LEFT:
			move((reversedcontrols ? -1 : 1), 0);
			glutPostRedisplay();
			break;
		case KEY_RIGHT:
			move((reversedcontrols ? 1 : -1), 0);
			glutPostRedisplay();
			break;
		case KEY_UP:
			move(0, (reversedcontrols ? -1 : 1));
			glutPostRedisplay();
			break;
		case KEY_DOWN:
			move(0, (reversedcontrols ? 1 : -1));
			glutPostRedisplay();
			break;
		default:
			break;
	}
}

void process_args(int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
		if(MATCH_OPT(argv[i], "-G", "--no-grid")) {
			showgrid = false;
		}
		else if (MATCH_OPT(argv[i], "-h", "--help")) {
			usage();
			exit(0);
		}
		else if (MATCH_OPT(argv[i], "-r", "--reversed-controls")) {
			reversedcontrols = true;
		}
		else {
			if (strcmp(argv[i], "-") == 0) world_load(stdin);
			else world_load(fopen(argv[i], "r"));
			break;
		}
	}
}

int main(int argc, char** argv) {
	set_frame_rate(10);
	world_init();	
	process_args(argc, argv);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("conway's game of life");
	glClearColor(0, 0, 0, 1.0);
	glShadeModel(GL_FLAT);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutPassiveMotionFunc(mouse_motion);
	glutKeyboardFunc(keyboard);

	glutMainLoop();

	return 0;
}
