#include <flow.h>
#include <stdio.h>

int flow_run() {
	unsigned number = 9;
	flow_read(0, &number, sizeof(number));
	printf("NUMBER: %u\n", number);
	return 0;
}
