#include <flow.h>
#include <stdio.h>

int flow_run() {
	unsigned number = 0;
	flow_read(0, &number, sizeof(number));
	printf("number: %u\n", number);
	return 0;
}
