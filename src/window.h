#ifndef __GAMEOFLIFE_WINDOW_H__
#define __GAMEOFLIFE_WINDOW_H__

#include "numdef.h"

typedef void (*window_display_callback_t)(i32 width, i32 height);
typedef void (*window_mouse_down_callback_t)(i32 x, i32 y, i32 button);
typedef void (*window_key_press_callback_t)(u8 key);
typedef void (*window_loopfunc_t)(void);

extern void
window_init(const char *name);

extern void
window_loop_start(void);

extern void
window_set_display_callback(window_display_callback_t cb);

extern void
window_set_mouse_down_callback(window_mouse_down_callback_t cb);

extern void
window_set_key_press_callback(window_key_press_callback_t cb);

extern void
window_set_loop(window_loopfunc_t f);

extern void
window_clear_loop();

extern void
window_force_redraw(void);

#endif
