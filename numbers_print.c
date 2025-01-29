#include <flow.h>
#include <stdio.h>

int flow_run() {
	unsigned a, b;
	flow_read(0, &a, sizeof(a));
	flow_read(1, &b, sizeof(b));
	printf("numbers: %u %u\n", a, b);
	return 0;
}
