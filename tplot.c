#include <sys/ioctl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tplot.h"

static cell_t cells[300][300];

/*
 * print a braille "pixel"
 */
void
dot(unsigned rx, unsigned ry) {
	unsigned y, x, i;

	x = (rx - 1) / 2;
	y = (ry - 1) / 4;

	/* A B
	 * C D
	 * E F
	 * G H
	 */

	switch (ry % 4) {
	case 1: /* A B */
		if (rx % 2)
			cells[x][y].a = 1;
		else
			cells[x][y].b = 1;
		break;
	case 2: /* C D */
		if (rx % 2)
			cells[x][y].c = 1;
		else
			cells[x][y].d = 1;
		break;
	case 3: /* E F */
		if (rx % 2)
			cells[x][y].e = 1;
		else
			cells[x][y].f = 1;
		break;
	case 0: /* G H */
		if (rx % 2)
			cells[x][y].g = 1;
		else
			cells[x][y].h = 1;
		break;
	}

	/* find the right char to print via table */
	for (i=0; i < LEN(br); i++)
		if (	   cells[x][y].a == br[i].a && cells[x][y].b == br[i].b
			&& cells[x][y].c == br[i].c && cells[x][y].d == br[i].d
			&& cells[x][y].e == br[i].e && cells[x][y].f == br[i].f
			&& cells[x][y].g == br[i].g && cells[x][y].h == br[i].h
		)
			printf("\033[%u;%uH%s", y+1, x+1, br[i].chr);
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
	char *argv0;
	unsigned x1, x2, y1, y2;
	struct termios tc_new, tc_old;

	tcgetattr(1, &tc_new);
	tc_old = tc_new;

	tc_new.c_lflag &= ~(ICANON | IEXTEN | ISIG);
	tcsetattr(1, TCSAFLUSH, &tc_new);

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
	putchar('\n');

	return 0;
}
