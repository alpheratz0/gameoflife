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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include <GL/freeglut.h>

#include "board.h"
#include "buttons.h"
#include "debug.h"
#include "keys.h"
#include "numdef.h"
#include "window.h"

static board_t *board;
static i32 cellsize = 20;
static i32 loop_start = 0;
static bool running = false;

static bool
match_opt(const char *in, const char *sh, const char *lo) {
	return (strcmp(in, sh) == 0) ||
		   (strcmp(in, lo) == 0);
}

static void
usage(void) {
	puts("Usage: gameoflife [ -hkv ] [ -l FILE ]");
	puts("Options are:");
	puts("     -h | --help                    display this message and exit");
	puts("     -k | --keybindings             display the keybindings");
	puts("     -v | --version                 display the program version");
	puts("     -l | --load                    load a .cgol file");
	exit(0);
}

static void
version(void) {
	puts("gameoflife version "VERSION);
	exit(0);
}

static void
keybindings(void) {
	puts("Keybindings are:");
	puts("s: save board");
	puts("n: advance to next generation");
	puts("h/l: move left/right");
	puts("k/j: move up/down");
	puts("+/-: zoom in/out");
	puts("space: toggle pause");
	puts("q: exit");
	exit(0);
}

static void
display(i32 width, i32 height) {
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.188, 0.639, 0.956);

	for (i32 x = 0; x <= width / cellsize; ++x) {
		for (i32 y = 0; y <= height / cellsize; ++y) {
			if (board_get(board, x, y)) {
				glRectf(x * cellsize, y * cellsize, (x + 1) * cellsize, (y + 1) * cellsize);
			}
		}
	}

	glColor3f(0.1, 0.1, 0.1);

	for (i32 column = 0; column < (1 + width / cellsize); ++column) {
		glBegin(GL_LINES);
		glVertex2d(column * cellsize, 0);
		glVertex2d(column * cellsize, height);
		glEnd();
	}

	for (i32 row = 0; row < (1 + height / cellsize); ++row) {
		glBegin(GL_LINES);
		glVertex2d(0, row * cellsize);
		glVertex2d(width, row * cellsize);
		glEnd();
	}

	glColor4f(0.94117, 0.96862, 0.16470, 1.0);
	glRasterPos2f(10, height - 10);
	glutBitmapString(GLUT_BITMAP_HELVETICA_12, (u8 *)(running ? "RUNNING" : "PAUSED"));
}

static void
loop(void) {
	i32 elapsed = glutGet(GLUT_ELAPSED_TIME) - loop_start;
	if (elapsed > (1000 / 12)) {
		loop_start += elapsed;
		board_advance(&board);
		window_force_redraw();
	}
}

static void
keydown(u8 key) {
	switch(tolower(key)) {
		case KEY_Q:
			exit(0);
			break;
		case KEY_N:
			board_advance(&board);
			window_force_redraw();
			break;
		case KEY_S:
			board_save(board);
			break;
		case KEY_SPACE:
			running = !running;
			if (running) {
				loop_start = glutGet(GLUT_ELAPSED_TIME);
				window_set_loop(loop);
			}
			else window_clear_loop();
			window_force_redraw();
			break;
		case KEY_PLUS:
			cellsize = cellsize == 80 ? 80 : cellsize + 1;
			window_force_redraw();
			break;
		case KEY_MINUS:
			cellsize = cellsize == 4 ? 4 : cellsize - 1;
			window_force_redraw();
			break;
		case KEY_H:
			board_shift(&board, -1, 0);
			window_force_redraw();
			break;
		case KEY_L:
			board_shift(&board, 1, 0);
			window_force_redraw();
			break;
		case KEY_K:
			board_shift(&board, 0, -1);
			window_force_redraw();
			break;
		case KEY_J:
			board_shift(&board, 0, 1);
			window_force_redraw();
			break;
	}
}

static void
mousedown(i32 x, i32 y, i32 button) {
	if (!running && button == BUTTON_LEFT) {
		board_toggle(board, (x / cellsize), (y / cellsize));
		window_force_redraw();
	}
}

int
main(int argc, char** argv) {
	/* skip program name */
	--argc; ++argv;

	if (argc > 0) {
		if (match_opt(*argv, "-h", "--help")) usage();
		else if (match_opt(*argv, "-l", "--load") && --argc > 0) board = board_load(*++argv);
		else if (match_opt(*argv, "-k", "--keybindings")) keybindings();
		else if (match_opt(*argv, "-v", "--version")) version();
		else dief("invalid option %s", *argv);
	}

	if (!board) {
		board = board_create(300, 300);
	}

	window_init("conway's game of life");
	window_set_display_callback(display);
	window_set_keydown_callback(keydown);
	window_set_mousedown_callback(mousedown);
	window_loop_start();

	return 0;
}
