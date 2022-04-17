#ifndef __GAMEOFLIFE_BASE_BOARD_H__
#define __GAMEOFLIFE_BASE_BOARD_H__

#include <stdbool.h>

#include "numdef.h"

typedef struct board board_t;

struct board {
	i32 rows;
	i32 columns;
	bool *cells;
};

extern board_t *
board_create(i32 rows, i32 columns);

extern board_t *
board_load(const char *path);

extern void
board_save(board_t *board);

extern bool
board_get(board_t *board, i32 x, i32 y);

extern void
board_set(board_t *board, i32 x, i32 y, bool value);

extern void
board_toggle(board_t *board, i32 x, i32 y);

extern void
board_advance(board_t **ref);

extern void
board_shift(board_t **ref, i32 xo, i32 yo);

extern void
board_free(board_t *board);

#endif
