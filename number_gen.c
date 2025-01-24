#include <flow.h>

int flow_run() {
	unsigned number = 3;
	flow_write(0, &number, sizeof(number));
	return 0;
}
