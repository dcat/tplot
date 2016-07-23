#include <sys/ioctl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <stdio.h>
#include <err.h>

#include "tplot.h"

static int *cells;
static struct winsize ws;

/*
 * print a braille "pixel"
 */
void
dot(int rx, int ry) {
	int y, x, i, *p;

	if (rx > (ws.ws_col * 2) || rx < 1 || ry > (ws.ws_row * 4) || ry < 1) {
		warnx("out of bounds");
		return;
	}

	x = (rx - 1) / 2;
	y = (ry - 1) / 4;

	/* A B
	 * C D
	 * E F
	 * G H
	 */

	p = cells;
	p += (y * ws.ws_col) + x;


	switch (ry % 4) {
	case 1: /* A B */
		*p |= rx & 1 ? FIELD_A : FIELD_B;
		break;
	case 2: /* C D */
		*p |= rx & 1 ? FIELD_C : FIELD_D;
		break;
	case 3: /* E F */
		*p |= rx & 1 ? FIELD_E : FIELD_F;
		break;
	case 0: /* G H */
		*p |= rx & 1 ? FIELD_G : FIELD_H;
		break;
	}

	/* find the right char to print via table */
	printf("\033[%u;%uH%lc", y + 1, x + 1, *p | BRAILLE_EMPTY);
}

/*
 * dot() a line from x0, y0 to x1, y1
 */
void
line(int x0, int y0, int x1, int y1) {
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;
	int er = (dx > dy ? dx : -dy)/2;
	int e2;

	for (;;) {
		dot(x0, y0);

		if (x0 == x1 && y0 == y1)
			break;

		e2 = er;

		if (e2 > -dx) {
			er -= dy;
			x0 += sx;
		}

		if (e2 < dy) {
			er += dx;
			y0 += sy;
		}
	}
}


int
main(void) {
	char buf[BUFSIZ];
	int x1, x2, y1, y2;
	struct termios tc_new, tc_old;

	if (ioctl(1, TIOCGWINSZ, &ws) < 0)
		err(1, "ioctl()");

	cells = malloc(sizeof(wchar_t) * (ws.ws_col * ws.ws_row) * 8);
	if (cells == NULL)
		err(1, "malloc()");

	tcgetattr(1, &tc_new);
	tc_old = tc_new;

	tc_new.c_lflag &= ~(ISIG);
	tcsetattr(1, TCSAFLUSH, &tc_new);

	setlocale(LC_ALL, "");

	while (fgets(buf, BUFSIZ, stdin)) {
		sscanf(buf, "%u %u %*s %u %u", &x1, &y1, &x2, &y2);

		if (x2 && y2)
			line(x1, y1, x2, y2);
		else if (x1 && y1)
			dot(x1, y1);
		else
			/* if input is not valid, quit */
			break;

		x1 = x2 = y1 = y2 = 0;
	}

	tcsetattr(1, TCSAFLUSH, &tc_old);
	free(cells);

	return 0;
}
