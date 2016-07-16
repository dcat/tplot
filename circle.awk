#!/usr/bin/awk -f

BEGIN {
	pi = 3.14159;
	c = 50;
	r = 45;

	for (i=0; i < pi * 2; i += pi/10) {
		x = cos(i) * r;
		y = sin(i) * r;

		printf("%d %d to %d %d\n", c + x, c + y, c, c);
	}
}
