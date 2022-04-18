#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "../util/debug.h"
#include "../util/numdef.h"
#include "board.h"

extern bool
board_get(board_t *board, i32 x, i32 y) {
	x = ((x % board->columns) + board->columns) % board->columns;
	y = ((y % board->rows) + board->rows) % board->rows;
	return board->cells[y * board->columns + x];
}

extern void
board_set(board_t *board, i32 x, i32 y, bool value) {
	x = ((x % board->columns) + board->columns) % board->columns;
	y = ((y % board->rows) + board->rows) % board->rows;
	board->cells[y * board->columns + x] = value;
}

extern void
board_toggle(board_t *board, i32 x, i32 y) {
	x = ((x % board->columns) + board->columns) % board->columns;
	y = ((y % board->rows) + board->rows) % board->rows;
	board->cells[y * board->columns + x] ^= true;
}

extern void
board_advance(board_t **ref) {
	board_t *board = *ref;
	board_t *new = board_create(board->rows, board->columns);

	u8 neighbours_alive = 0;

	for (i32 x = 0; x < board->columns; ++x) {
		for (i32 y = 0; y < board->rows; ++y) {

			neighbours_alive = 0;

			neighbours_alive += board_get(board, x - 1, y - 1);
			neighbours_alive += board_get(board, x - 1, y);
			neighbours_alive += board_get(board, x - 1, y + 1);

			neighbours_alive += board_get(board, x, y - 1);
			neighbours_alive += board_get(board, x, y + 1);

			neighbours_alive += board_get(board, x + 1, y - 1);
			neighbours_alive += board_get(board, x + 1, y);
			neighbours_alive += board_get(board, x + 1, y + 1);

			/* follow the rules */
			if (!board_get(board, x, y)) {
				board_set(new, x, y, neighbours_alive == 3 ? true : false);
				continue;
			}

			board_set(new, x, y, (neighbours_alive < 2 || neighbours_alive > 3) ? false : true);
		}
	}

	board_free(board);
	*ref = new;
}

extern void
board_shift(board_t **ref, i32 xo, i32 yo) {
	board_t *board = *ref;
	board_t *new = board_create(board->rows, board->columns);

	for (i32 y = 0; y < board->rows; ++y) {
		for (i32 x = 0; x < board->columns; ++x) {
			board_set(new, x + xo, y + yo, board_get(board, x, y));
		}
	}

	board_free(board);
	*ref = new;
}

extern void
board_save(board_t *board) {
	time_t timer;
	struct tm *tm_info;
	char filename[21];
	FILE *file;

	timer = time(NULL);
	tm_info = localtime(&timer);

	strftime(filename, sizeof(filename), "%Y%m%d%H%M%S.cgol", tm_info);

	if (!(file = fopen(filename, "w"))) {
		dief("failed to open file %s: %s", filename, strerror(errno));
	}

	for (i32 x = 0; x < board->columns; ++x) {
		for (i32 y = 0; y < board->rows; ++y) {
			if (board_get(board, x, y)) {
				fprintf(file, "%d,%d\n", x, y);
			}
		}
	}

	fclose(file);
}

extern board_t *
board_load(const char *path) {
	int x, y;
	FILE *file;
	board_t *board;

	if (!(file = fopen(path, "r"))) {
		dief("failed to open file %s: %s", path, strerror(errno));
	}

	board = board_create(300, 300);

	while (fscanf(file, "%d,%d\n", &x, &y) == 2) {
		board_set(board, x, y, true);
	}

	fclose(file);

	return board;
}

extern board_t *
board_create(i32 rows, i32 columns) {
	board_t *board;

	if ((board = malloc(sizeof(board_t)))) {
		board->rows = rows;
		board->columns = columns;

		if ((board->cells = calloc(rows * columns, sizeof(bool)))) {
			return board;
		}
	}

	die("error while calling malloc");

	return (void *)(0);
}

extern void
board_free(board_t *board) {
	free(board->cells);
	free(board);
}
