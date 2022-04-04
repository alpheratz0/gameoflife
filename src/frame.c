#include <GL/freeglut.h>
#include "frame.h"

int frame_start = 0;
int frames_per_second = 1;

extern void
set_frame_rate(int fps) {
	frames_per_second = fps;
}

extern void
set_frame_start() {
	frame_start = glutGet(GLUT_ELAPSED_TIME);
}

extern int
is_on_frame() {
	int elapsed = glutGet(GLUT_ELAPSED_TIME) - frame_start;
	
	if (elapsed > (1000 / frames_per_second)) {
		frame_start += elapsed;
		return 1;
	}

	return 0;
}
